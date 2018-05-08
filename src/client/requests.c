#include "requests.h"
#include "options.h"
#include "debug.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>

static inline int substring(char* buf, const char* str, int off_s, int off_e) {
    stpcpy(stpncpy(buf, str + off_s, off_e - off_s), "\0");
    return 0;
}

static int validate_intarray(const char* rest) {
    static const char* const intarray_pattern = "^ *[0-9]+( +[0-9]+)* *$";
    regex_t regex;
    regcomp(&regex, intarray_pattern, REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
    regmatch_t match[1];

    if (regexec(&regex, rest, 1, match, 0) == 0) {
        regfree(&regex);
        return 0;
    } else {
        regfree(&regex);
        return 1;
    }
}

static int parse_ints(const char* str, int** ints_p, int* total_p) {
    static const char* const pattern = "^ *([0-9]+) *";
    if (validate_intarray(str) != 0) return 1;

    regex_t regex;
    regcomp(&regex, pattern, REG_EXTENDED | REG_NEWLINE);
    regmatch_t match[2];
    int offset = 0;

    int ints_size = 4, total = 0;
    int* ints = malloc(ints_size * sizeof(int));

    while (regexec(&regex, str + offset, 2, match, 0) == 0) {
        if (total == ints_size) {
            ints_size *= 2;
            ints = realloc(ints, ints_size * sizeof(int));
        }

        char buf[16];
        int off_s = match[1].rm_so, off_e = match[1].rm_eo;
        substring(buf, str + offset, off_s, off_e);
        ints[total++] = atoi(buf);

        offset += off_e;
    }

    *ints_p = ints;
    *total_p = total;
    regfree(&regex);
    return 0;
}

void free_request(request_t* request) {
    free(request->preferred);
    free(request->reserved);
    free((char*)request->message);
    free((char*)request->rest);
    free((char*)request->answer);
    free(request);
}

request_t* make_request() {
    request_t* request = malloc(sizeof(request_t));
    request->client = getpid();
    request->number = o_number;
    request->error = 0;

    char* message = malloc((32 + strlen(o_preferred)) * sizeof(char));
    sprintf(message, "%d %d %s\n",
        request->client, request->number, o_preferred);
    request->message = message;
    request->rest = strdup(o_preferred);
    request->answer = NULL;

    int parse_s = parse_ints(request->rest, &request->preferred, &request->total);
    if (parse_s != 0) {
        free(request);
        return NULL;
    }
    
    return request;
}

const char* error_string(int error) {
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

static int validate_success(request_t* request) {
    static const char* const pattern = "^SUCCESS(( +[0-9]+)+) *$";
    regex_t regex;
    regcomp(&regex, pattern, REG_EXTENDED | REG_NEWLINE);
    regmatch_t match[2];

    if (regexec(&regex, request->answer, 2, match, 0) == 0) {
        char* str = malloc(strlen(request->answer) * sizeof(char));
        int off_s = match[1].rm_so, off_e = match[1].rm_eo;
        substring(str, request->answer, off_s, off_e);
        parse_ints(str, &request->reserved, &request->number);

        regfree(&regex);
        return 0;
    } else {
        regfree(&regex);
        return 1;
    }
}

static int validate_failure(request_t* request) {
    static const char* const pattern = "^ *(-?[0-9]+) *$";
    regex_t regex;
    regcomp(&regex, pattern, REG_EXTENDED | REG_NEWLINE);
    regmatch_t match[2];

    if (regexec(&regex, request->answer, 2, match, 0) == 0) {
        char str[16];
        int off_s = match[1].rm_so, off_e = match[1].rm_eo;
        substring(str, request->answer, off_s, off_e);
        request->error = atoi(str);

        regfree(&regex);
        return 0;
    } else {
        regfree(&regex);
        return 1;
    }
}

int parse_answer(request_t* request) {
    if (validate_success(request) == 0) {
        return 0;
    }

    if (validate_failure(request) == 0) {
        return 0;
    }

    printf("client %d: unrecognized answer message: %s\n", getpid(), request->answer);
    return 1;
}