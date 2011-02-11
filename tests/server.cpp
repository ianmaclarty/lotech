#include <unistd.h>

#include "lt.h"

int main() {
    LTServerState server;
    while (!server.isReady() && !server.isError()) {
        printf("server step\n");
        server.connectStep();
        usleep(100);
    }
    if (server.isReady()) {
        printf("Server connect succeeded\n");
    } else {
        printf("Server connection failed: %s\n", server.errmsg);
    }
}
