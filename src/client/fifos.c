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
#define ANS_WHERE ""
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
    sprintf(fifoname, "%sans%d", ANS_WHERE, getpid());
    ans_name = fifoname;
}

int open_fifo_ans() {
    assert(!ans_initialised);
    
    make_fifoname();

    int s = mkfifo(ans_name, ANS_PERMISSIONS);
    if (s != 0) {
        printf("client %d: Failed to make fifo %s: %s\n", getpid(), ans_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    ans_no = open(ans_name, O_RDWR);
    if (ans_no == -1) {
        printf("client %d: Failed to open fifo %s: %s\n", getpid(), ans_name, strerror(errno));
        unlink(ans_name);
        exit(EXIT_FAILURE);
    }

    ans_fifo = fdopen(ans_no, "r");
    if (ans_fifo == NULL) {
        printf("client %d: Failed to fdopen fifo %s: %s\n", getpid(), ans_name, strerror(errno));
        close(ans_no);
        unlink(ans_name);
        exit(EXIT_FAILURE);
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
    free((char*)ans_name);

    ans_initialised = false;
}

int read_fifo_ans(const char** message_p) {
    char* buf = NULL;
    size_t n = 0;

    errno = 0;
    ssize_t s = getline(&buf, &n, ans_fifo); // blocks
    if (s <= 0 || errno == EINTR) {
        free(buf);
        exit(EXIT_FAILURE);
    } else {
        *message_p = buf;
        if (PDEBUG) printf("client %d in: %s\n", getpid(), buf);
        return 0;
    }
}

int write_fifo_requests(const char* message) {
    int requestsno = open(REQUESTS_FIFO, O_WRONLY | O_NONBLOCK);
    if (requestsno == -1) {
        printf("client %d: Failed to open fifo requests: %s\n", getpid(), strerror(errno));
        exit(EXIT_FAILURE);
    }

    write(requestsno, message, strlen(message));

    close(requestsno);
    return 0;
}
