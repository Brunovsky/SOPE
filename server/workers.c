#include "workers.h"
#include "options.h"
#include "wthreads.h"
#include "seats.h"
#include "queue.h"
#include "fifos.h"
#include "log.h"
#include "debug.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>

#define WORKER_EXIT_STRING "EXIT"

// The number of seats requested exceeds the per request maximum.
#define MAX  -1
#define MAX_S "MAX"

// The number of seats requested is invalid.
#define NST  -2
#define NST_S "NST"

// Some seat id is outside of range.
#define IID  -3
#define IID_S "IID"

// Another error.
#define ERR  -4
#define ERR_S "ERR"

// At least one of the seats is unavailable (?)
#define NAV  -5
#define NAV_S "NAV"

// Full house boys, sorry.
#define FUL  -6
#define FUL_S "FUL"

// (Internal) Bad format
#define BAD  -7
#define BAD_S "BAD FORMAT"

// Message lexing errors:
#define PARSE_OK             0
#define PARSE_FAILURE        1
#define PARSE_BAD_FORMAT     2

// Request meaning errors:
#define REQ_OK             0
#define REQ_FEW_SEATS      1
#define REQ_INVALID_SEAT   2
#define REQ_MAX_EXCEEDED   3

// Request processing outcomes:
#define PROC_OK            0
#define PROC_UNSATISFIED   1
#define PROC_FULL_HOUSE    2

typedef int client_t;

typedef struct {
    client_t client;
    int number, total;
    int* preferred;
    int* reserved;
    const char* message;
    const char* rest;
    const char* fifoname;
    int worker;
    int error;
} request_t;

static inline void free_request(request_t* request) {
    free(request->preferred);
    free(request->reserved);
    free((char*)request->message);
    free((char*)request->rest);
    free((char*)request->fifoname);
}

static inline request_t construct_request(int id, const char* message) {
    request_t request = {
        .client = 0,
        .number = 0,
        .total = 0,
        .preferred = NULL,
        .reserved = NULL,
        .message = message,
        .rest = NULL,
        .fifoname = NULL,
        .worker = id,
        .error = 0
    };
    return request;
}

static inline void print_request(request_t* request) {
    char str[2048];

    sprintf(str, "[%d][%d %d][%d][%d]\n%s\n%s\n%s\n",
        request->client, request->number, request->total,
        request->worker, request->error, request->message,
        request->rest, request->fifoname);

    write(STDOUT_FILENO, str, strlen(str));
}

static inline const char* error_code(int error) {
    switch (error) {
    case MAX: return MAX_S;
    case NST: return NST_S;
    case IID: return IID_S;
    case ERR: return ERR_S;
    case NAV: return NAV_S;
    case FUL: return FUL_S;
    default:  return BAD_S;
    }
}

static inline int substring(char* buf, const char* str, int off_s, int off_e) {
    stpcpy(stpncpy(buf, str + off_s, off_e - off_s), "\0");
    return 0;
}

static inline void make_fifoname(request_t* request) {
    char* fifoname = malloc(16 * sizeof(char));
    sprintf(fifoname, "ans%d", request->client);
    request->fifoname = fifoname;
}



static int answer_client_success(const request_t* request) {
    char* ints = stringify_intarray(request->reserved, request->number);
    char* str = malloc((16 + strlen(ints)) * sizeof(char));

    sprintf(str, "SUCCESS %s\n", ints);

    write_to_fifo(request->fifoname, str);

    free(ints);
    free(str);
    return 0;
}

static int answer_client_failure(const request_t* request) {
    char str[16];

    sprintf(str, "%d\n", request->error);

    write_to_fifo(request->fifoname, str);

    return 0;
}

static void success_log(const request_t* request) {
    slog_success_t success = {
        .pid = request->client,
        .number = request->number,
        .total = request->total,
        .preferred = request->preferred,
        .reserved = request->reserved
    };

    slog_success(request->worker, success);
    //answer_client_success(request);
}

static void failure_log(const request_t* request) {
    slog_failure_t fail = {
        .pid = request->client,
        .number = request->number,
        .total = request->total,
        .preferred = request->preferred,
        .error = error_code(request->error)
    };

    slog_failure(request->worker, fail);
    //answer_client_failure(request);
}

static void hard_fail_log(const request_t* request) {
    slog_hard_failure_t hard_fail = {
        .pid = request->client,
        .number = request->number,
        .rest = request->rest,
        .error = error_code(request->error)
    };

    // TODO: wrap this in a debug variable
    slog_hard_failure(request->worker, hard_fail);
}



static int validate_request(request_t* request) {
    if (request->total < request->number) {
        request->error = NST;
        failure_log(request);
        return REQ_FEW_SEATS;
    }

    for (int i = 0; i < request->total; ++i) {
        int seat = request->preferred[i];

        if (!is_valid_seat(seat)) {
            request->error = IID;
            failure_log(request);
            return REQ_INVALID_SEAT;
        }
    }

    if (request->number > o_max_client) {
        request->error = MAX;
        failure_log(request);
        return REQ_MAX_EXCEEDED;
    }

    return REQ_OK;
}

