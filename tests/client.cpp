#include <unistd.h>

#include "lt.h"

#define DELAY ((int)((1.0 / 60.0) * 1000000.0))

int main() {
    LTClientConnection client;
    while (!client.isReady() && !client.isError()) {
        printf("Client state: %s\n", client.stateStr());
        client.connectStep();
        usleep(DELAY);
    }
    if (client.isReady()) {
        printf("Client connect succeeded\n");
        char *buf;
        int n;
        while (1) {
            if (client.recvMsg(&buf, &n)) {
                printf("Received message: %s\n", buf);
                client.closeClient();
                break;
            } else {
                if (client.isError()) {
                    fprintf(stderr, "Error receiving message: %s\n", client.errmsg);
                    client.closeClient();
                    break;
                } else {
                    usleep(DELAY);
                    continue;
                }
            }
        }
    } else {
        printf("Client connection failed: %s\n", client.errmsg);
    }
}
