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
    int target_fifo = open(fifoname, O_RDONLY | O_NONBLOCK);
    if (target_fifo == -1) {
        // The fifo is closed
        return errno;
    }

    write(target_fifo, message, strlen(message));

    close(target_fifo);
    return 0;
}