static void parse_request_ints(request_t* request) {
    // Using regex here is quite overkill...
    static const char* const int_pattern = "^ *([0-9]+) *";
    regex_t regex;
    regcomp(&regex, int_pattern, REG_EXTENDED | REG_NEWLINE);
    regmatch_t match[2];
    int offset = 0;

    int preferred_size = request->number, total = 0;
    request->preferred = malloc(preferred_size * sizeof(int));

    while (regexec(&regex, request->rest + offset, 2, match, 0) == 0) {
        if (total == preferred_size) {
            preferred_size *= 2;
            request->preferred = realloc(request->preferred, preferred_size * sizeof(int));
        }

        char buf[32];
        int off_s = match[1].rm_so, off_e = match[1].rm_eo;
        substring(buf, request->rest + offset, off_s, off_e);
        request->preferred[total++] = atoi(buf);

        printf("%d: %d (%s)\n", total, request->preferred[total-1], buf);

        offset += off_e;
    }

    request->total = total;
    if (PDEBUG) print_request(request);
    regfree(&regex);
}

static int parse_request(request_t* request) {
    // ECMA:  /^ *(\d+) +(\d+) +((\d+ +)*\d+) *$/gm
    static const char* const regex_pattern = "^ *([0-9]+) +([0-9]+) +(([0-9]+ +)*[0-9]+) *$";
    static const char* const bad_pattern = "^ *([0-9]+) +([0-9]+) +(.*)$";
    regex_t regex;
    regcomp(&regex, regex_pattern, REG_EXTENDED | REG_NEWLINE);
    regmatch_t match[4];

    if (regexec(&regex, request->message, 4, match, 0) == 0) {
        int off_s, off_e;
        char buf[32];

        off_s = match[1].rm_so; off_e = match[1].rm_eo;
        substring(buf, request->message, off_s, off_e);
        request->client = atoi(buf);

        printf("1. %d %d %s\n", off_s, off_e, buf);
        
        off_s = match[2].rm_so; off_e = match[2].rm_eo;
        substring(buf, request->message, off_s, off_e);
        request->number = atoi(buf);

        printf("2. %d %d %s\n", off_s, off_e, buf);

        off_s = match[3].rm_so; off_e = match[3].rm_eo;
        char* tmp = malloc((off_e - off_s + 1) * sizeof(char));
        substring(tmp, request->message, off_s, off_e);
        request->rest = tmp;
        make_fifoname(request);

        printf("3. %d %d %s\n", off_s, off_e, tmp);

        regfree(&regex);
        parse_request_ints(request);
        return validate_request(request);
    } else {
        regfree(&regex);
        // If bad_pattern matches then we can display an ERR message.
        // Otherwise its a hard error, the message does not match our core lex,
        // and there isn't much we can do about it.
        regex_t bad;
        regcomp(&bad, bad_pattern, REG_EXTENDED | REG_NEWLINE);

        if (regexec(&bad, request->message, 4, match, 0) == 0) {
            int off_s, off_e;
            char buf[32];

            off_s = match[1].rm_so; off_e = match[1].rm_eo;
            substring(buf, request->message, off_s, off_e);
            request->client = atoi(buf);
            
            off_s = match[2].rm_so; off_e = match[2].rm_eo;
            substring(buf, request->message, off_s, off_e);
            request->number = atoi(buf);

            off_s = match[3].rm_so; off_e = match[3].rm_eo;
            char* tmp = malloc((off_e - off_s + 1) * sizeof(char));
            substring(tmp, request->message, off_s, off_e);
            request->rest = tmp;
            make_fifoname(request);

            regfree(&bad);
            request->error = ERR;
            hard_fail_log(request);
            return PARSE_FAILURE;
        } else {
            request->rest = request->message;

            regfree(&bad);
            request->error = BAD;
            hard_fail_log(request);
            return PARSE_BAD_FORMAT;
        }
    }
}

static int process_request(request_t* request) {
    int reserved_so_far = 0;
    // once leeway < 0 we cannot satisfy the request
    int leeway = request->total - request->number;

    request->reserved = malloc(request->number * sizeof(int));

    // attempt to reserve seats iteratively
    for (int i = 0; i < request->total; ++i) {
        int seat = request->preferred[i];
        client_t client = request->client;

        if (is_seat_free(seat)) {
            switch (book_seat(seat, client)) {
            case SEAT_BOOKED:
                request->reserved[reserved_so_far++] = seat;
                break;
            case SEAT_IS_RESERVED:
            default:
                --leeway;
                break;
            }
        } else {
            --leeway;
        }

        // all reserved.
        if (reserved_so_far == request->number) break;
        // can no longer satisfy the request. stop.
        if (leeway < 0) break;
    }

    // free reserved seats upon failure
    if (reserved_so_far != request->number) {
        for (int i = 0; i < reserved_so_far; ++i) {
            int seat = request->reserved[i];
            int ret = free_seat(seat);
            assert(ret == SEAT_FREED);
        }

        request->error = NAV;
        failure_log(request);
        return PROC_UNSATISFIED;
    } else {
        success_log(request);
        return PROC_OK;
    }
}

static void process_message(int id, const char* message) {
    request_t request = construct_request(id, message);

    if (parse_request(&request) == 0) {
        process_request(&request);
    }

    free_request(&request);
}



static bool is_exit_command(const char* message) {
    bool is = strcmp(message, WORKER_EXIT_STRING) == 0;
    // do not free message if true, as it is static
    return is;
}

static int worker(int id) {
    slog_worker_open(id);

    while (true) {
        const char* message;
        read_message(&message);

        if (is_exit_command(message)) break;
        
        process_message(id, message);
    }

    slog_worker_exit(id);
    return 0;
}

int launch_workers() {
    for (int i = 1; i <= o_workers; ++i) {
        launch_wthread(worker, i);
    }

    return 0;
}

int terminate_workers() {
    for (int i = 0; i < o_workers; ++i) {
        write_message(WORKER_EXIT_STRING);
    }

    joinall_wthreads();
    return 0;
}