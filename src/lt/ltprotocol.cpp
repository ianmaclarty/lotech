/* Copyright (C) 2011 Ian MacLarty */
#include "ltprotocol.h"

#include <list>
#include <sys/stat.h>

#include "ltiosutil.h"
#include "ltnet.h"
#include "ltlua.h"
#include "ltutil.h"

enum LTCommandOpcode {
    LT_CMD_OP_LOG = 'L',
    LT_CMD_OP_UPDATEFILE = 'U',
    LT_CMD_OP_RESET = 'R',
};

struct LTCommand {
    LTCommandOpcode opcode;
    LTCommand(LTCommandOpcode opcode) {
        LTCommand::opcode = opcode;
    }
    virtual ~LTCommand() {};

    virtual void doCommand() = 0;
};

// The caller must free the returned message with delete[].
static char* encode_command(LTCommand *cmd, int *size);

// The caller must free the command with delete.
static LTCommand* decode_command(const char *buf, int size);

//------------------ Client ------------------------------

static LTClientConnection *client_connection = NULL;
static std::list<LTCommand *> client_command_queue;
static std::list<char *> client_logs;

struct LTCommandLog : LTCommand {
    char *msg;

    LTCommandLog(const char *m) : LTCommand(LT_CMD_OP_LOG) {
        msg = new char[strlen(m) + 1];
        strcpy(msg, m);
    }
    virtual ~LTCommandLog() {
        delete[] msg;
    }

    virtual void doCommand() {
        char *log = new char[strlen(msg) + 1];
        strcpy(log, msg);
        client_logs.push_back(log);
    }
};

bool ltAmClient() {
    return client_connection != NULL;
}

void ltClientInit() {
    if (client_connection == NULL) {
        client_connection = new LTClientConnection();
    }
}

#define MAX_RETRIES 100

void ltClientStep() {
    static int retry_count = 0;
    int len;
    char *buf;
    if (client_connection == NULL) {
        return;
    }
    if (!client_connection->isReady() && !client_connection->isError()) {
        client_connection->connectStep();
        return;
    }
    // Receive and execute commands from the server.
    while (client_connection->isReady() && client_connection->recvMsg(&buf, &len)) {
        LTCommand *cmd = decode_command(buf, len);
        delete buf;
        if (cmd != NULL) {
            cmd->doCommand();
            delete cmd;
        }
    }
    // Send any pending commands to the server.
    while (client_connection->isReady() && client_command_queue.size() > 0) {
        LTCommand *cmd = client_command_queue.front();
        buf = encode_command(cmd, &len);
        client_connection->sendMsg(buf, len);
        delete[] buf;
        if (!client_connection->isError()) {
            client_command_queue.pop_front();
            delete cmd;
        }
    }
    if (client_connection->isError()) {
        if (retry_count < MAX_RETRIES) {
            fprintf(stderr, "Client connection error: %s\n", client_connection->errmsg);
            retry_count++;
            fprintf(stderr, "Retrying...\n");
            client_connection->closeClient();
            delete client_connection;
            client_connection = NULL;
            ltClientInit();
        } else {
            fprintf(stderr, "Too many client errors.  Giving up.\n");
        }
    }
}

void ltClientLog(const char *msg) {
    LTCommandLog *cmd = new LTCommandLog(msg);
    client_command_queue.push_back(cmd);
}

char *ltPopClientLog() {
    if (client_logs.size() > 0) {
        char *log = client_logs.front();
        client_logs.pop_front();
        return log;
    } else {
        return NULL;
    }
}

//------------------ Server ------------------------------

static LTServerConnection *server_connection = NULL;
static std::list<LTCommand *> server_command_queue;

struct LTCommandUpdateFile : LTCommand {
    char *file_name;
    char *data;
    int data_size;

    LTCommandUpdateFile(const char *fname, const char *dat, int len) : LTCommand(LT_CMD_OP_UPDATEFILE) {
        file_name = new char[strlen(fname) + 1];
        strcpy(file_name, fname);
        data = new char[len];
        memcpy(data, dat, len);
        data_size = len;
    }
    virtual ~LTCommandUpdateFile() {
        delete[] file_name;
        delete[] data;
    }

    virtual void doCommand() {
        FILE *f;
        #ifdef LTIOS
            const char *path = ltIOSBundlePath(file_name, NULL);
            f = fopen(path, "w");
            delete[] path;
        #else
            f = fopen(file_name, "w");
        #endif
        if (f != NULL) {
            size_t r = fwrite(data, 1, data_size, f);
            if (r < (size_t)data_size) {
                ltLog("Unable to write to %s: %s", file_name, strerror(errno));
            } else {
                ltLog("Synced %s", file_name);
            }
            fclose(f);
        } else {
            ltLog("Unable to open %s for writing: %s", file_name, strerror(errno));
        }
    }
};

struct LTCommandReset : LTCommand {
    LTCommandReset() : LTCommand(LT_CMD_OP_RESET) { }
    virtual ~LTCommandReset() { }

