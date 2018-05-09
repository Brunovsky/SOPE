#include "requests.h"
#include "seats.h"
#include "options.h"

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

static inline void make_fifoname(request_t* request) {
    char* fifoname = malloc(16 * sizeof(char));
    sprintf(fifoname, "ans%d", request->client);
    request->fifoname = fifoname;
}

static int validate_request(request_t* request) {
    if (request->number > o_max_client || request->total > o_max_client) {
        request->error = MAX;
        return 1;
    }

    if (request->total < request->number || request->number == 0) {
        request->error = NST;
        return 1;
    }

    for (int i = 0; i < request->total; ++i) {
        int seat = request->preferred[i];

        if (!is_valid_seat(seat)) {
            request->error = IID;
            return 1;
        }
    }

    return 0;
}

static int parse_message(request_t* request) {
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
        
        off_s = match[2].rm_so; off_e = match[2].rm_eo;
        substring(buf, request->message, off_s, off_e);
        request->number = atoi(buf);

        off_s = match[3].rm_so; off_e = match[3].rm_eo;
        char* tmp = malloc((off_e - off_s + 1) * sizeof(char));
        substring(tmp, request->message, off_s, off_e);
        request->rest = tmp;

        parse_ints(request->rest, &request->preferred, &request->total);
        make_fifoname(request);

        regfree(&regex);
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
            request->error = ERR;

            regfree(&bad);
            return 1;
        } else {
            request->rest = strdup(request->message);
            request->error = BAD;

            regfree(&bad);
            return 1;
        }
    }
}

void free_request(request_t* request) {
    free(request->preferred);
    free(request->reserved);
    free((char*)request->message);
    free((char*)request->rest);
    free((char*)request->answer);
    free((char*)request->fifoname);
    free(request);
}

request_t* make_request(int worker, const char* message) {
    request_t* request = calloc(1, sizeof(request_t));

    request->message = message;
    request->worker = worker;
    
    parse_message(request);

    return request;
}
