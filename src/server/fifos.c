#include "fifos.h"

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
#define REQUESTS_PERMISSIONS 0660

/**
 * requests
 */
static bool requests_initialised = false;
static bool requests_atexit_set = false;
static FILE* requests_fifo = NULL;
static const char* const requests_name = REQUESTS_FIFO;
static int requests_no;

int open_fifo_requests() {
    int s = mkfifo(requests_name, REQUESTS_PERMISSIONS);
    if (s != 0) {
        int err = errno;
        printf("Failed to make fifo requests: %s\n", strerror(err));
        return err;
    }

    // Open fifo for reading and writing
    requests_no = open(requests_name, O_RDWR);
    if (requests_no == -1) {
        int err = errno;
        printf("Failed to open fifo requests: %s\n", strerror(err));
        return err;
    }

    requests_fifo = fdopen(requests_no, "r");
    if (requests_fifo == NULL) {
        int err = errno;
        printf("Failed to fdopen fifo requests: %s\n", strerror(err));
        return err;
    }

    requests_initialised = true;

    if (!requests_atexit_set) {
        atexit(close_fifo_requests);
        requests_atexit_set = true;
    }

    return 0;
}

void close_fifo_requests() {
    if (!requests_initialised) return;

    fclose(requests_fifo);
    unlink(requests_name);

    requests_initialised = false;
}

int read_fifo_requests(const char** message_p) {
    char* buf = NULL;
    size_t n = 0;
    ssize_t s = getline(&buf, &n, requests_fifo);
    if (s <= 0) {
        free(buf);
        return errno;
    } else {
        *message_p = buf;
        return 0;
    }
}

int write_to_fifo(const char* fifoname, const char* message) {
    int target_fifo = open(fifoname, O_WRONLY | O_NONBLOCK);
    if (target_fifo == -1) {
        int err = errno;
        printf("server: error opening fifo %s: %s\n", fifoname, strerror(errno));
        return err;
    }

    // race condition: we're racing the client process in this write.
    // if we get put on hold and client closes and/or destroys the fifo,
    // this write raises SIGPIPE, which we catch in signals.c.
    write(target_fifo, message, strlen(message));

    printf("server: wrote to fifo %d: %s\n", target_fifo, message);

    close(target_fifo);
    return 0;
}