    virtual void doCommand() {
        ltLuaReset();
    }
};

bool ltAmServer() {
    return server_connection != NULL;
}

void ltServerInit() {
    if (server_connection == NULL) {
        server_connection = new LTServerConnection();
    }
}

void ltServerStep() {
    static int retry_count = 0;
    int len;
    char *buf;
    if (server_connection == NULL) {
        return;
    }
    if (!server_connection->isReady() && !server_connection->isError()) {
        server_connection->connectStep();
        return;
    }
    // Receive and execute commands from the client.
    while (server_connection->isReady() && server_connection->recvMsg(&buf, &len)) {
        LTCommand *cmd = decode_command(buf, len);
        delete buf;
        if (cmd != NULL) {
            cmd->doCommand();
            delete cmd;
        }
    }
    // Send any pending commands to the client.
    while (server_connection->isReady() && server_command_queue.size() > 0) {
        LTCommand *cmd = server_command_queue.front();
        buf = encode_command(cmd, &len);
        server_connection->sendMsg(buf, len);
        delete[] buf;
        if (!server_connection->isError()) {
            server_command_queue.pop_front();
            delete cmd;
        }
    }
    if (server_connection->isError()) {
        if (retry_count < MAX_RETRIES) {
            fprintf(stderr, "Server connection error: %s\n", server_connection->errmsg);
            retry_count++;
            fprintf(stderr, "Retrying...\n");
            server_connection->closeServer();
            delete server_connection;
            server_connection = NULL;
            ltServerInit();
        } else {
            fprintf(stderr, "Too many server errors.  Giving up.\n");
        }
    }
}

bool ltServerIsReady() {
    return server_connection != NULL && server_connection->isReady();
}

void ltServerShutdown() {
    if (server_connection != NULL) {
        server_connection->closeServer();
        delete server_connection;
        server_connection = NULL;
    }
}

void ltServerUpdateFile(const char *file) {
    struct stat info;
    int r = stat(file, &info);
    if (r != 0) {
        ltLog("Cannot stat %s: %s", file, strerror(errno));
        return;
    }
    int size = (int)info.st_size;
    FILE *f = fopen(file, "r");
    if (f != NULL) {
        char *buf = new char[size];
        r = (int)fread(buf, 1, size, f);
        if (r < 0 || r != size) {
            ltLog("Unable to read %s", file);
            fclose(f);
            delete[] buf;
            return;
        }
        fclose(f);
        LTCommandUpdateFile *cmd = new LTCommandUpdateFile(file, buf, size);
        server_command_queue.push_back(cmd);
        delete[] buf;
    } else {
        ltLog("Unable to open %s for reading: %s", file, strerror(errno));
        return;
    }
}

void ltServerReset() {
    LTCommandReset *cmd = new LTCommandReset();
    server_command_queue.push_back(cmd);
}

//------------------------------------------------

static char* encode_command(LTCommand *cmd, int *size) {
    LTCommandOpcode op = cmd->opcode;
    switch (op) {
        case LT_CMD_OP_LOG: {
            LTCommandLog *log = (LTCommandLog*)cmd;
            *size = strlen(log->msg) + 2;
            char *msg = new char[*size];
            msg[0] = (char)op;
            strcpy(&msg[1], log->msg);
            return msg;
        }
        case LT_CMD_OP_UPDATEFILE: {
            LTCommandUpdateFile *update = (LTCommandUpdateFile*)cmd;
            *size = strlen(update->file_name) + 2 + update->data_size;
            char *msg = new char[*size];
            msg[0] = (char)op;
            strcpy(&msg[1], update->file_name);
            memcpy(&msg[strlen(update->file_name) + 2], update->data, update->data_size);
            return msg;
        }
        case LT_CMD_OP_RESET: {
            *size = 1;
            char *msg = new char[*size];
            msg[0] = (char)op;
            return msg;
        }
    }
    return NULL;
}

static LTCommand* decode_command(const char *buf, int size) {
    if (size <= 0) return NULL;
    LTCommandOpcode op = (LTCommandOpcode)buf[0];
    switch (op) {
        case LT_CMD_OP_LOG: {
            if (size <= 1 || buf[size - 1] != '\0') return NULL;
            return new LTCommandLog(&buf[1]);
        }
        case LT_CMD_OP_UPDATEFILE: {
            if (size <= 1) return NULL;
            const char *ptr = buf + 1;
            const char *end = &buf[size];
            // Make sure the message contains a null character.
            while (true) {
                if (ptr == end) {
                    return NULL;
                }
                if (*ptr == '\0') {
                    break;
                }
                ptr++;
            }
            ptr++;
            int data_size = end - ptr;
            return new LTCommandUpdateFile(&buf[1], ptr, data_size);
        }
        case LT_CMD_OP_RESET: {
            return new LTCommandReset();
        }
    }
    return NULL;
}
