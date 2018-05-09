#include "workers.h"
#include "requests.h"
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

static bool workers_running = false;
static bool workers_atexit_set = false;

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

static int answer_client(const request_t* request) {
    if (request->error == 0 && request->fifoname != NULL) {
        return answer_client_success(request);
    } else if (request->error != 0 && request->fifoname != NULL) {
        return answer_client_failure(request);
    } else {
        if (PDEBUG) printf("server: Badly formatted request\n");
    }
    return 1;
}

static void free_reserved_seats(request_t* request, int total_reserved) {
    for (int i = 0; i < total_reserved; ++i) {
        int seat = request->reserved[i];
        int ret = free_seat(seat);
        assert(ret == SEAT_FREED);
    }
}

static int process_request(request_t* request) {
    if (request->error != 0) return 0;

    int reserved_so_far = 0;
    // once leeway < 0 we cannot satisfy the request
    int leeway = request->total - request->number;
    // if seat, we got a full house error
    bool fullhouse = false;

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
            case SEAT_FULL_HOUSE:
                fullhouse = true;
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
        if (leeway < 0 || fullhouse) break;
    }

    // free reserved seats upon failure
    if (reserved_so_far != request->number) {
        free_reserved_seats(request, reserved_so_far);

        request->error = fullhouse ? FUL : NAV;
        return 1;
    } else {
        return 0;
    }
}

static bool is_exit_command(const char* message) {
    bool is = strcmp(message, WORKER_EXIT_STRING) == 0;
    // do not free message if true, as it is static
    return is;
}

static void terminate_workers() {
    if (!workers_running) return;

    for (int i = 1; i <= o_workers; ++i) {
        write_message(WORKER_EXIT_STRING);
    }

    joinall_wthreads();
    workers_running = false;
}

static int worker(int id) {
    slog_worker_open(id);

    while (true) {
        const char* message;
        read_message(&message);

        if (is_exit_command(message)) break;

        request_t* request = make_request(id, message);

        process_request(request);
        answer_client(request);
        slog_request(request);
        free_request(request);
    }

    slog_worker_exit(id);
    return 0;
}

int launch_workers() {
    for (int i = 1; i <= o_workers; ++i) {
        launch_wthread(worker, i);
    }
    workers_running = true;

    if (!workers_atexit_set) {
        atexit(terminate_workers);
        workers_atexit_set = true;
    }

    return 0;
}
