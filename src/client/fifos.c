#include "fifos.h"
#include "debug.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#define REQUESTS_FIFO "requests"
#define ANS_PERMISSIONS 0660

/**
 * requests
 */
static bool ans_initialised = false;
static bool ans_atexit_set = false;
static FILE* ans_fifo = NULL;
static const char* ans_name = NULL;
static int ans_no;

static inline void make_fifoname() {
    char* fifoname = malloc(16 * sizeof(char));
    sprintf(fifoname, "ans%d", getpid());
    ans_name = fifoname;
}

int open_fifo_ans() {
    assert(!ans_initialised);
    
    make_fifoname();

    int s = mkfifo(ans_name, ANS_PERMISSIONS);
    if (s != 0) {
        int err = errno;
        printf("Failed to make fifo %s: %s\n", ans_name, strerror(err));
        return err;
    }

    ans_no = open(ans_name, O_RDWR);
    if (ans_no == -1) {
        int err = errno;
        printf("Failed to open fifo %s: %s\n", ans_name, strerror(err));
        unlink(ans_name);
        return err;
    }

    ans_fifo = fdopen(ans_no, "r");
    if (ans_fifo == NULL) {
        int err = errno;
        printf("Failed to fdopen fifo %s: %s\n", ans_name, strerror(err));
        close(ans_no);
        unlink(ans_name);
        return err;
    }

    ans_initialised = true;

    if (!ans_atexit_set) {
        atexit(close_fifo_ans);
        ans_atexit_set = true;
    }

    return 0;
}

void close_fifo_ans() {
    fclose(ans_fifo);
    unlink(ans_name);

    ans_initialised = false;
}

int read_fifo_ans(const char** message_p) {
    char* buf = NULL;
    size_t n = 0;
    ssize_t s = getline(&buf, &n, ans_fifo); // blocks
    if (s <= 0) {
        free(buf);
        return errno;
    } else {
        *message_p = buf;
        printf("client received message %s\n", buf);
        return 0;
    }
}

int write_fifo_requests(const char* message) {
    int requestsno = open(REQUESTS_FIFO, O_WRONLY | O_NONBLOCK);
    if (requestsno == -1) {
        int err = errno;
        printf("Failed to open fifo requests: %s\n", strerror(err));
        return err;
    }

    write(requestsno, message, strlen(message));

    close(requestsno);
    return 0;
}