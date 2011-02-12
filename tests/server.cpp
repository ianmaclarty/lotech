#include <unistd.h>

#include "lt.h"

#define MSG "hello"
#define DELAY ((int)((1.0 / 60.0) * 1000000.0))

int main() {
    LTServerConnection server;
    while (!server.isReady() && !server.isError()) {
        printf("Server state: %s\n", server.stateStr());
        server.connectStep();
        usleep(DELAY);
    }
    if (server.isReady()) {
        printf("Server connect succeeded\n");
        server.sendMsg(MSG, strlen(MSG) + 1);
        if (server.isError()) {
            fprintf(stderr, "Send error: %s\n", server.errmsg);
        } else {
            printf("Send OK\n");
        }
        server.closeServer();
    } else {
        printf("Server connection failed: %s\n", server.errmsg);
    }
}
