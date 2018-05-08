#include "requests.h"
#include "options.h"
#include "debug.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>

static inline int substring(char* buf, const char* str, int off_s, int off_e) {
    stpcpy(stpncpy(buf, str + off_s, off_e - off_s), "\0");
    return 0;
}

static void print_request(request_t* request) {
    char str = malloc(256 * sizeof(char));

    sprintf(str, "[client %d]\n[seats %d %d]\n[error %d]\n[msg %s]\n[rest %s]\n",
        request->client, request->number, request->total,
        request->error, request->message, request->rest);

    write(STDOUT_FILENO, str, strlen(str));
}

static int validate_intarray(const char* rest) {
    static const char* const intarray_pattern = "^ *[0-9]+( +[0-9]+)* *$";
    regex_t regex;
    regcomp(&regex, intarray_pattern, REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
    regmatch_t match[1];

    if (regexec(&regex, rest, 1, match, 0) == 0) {
        regfree(&regex);
        return 1;
    } else {
        regfree(&regex);
        return 0;
    }
}

static int parse_ints(const char* str, int** ints_p, int* total_p) {
    assert(ints_p != NULL && total_p != NULL);
    if (!validate_intarray(str)) return 1;

    int ints_size = 4, total = 0;
    int* ints = malloc(ints_size * sizeof(int));
    char* end = NULL;

    long res = strtol(str, &end, 10);
    
    do {
        if (total == ints_size) {
            ints_size *= 2;
            ints = realloc(ints, ints_size * sizeof(int));
        }

        ints[total++] = res;
        str = end;
        res = strtol(str, &end, 10);
    } while (str != end);

    *ints_p = ints;
    *total_p = total;
    return 0;
}

void free_request(request_t* request) {
    free(request->preferred);
    free(request->reserved);
    free((char*)request->message);
    free((char*)request->rest);
    free((char*)request->fifoname);
    free(request);
}

request_t* make_request() {
    request_t* request = malloc(sizeof(request_t));
    request->client = getpid();
    request->number = o_number;
    request->rest = strdup(o_preferred);
    request->error = 0;

    request->message = malloc((32 + strlen(request->rest)) * sizeof(char));
    sprintf(request->message, "%d %d %s\n",
        request->client, request->number, request->rest);

    int parse_s = parse_ints(request->rest, &request->preferred, &request->total);
    if (parse_s != 0) {
        free(request);
        return NULL;
    } else {
        if (PDEBUG) print_request(request);
        return request;
    }
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

static int parse_success(request_t* request) {
    static const char* const success_pattern = "^SUCCESS( +[0-9]+)+ *$";
    regex_t regex;
    regcomp(&regex, int_pattern, REG_EXTENDED | REG_NEWLINE);
    regmatch_t match[2];

    if (regexec(&regex, request->answer, 2, match, 0) == 0) {
        char* str = malloc(strlen(request->answer) * sizeof(char));
        int off_s = match[1].rm_so, off_e = match[1].rm_eo;
        substring(str, request->answer, off_s, off_e);

        int t;
        parse_ints(str, &request->reserved, &t);

        if (t != request->number) {
            printf("client %d: error - expected %d reserved seats, got %d\n",
                getpid(), request->number, t);
            return 1;
        }

        return 0;
    } else {
        
    }
}

int parse_answer(request_t* request) {
    static const char* const success_pattern = "^SUCCESS( +[0-9]+)*$";
    regex_t regex;
    regcomp(&regex, int_pattern, REG_EXTENDED | REG_NEWLINE);
    regmatch_t match[2];

    if (regexec(&regex, request->answer, 2, match, 0) == 0) {
        char* buf = ;
        int off_s = match[1].rm_so, off_e = match[1].rm_eo;
        substring(buf, request->rest + offset, off_s, off_e);
        request->preferred[total++] = atoi(buf);
    }
}