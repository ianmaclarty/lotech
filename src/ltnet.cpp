/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
#include "lt.h"

LT_INIT_IMPL(ltnet)
#ifndef LTMINGW

#define PORT 14091
#define MAGIC_WORD "lotech"
#define MAXBUFLEN 64
#define MAX_LISTENING_STEPS 60

/*
 * A (blocking) TCP socket is set up between the client and server as follows:
 * - The client broadcasts a udp packet on port PORT and then
 *   listens for TCP connections on the same port with a non-blocking socket.
 *   If no connections are received after MAX_LISTENING_STEPS calls
 *   to connectStep then no connection is established and the client
 *   goes into the closed state.
 * - The server listens for udp broadcasts on port PORT.  If it receives
 *   one, it tries to open a (blocking) TCP connection to the IP address
 *   the udp packet came from on PORT.
 * - The client accepts the TCP connection initiated by the server and
 *   creates a new (blocking) TCP socket for it.
 * - The new (blocking) TCP connection is then used for sending and receiving
 *   by the client and server.  Sending always blocks until the message
 *   is completely sent.  Receiving uses select to check if there is anything
 *   to be read on the socket, and if there is the whole message is read
 *   in one go.
 */

/*
static void checksum(const char *msg, const char *packet, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += packet[i];
    }
    fprintf(stderr, "%s: len = %d, checksum = %d\n", msg, len, sum);
}
*/

static void copy_string(char **dest, const char *src) {
    *dest = new char[strlen(src) + 1];
    strcpy(*dest, src);
}

static void copy_errmsg(char **errmsg) {
    char *msg = strerror(errno);
    copy_string(errmsg, msg);
}

static int enable_broadcast(int sock) {
    int opt = 1;
    return setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof opt);
}

static int enable_reuse(int sock) {
    int opt = 1;
    return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
}

static int make_nonblocking(int sock) {
    long flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

static int make_blocking(int sock) {
    long flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    return fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
}

static int tcp_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return -1;
    }
    if (enable_reuse(sock) != 0) {
        return -1;
    }
    return sock;
}

static int udp_socket() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return -1;
    }
    if (enable_reuse(sock) != 0) {
        return -1;
    }
    if (make_nonblocking(sock) != 0) {
        return -1;
    }
    return sock;
}

/**
 * Returns fd of listening socket on success and -1
 * on error (setting errno).
 */
static int server_start_listening() {
    int lsock;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    lsock = udp_socket();
    if (lsock < 0) {
        return -1;
    }
    if (bind(lsock, (const sockaddr*)&server_addr, sizeof(sockaddr_in)) != 0) {
        return -1;
    }
    return lsock;
}

/**
 * Returns 1 if a broadcast is received and fills in client_addr.
 * Returns 0 if no broadcast received yet.
 * Returns -1 and sets errno on error.
 */
static int server_check_for_broadcast(int lsock, sockaddr_in *client_addr) {
    char buf[MAXBUFLEN];
    socklen_t client_addr_len = sizeof(sockaddr_in);
    int rv = recvfrom(lsock, buf, MAXBUFLEN, 0, (sockaddr*)client_addr, &client_addr_len);
    if (rv > 0) {
        if (rv < MAXBUFLEN) {
            buf[rv] = '\0';
            if (strcmp(buf, MAGIC_WORD) == 0) {
                return 1;
            }
        }
        // Ignore the packet if it doesn't contain the expected data.
        return 0;
    } else {
        if (errno == EAGAIN) {
            return 0;
        } else {
            return -1;
        }
    }
}

/**
 * Returns connecting socket on success,
 * or -1 on failure and sets errno.
 */
static int server_start_connect_client(sockaddr_in *their_addr) {
    int sock;
    sock = tcp_socket();
    if (sock < 0) {
        return -1;
    }
    their_addr->sin_port = htons(PORT);
    int r = connect(sock, (const sockaddr*)their_addr, sizeof(sockaddr_in));
    if (r == 0 || (r == -1 && errno == EINPROGRESS)) {
        return sock;
    } else {
        return -1;
    }
}

