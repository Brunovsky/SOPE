#ifndef REQUESTS_H___
#define REQUESTS_H___

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

// Could not reserve sufficient seats.
#define NAV  -5
#define NAV_S "NAV"

// Full house boys, sorry.
#define FUL  -6
#define FUL_S "FUL"

// (Internal) Bad format.
#define BAD  -7
#define BAD_S "BAD"

typedef int client_t;

typedef struct {
    client_t client;
    int number, total;
    int* preferred;
    int* reserved;
    const char* message;
    const char* rest;
    const char* answer;
    int error;
} request_t;

void free_request(request_t* request);

request_t* make_request();

const char* error_string(int error);

int parse_answer(request_t* request);

#endif // REQUESTS_H___
