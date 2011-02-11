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
    } else {
        printf("Client connection failed: %s\n", client.errmsg);
    }
}