/**
 * Return 1 if the socket is ready for writing, 0 if it isn't
 * and -1 if there was an error (errno is set).
 */
static int socket_can_write(int sock) {
    fd_set writefds;
    struct timeval tv;
    tv.tv_sec = 1;  //  XXX Should be 0?
    tv.tv_usec = 1; //  XXX Should be 0?
    FD_ZERO(&writefds);
    FD_SET(sock, &writefds);
    return select(sock + 1, NULL, &writefds, NULL, &tv);
}

/**
 * Return 1 if the socket is ready for reading, 0 if it isn't
 * and -1 if there was an error (errno is set).
 */
static int socket_can_read(int sock) {
    fd_set readfds;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    return select(sock + 1, &readfds, NULL, NULL, &tv);
}

static void server_error_state(LTServerConnection *state, const char *msg) {
    if (state->errmsg != NULL) {
        delete[] state->errmsg;
        state->errmsg = NULL;
    }
    if (msg == NULL) {
        copy_errmsg(&state->errmsg);
    } else {
        copy_string(&state->errmsg, msg);
    }
    state->state = LT_SERVER_STATE_ERROR;
}

/**
 * Return 1 if the entire message was received, 0 if
 * there is no message to receive yet (try again later)
 * and -1 on error (and errmsg is set with newly allocated string
 * that must be freed with delete[]).
 */
static int receive_msg(int sock, char **msg, int *n, char **errmsg) {
    *errmsg = NULL;
    *msg = NULL;
    *n = 0;
    LTuint32 len;
    int r;
    r = socket_can_read(sock);
    if (r > 0) {
        r = read(sock, &len, 4);
        if (r < 0) {
            copy_errmsg(errmsg);
            return -1;
        }
        if (r == 0) {
            copy_string(errmsg, "Connection reset by peer.");
            return -1;
        }
        if (r != 4) {
            copy_string(errmsg, "Unable to read message length.");
            return -1;
        }
        *msg = new char[len];
        *n = (int)len;
        char *ptr = *msg;
        while (len > 0) {
            // read doesn't have to read all the bytes before returning.
            r = read(sock, ptr, len);
            if (r < 0) {
                copy_errmsg(errmsg);
                delete[] *msg;
                return -1;
            } 
            len -= r;
            ptr += r;
        }
        //checksum("received", *msg, *n);
        return 1;
    } else if (r < 0) {
        copy_errmsg(errmsg);
        return -1;
    } else {
        return 0;
    }
}

/**
 * Sends the entire message.
 * Returns 0 on success or -1 on error (and sets errmsg to a
 * newly allocated string that must be freed with delete[]).
 */
static int send_msg(int sock, const char *msg, int n, char **errmsg) {
    *errmsg = NULL;
    LTuint32 len = (LTuint32)n;
    int r = write(sock, (const char *)&len, 4);
    if (r < 0) {
        copy_errmsg(errmsg);
        return -1;
    }
    if (r != 4) {
        copy_string(errmsg, "Unable to send length.");
        return -1;
    }
    r = write(sock, msg, n);
    if (r < 0) {
        copy_errmsg(errmsg);
        return -1;
    }
    if (r != n) {
        copy_string(errmsg, "Unable to send entire message.");
        return -1;
    }
    //checksum("send", msg, len);
    return 0;
}

LTServerConnection::LTServerConnection() {
    state = LT_SERVER_STATE_INITIALIZED;
    errmsg = NULL;
}

LTServerConnection::~LTServerConnection() {
    if (errmsg != NULL) {
        delete[] errmsg;
    }
}

