#include <unistd.h>

#include "lt.h"

#define MSG "hello"

int main() {
    LTServerState server;
    while (!server.isReady() && !server.isError()) {
        printf("server step\n");
        server.connectStep();
        usleep(100);
    }
    if (server.isReady()) {
        printf("Server connect succeeded\n");
        server.sendMsg(MSG, strlen(MSG) + 1);
        server.closeServer();
    } else {
        printf("Server connection failed: %s\n", server.errmsg);
    }
}
