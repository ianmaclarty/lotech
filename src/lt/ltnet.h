/* Copyright (C) 2010 Ian MacLarty */
#ifndef LTNET_H
#define LTNET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

enum LTServerStateEnum {
    LT_SERVER_STATE_INITIALIZED,
    LT_SERVER_STATE_WAITING_FOR_CLIENT_BROADCAST,
    LT_SERVER_STATE_CONNECTING_TO_CLIENT,
    LT_SERVER_STATE_READY,
    LT_SERVER_STATE_ERROR,
};

struct LTServerState {
    LTServerStateEnum state;
    int sock; // UDP socket when listening, TCP socket when connecting.
    sockaddr_in client_addr;
    char *errmsg;

    LTServerState();
    virtual ~LTServerState();

    void connectStep();

    bool isReady();
    bool isError();
};

enum LTClientStateEnum {
    LT_CLIENT_STATE_INITIALIZED,
    LT_CLIENT_STATE_BROADCASTING,
    LT_CLIENT_STATE_LISTENING,
    LT_CLIENT_STATE_READY,
    LT_CLIENT_STATE_ERROR,
};

struct LTClientState {
    LTClientStateEnum state;
    int sock; // UDP socket when broadcasting, TCP socket when accepting.
    char *errmsg;
    int listen_step;

    LTClientState();
    virtual ~LTClientState();

    void connectStep();

    bool isReady();
    bool isError();
};

#endif