void LTServerConnection::connectStep() {
    int rv;
    switch (state) {
        case LT_SERVER_STATE_INITIALIZED: 
            sock = server_start_listening();
            if (sock < 0) {
                server_error_state(this, NULL);
                return;
            }
            state = LT_SERVER_STATE_WAITING_FOR_CLIENT_BROADCAST;
            return;
        case LT_SERVER_STATE_WAITING_FOR_CLIENT_BROADCAST:
            rv = server_check_for_broadcast(sock, &client_addr);
            if (rv < 0) {
                server_error_state(this, NULL);
                return;
            }
            if (rv == 1) {
                close(sock);
                sock = server_start_connect_client(&client_addr);
                if (sock < 0) {
                    server_error_state(this, NULL);
                    return;
                }
                state = LT_SERVER_STATE_CONNECTING_TO_CLIENT;
            }
            return;
        case LT_SERVER_STATE_CONNECTING_TO_CLIENT:
            rv = socket_can_write(sock);
            if (rv < 0) {
                server_error_state(this, NULL);
                return;
            }
            if (rv == 1) {
                state = LT_SERVER_STATE_READY;
            }
            return;
        case LT_SERVER_STATE_READY:
            return;
        case LT_SERVER_STATE_ERROR:
            return;
        case LT_SERVER_STATE_CLOSED:
            return;
    }
}

bool LTServerConnection::isReady() {
    return state == LT_SERVER_STATE_READY;
}

bool LTServerConnection::isError() {
    return state == LT_SERVER_STATE_ERROR;
}

void LTServerConnection::sendMsg(const char *msg, int n) {
    char *err;
    if (state != LT_SERVER_STATE_READY) {
        server_error_state(this, "Not ready");
        return;
    }
    int r = send_msg(sock, msg, n, &err);
    if (r < 0) {
        server_error_state(this, err);
        delete[] err;
    }
}

bool LTServerConnection::recvMsg(char **msg, int *n) {
    char *err;
    if (state != LT_SERVER_STATE_READY) {
        server_error_state(this, "Not ready");
        return false;
    }
    int r = receive_msg(sock, msg, n, &err);
    if (r < 0) {
        server_error_state(this, err);
        delete[] err;
        return false;
    }
    if (r == 0) {
        // No error, but no message available either.
        return false;
    }
    return true;
}

void LTServerConnection::closeServer() {
    close(sock);
    state = LT_SERVER_STATE_CLOSED;
}

const char* LTServerConnection::stateStr() {
    switch (state) {
        case LT_SERVER_STATE_INITIALIZED: return "Initialized";
        case LT_SERVER_STATE_WAITING_FOR_CLIENT_BROADCAST: return "Waiting for broadcast";
        case LT_SERVER_STATE_CONNECTING_TO_CLIENT: return "Connecting to client";
        case LT_SERVER_STATE_READY: return "Ready";
        case LT_SERVER_STATE_ERROR: return "Error";
        case LT_SERVER_STATE_CLOSED: return "Closed";
    }
    return "unknown";
}

//-------------------------------------------------------------

/**
 * Returns 0 on success.
 * Otherwise returns -1 and sets errno.
 */
static int client_broadcast() {
    int sock;
    struct sockaddr_in broadcast_addr;
    int numbytes;

    sock = udp_socket();
    if (sock < 0) {
        return -1;
    }
    
    if (enable_broadcast(sock) != 0) {
        return -1;
    }

    broadcast_addr.sin_family = AF_INET;         // host byte order
    broadcast_addr.sin_port = htons(PORT); // short, network byte order
    broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;

    numbytes = sendto(sock, MAGIC_WORD, strlen(MAGIC_WORD), 0,
        (struct sockaddr *)&broadcast_addr, sizeof(sockaddr_in));
    if (numbytes < 0) {
        return -1;
    }

    close(sock);
    return 0;
}

/**
 * Returns listening socket fd on success, -1 on failure and sets errno.
 */
static int client_start_listening() {
    int lsock;
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT);
    client_addr.sin_addr.s_addr = INADDR_ANY;
    lsock = tcp_socket();
    if (lsock < 0) {
        return -1;
    }
    if (bind(lsock, (const sockaddr*)&client_addr, sizeof(sockaddr_in)) != 0) {
        return -1;
    }
    if (listen(lsock, 1) != 0) {
        return -1;
    }
    // Make non-blocking so accept doesn't block.
    if (make_nonblocking(lsock) != 0) {
        return -1;
    }
    return lsock;
}

/**
 * Tries to accept a connection on a listening socket.
 * Returns new connection socket fd on success, 0 if there
 * are no connections to be accepted yet and -1 on other errors,
 * setting errno.
 */
