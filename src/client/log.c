#include "log.h"
#include "debug.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define CLOG_PERMISSIONS  0660
#define CBOOK_PERMISSIONS 0660

static const char* const clogname = "clog.txt";
static int clogno;

static const char* const cbookname = "cbook.txt";
static int cbookno;

static inline void close_clog() {
    close(clogno);
}

static inline void close_cbook() {
    close(cbookno);
}

static int open_clog() {
    clogno = open(clogname, O_WRONLY | O_CREAT | O_APPEND, CLOG_PERMISSIONS);
    if (clogno == -1) {
        int err = errno;
        printf("client %d: Failed to open log file clog.txt: %s\n", getpid(), strerror(err));
        return err;
    }
    return 0;
}

static int open_cbook() {
    cbookno = open(cbookname, O_WRONLY | O_CREAT | O_APPEND, CBOOK_PERMISSIONS);
    if (cbookno == -1) {
        int err = errno;
        printf("client %d: Failed to open log file cbook.txt: %s\n", getpid(), strerror(err));
        return err;
    }
    return 0;
}

static int clog_success(request_t* request) {
    char str[64];

    for (int i = 0; i < request->number; ++i) {
        sprintf(str, "%05d %02d.%02d %04d\n",
            request->client, i + 1, request->number, request->reserved[i]);
        write(clogno, str, strlen(str));
    }

    return 0;
}

static int clog_failure(request_t* request) {
    char str[32];

    sprintf(str, "%05d %s\n",
        request->client, error_string(request->error));
    write(clogno, str, strlen(str));

    return 0;
}

int clog_log(request_t* request) {
    int open_s = open_clog();
    if (open_s != 0) return open_s;

    if (request->error == 0) {
        clog_success(request);
    } else {
        clog_failure(request);
    }

    close_clog();
    return 0;
}

int cbook_log(request_t* request) {
    if (request->error != 0) return 0;

    int open_s = open_cbook();
    if (open_s != 0) return open_s;

    for (int i = 0; i < request->number; ++i) {
        char str[16];
        sprintf(str, "%04d\n", request->reserved[i]);
        write(cbookno, str, strlen(str));
    }

    close_cbook();
    return 0;
}
