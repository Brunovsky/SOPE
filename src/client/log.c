#include "log.h"
#include "options.h"
#include "requests.h"
#include "debug.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define CLOG_NAME   "clog.txt"
#define CBOOK_NAME "cbook.txt"
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
        printf("client %d: Failed to open client log file %s: %s\n", getpid(), CLOG_NAME, strerror(errno));
        return 1; // do not exit here.
    }
    return 0;
}

static int open_cbook() {
    cbookno = open(cbookname, O_WRONLY | O_CREAT | O_APPEND, CBOOK_PERMISSIONS);
    if (cbookno == -1) {
        printf("client %d: Failed to open client log file %s: %s\n", getpid(), CBOOK_NAME, strerror(errno));
        return 1; // do not exit here.
    }
    return 0;
}

static int clog_success() {
    char str[64];

    for (int i = 0; i < request->number; ++i) {
        sprintf(str, "%0*d %0*d.%0*d %0*d\n",
            o_pidwidth, request->client,
            o_xwidth, i + 1,
            o_nwidth, request->number,
            o_seatwidth, request->reserved[i]);
        write(clogno, str, strlen(str));
    }

    return 0;
}

static int clog_failure() {
    char str[32];

    sprintf(str, "%0*d %s\n",
        o_pidwidth, request->client, error_string(request->error));
    write(clogno, str, strlen(str));

    return 0;
}

int clog_log() {
    int s = open_clog();
    if (s != 0) return s;

    if (request->error == 0) {
        clog_success(request);
    } else {
        clog_failure(request);
    }

    close_clog();
    return 0;
}

int cbook_log() {
    if (request->error != 0) return 0;

    int s = open_cbook();
    if (s != 0) return s;

    for (int i = 0; i < request->number; ++i) {
        char str[16];
        sprintf(str, "%0*d\n", o_seatwidth, request->reserved[i]);
        write(cbookno, str, strlen(str));
    }

    close_cbook();
    return 0;
}