static int client_try_accept(int lsock) {
    int fd = accept(lsock, NULL, NULL);
    if (fd > 0) {
        if (make_blocking(fd) != 0) {
            close(fd);
            return -1;
        }
        return fd;
    } else {
        if (errno == EAGAIN) {
            return 0;
        } else {
            return -1;
        }
    }
}

static void client_error_state(LTClientConnection *state, const char *msg) {
    if (state->errmsg != NULL) {
        delete[] state->errmsg;
        state->errmsg = NULL;
    }
    if (msg == NULL) {
        copy_errmsg(&state->errmsg);
    } else {
        copy_string(&state->errmsg, msg);
    }
    state->state = LT_CLIENT_STATE_ERROR;
}

LTClientConnection::LTClientConnection() {
    state = LT_CLIENT_STATE_INITIALIZED;
    errmsg = NULL;
    listen_step = 0;
}

LTClientConnection::~LTClientConnection() {
    if (errmsg != NULL) {
        delete[] errmsg;
    }
}

void LTClientConnection::connectStep() {
    int rv;
    switch (state) {
        case LT_CLIENT_STATE_INITIALIZED: 
            state = LT_CLIENT_STATE_BROADCASTING;
            return;
        case LT_CLIENT_STATE_BROADCASTING:
            rv = client_broadcast();
            if (rv < 0) {
                client_error_state(this, NULL);
                return;
            }
            sock = client_start_listening();
            if (sock < 0) {
                client_error_state(this, NULL);
                return;
            }
            state = LT_CLIENT_STATE_LISTENING;
            listen_step = 0;
            return;
        case LT_CLIENT_STATE_LISTENING:
            rv = client_try_accept(sock);
            if (rv < 0) {
                client_error_state(this, NULL);
                return;
            }
            if (rv > 0) {
                // Connection established.
                close(sock); // Close listening socket.
                sock = rv;
                state = LT_CLIENT_STATE_READY;
                return;
            }
            // rv == 0
            listen_step++;
            if (listen_step > MAX_LISTENING_STEPS) {
                // Give up.
                close(sock);
                state = LT_CLIENT_STATE_CLOSED;
                return;
            }
            return;
        case LT_CLIENT_STATE_READY:
            return;
        case LT_CLIENT_STATE_ERROR:
            return;
        case LT_CLIENT_STATE_CLOSED:
            return;
    }
}

bool LTClientConnection::isReady() {
    return state == LT_CLIENT_STATE_READY;
}

bool LTClientConnection::isTryingToConnect() {
    return state < LT_CLIENT_STATE_READY;
}

bool LTClientConnection::isError() {
    return state == LT_CLIENT_STATE_ERROR;
}

void LTClientConnection::sendMsg(const char *msg, int n) {
    char *err;
    if (state != LT_CLIENT_STATE_READY) {
        client_error_state(this, "Not ready");
        return;
    }
    int r = send_msg(sock, msg, n, &err);
    if (r < 0) {
        client_error_state(this, err);
        delete[] err;
        return;
    }
}

bool LTClientConnection::recvMsg(char **msg, int *n) {
    char *err;
    if (state != LT_CLIENT_STATE_READY) {
        client_error_state(this, "Not ready");
        return false;
    }
    int r = receive_msg(sock, msg, n, &err);
    if (r < 0) {
        client_error_state(this, err);
        delete[] err;
        return false;
    }
    if (r == 0) {
        return false;
    }
    return true;
}

void LTClientConnection::closeClient() {
    close(sock);
    state = LT_CLIENT_STATE_CLOSED;
}

const char* LTClientConnection::stateStr() {
    switch (state) {
        case LT_CLIENT_STATE_INITIALIZED: return "initialized";
        case LT_CLIENT_STATE_BROADCASTING: return "broadcasting";
        case LT_CLIENT_STATE_LISTENING: return "listening";
        case LT_CLIENT_STATE_READY: return "ready";
        case LT_CLIENT_STATE_ERROR: return "error";
        case LT_CLIENT_STATE_CLOSED: return "closed";
    }
    return "unknown";
}
#endif
