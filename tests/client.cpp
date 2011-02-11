#include <unistd.h>

#include "lt.h"

int main() {
    LTClientState client;
    while (!client.isReady() && !client.isError()) {
        printf("client step\n");
        client.connectStep();
        usleep(100);
    }
    if (client.isReady()) {
        printf("Client connect succeeded\n");
        int r;
        char *buf;
        int n;
        while (1) {
            r = client.recvMsg(&buf, &n);
            if (r == 0) {
                usleep(100);
                continue;
            }
            if (r == 1) {
                printf("Received message: %s\n", buf);
                client.closeClient();
                break;
            }
            fprintf(stderr, "Error: %s\n", client.errmsg);
            client.closeClient();
            break;
        }
    } else {
        printf("Client connection failed: %s\n", client.errmsg);
    }
}
