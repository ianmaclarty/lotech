#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lt.h"

#define DELAY ((int)((1.0 / 60.0) * 1000000.0))
#define MAX_CMD_LEN 1024

static const char *sync_file_patterns[] = {"*.png", "*.lua", NULL};

static bool need_prompt = true;
static bool need_nl_before_logs = true;
static time_t last_sync_time = 0;

static char command[MAX_CMD_LEN];

static void banner() {
    printf("LoTech Dev Server\n");
    fflush(NULL);
}

static void prompt() {
    static int steps_since_last_prompt = 10;
    if (need_prompt) {
        if (steps_since_last_prompt > 5) {
            printf("[%s] ", command);
            fflush(NULL);
            need_prompt = false;
            steps_since_last_prompt = 0;
            need_nl_before_logs = true;
        } else {
            steps_since_last_prompt++;
        }
    }
}

static void cmd_sync() {
    struct stat info;
    char *matches = ltGlob(sync_file_patterns);
    char *ptr = matches;
    bool were_updates = false;
    while (*ptr != '\0') {
        if (stat(ptr, &info) == 0) {
            if (info.st_mtime >= last_sync_time) {
                ltServerClientUpdateFile(ptr);
                were_updates = true;
            }
        } else {
            fprintf(stderr, "Unable to stat %s: %s\n", ptr, strerror(errno));
        }
        ptr += strlen(ptr) + 1;
    }
    delete[] matches;
    last_sync_time = time(NULL);
    if (!were_updates) {
        printf("No files changed since last sync\n");
    }
    ltServerClientReset();
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
    need_prompt = true;
}

static void print_logs() {
    char *log;
    bool first = true;
    while (true) {
        log = ltPopClientLog();
        if (log == NULL) {
            break;
        }
        if (first && need_nl_before_logs) {
            printf("\n");
            first = false;
        }
        printf("%s\n", log);
        delete[] log;
        need_prompt = true;
    }
    fflush(NULL);
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
    int t = 0;
    while (!ltServerIsReady()) {
        ltServerStep();
        usleep(DELAY);
        if (t % 30 == 0) {
            printf(".");
            fflush(NULL);
        }
        t++;
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
                need_nl_before_logs = false;
            }
            if (buf[0] != '\0') {
                strcpy(command, buf);
            }
            execute_command();
        }
        ltServerStep();
        print_logs();
        usleep(DELAY);
        prompt();
    }
}
