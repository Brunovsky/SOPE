#include "fifos.h"
#include "signals.h"
#include "queue.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#define REQUESTS_NAME "requests"
#define REQUESTS_PERMISSIONS 0660

static bool requests_initialised = false;
static bool requests_atexit_set = false;
static FILE* requests_fifo = NULL;
static int requests_no;

static void destroy_requests() {
    int fd = open(REQUESTS_NAME, O_RDONLY | O_NONBLOCK);
    
    if (fd != -1) {
        close(fd);
        unlink(REQUESTS_NAME);
    }
}

static void close_fifo_requests() {
    if (!requests_initialised) return;

    fclose(requests_fifo);
    unlink(REQUESTS_NAME);

    requests_initialised = false;
}

int open_fifo_requests() {
    if (requests_initialised) return 0;
    destroy_requests();

    int s = mkfifo(REQUESTS_NAME, REQUESTS_PERMISSIONS);
    if (s != 0) {
        int err = errno;
        printf("Failed to make fifo requests: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }

    // Open fifo for reading and writing
    requests_no = open(REQUESTS_NAME, O_RDWR);
    if (requests_no == -1) {
        int err = errno;
        printf("Failed to open fifo requests: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }

    requests_fifo = fdopen(requests_no, "r");
    if (requests_fifo == NULL) {
        int err = errno;
        printf("Failed to fdopen fifo requests: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }

    requests_initialised = true;

    if (!requests_atexit_set) {
        atexit(close_fifo_requests);
        requests_atexit_set = true;
    }

    return 0;
}

int read_fifo_requests(const char** message_p) {
    char* buf = NULL;
    size_t n = 0;
    ssize_t s = getline(&buf, &n, requests_fifo);
    if (s <= 0 || errno == EINTR) {
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
        return errno;
    }

    // race condition: we're racing the client process in this write.
    // if we get put on hold and client closes and/or destroys the fifo,
    // this write raises SIGPIPE, which we catch in signals.c.
    write(target_fifo, message, strlen(message));

    close(target_fifo);
    return 0;
}

int fifo_read_loop() {
    while (true) {
        if (alarm_timeout()) return 0;

        const char* message;
        int s = read_fifo_requests(&message);

        if (s == 0) {
            write_message(message);
        }
    }
}