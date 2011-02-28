#include <unistd.h>

#include "lt.h"

#define DELAY ((int)((1.0 / 60.0) * 1000000.0))

#define MAX_CMD_LEN 1024

static char command[MAX_CMD_LEN];

static void banner() {
    printf("LoTech Dev Server\n");
    fflush(NULL);
}

static void prompt() {
    printf("[%s] ", command);
    fflush(NULL);
}

static void cmd_sync() {
    ltServerUpdateFile("test.png");
}

static void execute_command() {
    if (strcmp(command, "quit") == 0) {
        ltServerShutdown();
        printf("Bye!\n");
        exit(0);
    } else if (strcmp(command, "sync") == 0) {
        cmd_sync();
    } else {
        printf("Unrecognized command: %s\n", command);
    }
}

int main() {
    ssize_t r;
    const int in = 0;
    char buf[MAX_CMD_LEN];
    long flags = fcntl(in, F_GETFL, 0);
    if (flags < 0) {
        fprintf(stderr, "Unable to get flags for stdin: %s\n", strerror(errno));
        exit(1);
    }
    if (fcntl(in, F_SETFL, flags | O_NONBLOCK) != 0) {
        fprintf(stderr, "Unable to set non-blocking flag for stdin: %s\n", strerror(errno));
        exit(1);
    }
     
    strcpy(command, "sync");
    banner();
    printf("Connecting...");
    fflush(NULL);
    ltServerInit();
    while (!ltServerIsReady()) {
        ltServerStep();
        usleep(DELAY);
        printf(".");
        fflush(NULL);
    }
    printf("\nConnection established.\n");

    prompt();
    while (true) {
        r = read(in, buf, MAX_CMD_LEN);
        if (r < 0) {
            if (errno != EAGAIN) {
                fprintf(stderr, "Error reading from stdin: %s\n", strerror(errno));
                exit(1);
            }
        }
        if (r > 0) {
            buf[r] = '\0';
            if (buf[r - 1] == '\n') {
                buf[r - 1] = '\0';
            }
            if (buf[0] != '\0') {
                strcpy(command, buf);
            }
            execute_command();
            prompt();
        }
        ltServerStep();
        usleep(DELAY);
    }
}
