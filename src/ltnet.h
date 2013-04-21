/* Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in lt.h. */
LT_INIT_DECL(ltnet)
#ifndef LTMINGW

enum LTServerState {
    LT_SERVER_STATE_INITIALIZED,
    LT_SERVER_STATE_WAITING_FOR_CLIENT_BROADCAST,
    LT_SERVER_STATE_CONNECTING_TO_CLIENT,
    LT_SERVER_STATE_READY,
    LT_SERVER_STATE_ERROR,
    LT_SERVER_STATE_CLOSED,
};

struct LTServerConnection {
    LTServerState state;
    int sock; // UDP socket when listening, TCP socket when connecting.
    sockaddr_in client_addr;
    char *errmsg;

    LTServerConnection();
    virtual ~LTServerConnection();

    void connectStep();

    bool isReady();
    bool isError();

    void sendMsg(const char *buf, int n);
    // Tries to recv one message.  Returns true on success and false if no
    // message was retrieved.  Call isError to see if there was an error.  On
    // success space for the data is allocated in buf.  The caller is
    // responsible for freeing the data with delete[].
    bool recvMsg(char **buf, int *n);

    void closeServer();

    const char *stateStr();
};

enum LTClientState {
    LT_CLIENT_STATE_INITIALIZED,
    LT_CLIENT_STATE_BROADCASTING,
    LT_CLIENT_STATE_LISTENING,
    LT_CLIENT_STATE_READY,
    LT_CLIENT_STATE_ERROR,
    LT_CLIENT_STATE_CLOSED,
};

struct LTClientConnection {
    LTClientState state;
    int sock; // UDP socket when broadcasting, TCP socket when accepting.
    char *errmsg;
    int listen_step;

    LTClientConnection();
    virtual ~LTClientConnection();

    void connectStep();

    bool isReady();
    bool isError();
    bool isTryingToConnect();

    void sendMsg(const char *buf, int n);
    // See LTServerState::recvMsg.
    bool recvMsg(char **buf, int *n);

    void closeClient();

    const char *stateStr();
};

#endif
