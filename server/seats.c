#include "seats.h"
#include "options.h"

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

typedef pthread_mutex_t mutex_t;

typedef struct {
    int num;
    int client; // pid
    bool reserved;
    mutex_t mutex;
} seat_t;

static bool seats_initialised = false;
static bool seats_atexit_set = false;
static seat_t* seats = NULL;
static int seats_reserved = 0, seats_size = 0;

static void free_mutexes() {
    for (int i = 0; i < seats_size; ++i) {
        pthread_mutex_destroy(&seats[i].mutex);
    }
}

static void init_mutexes() {
    for (int i = 0; i < seats_size; ++i) {
        pthread_mutex_init(&seats[i].mutex, NULL);
    }
}

static void free_seats() {
    if (seats_initialised) {
        free_mutexes();
        free((void*)seats);
        seats = NULL;
        seats_reserved = seats_size = 0;
        seats_initialised = false;
    }
}

static void init_seats() {
    for (int i = 0; i < seats_size; ++i) {
        seats[i].num = i + 1;
        seats[i].client = NO_CLIENT_CODE;
        seats[i].reserved = false;
    }

    init_mutexes();
}

void setup_seats() {
    if (seats_initialised) free_seats(); // assume we come from a fork

    seats = malloc(o_seats * sizeof(seat_t));
    seats_size = o_seats;
    init_seats();

    if (!seats_atexit_set) {
        atexit(free_seats);
        seats_atexit_set = true;
    }
}

static void lock(int seat_num) {
    int i = seat_num - 1;
    pthread_mutex_lock(&seats[i].mutex);
}

static void unlock(int seat_num) {
    int i = seat_num - 1;
    pthread_mutex_unlock(&seats[i].mutex);
}

static void free_seat(int seat_num) {
    int i = seat_num - 1;
    seats[i].client = NO_CLIENT_CODE;
    seats[i].reserved = false;
    --seats_reserved;
}

static void reserve_seat(int seat_num, int client_id) {
    int i = seat_num - 1;
    seats[i].client = client_id;
    seats[i].reserved = true;
    ++seats_reserved;
}

static bool is_reserved(int seat_num) {
    int i = seat_num - 1;
    return seats[i].reserved;
}

bool is_seat_free(int seat_num) {
    if (seat_num < 1 || seat_num > o_seats) {
        return SEATS_ERR_INVALID_SEATNUM;
    }

    return is_reserved(seat_num);
}

int book_seat(int seat_num, int client_id) {
    if (seat_num < 1 || seat_num > o_seats) {
        return SEATS_ERR_INVALID_SEATNUM;
    }

    lock(seat_num);
    int ret = 0;

    if (is_reserved(seat_num)) {
        ret = SEAT_ERR_SEAT_RESERVED;
    } else {
        reserve_seat(seat_num, client_id);
    }

    DELAY();
    unlock(seat_num);
    return ret;
}

int free_seat(int seat_num) {
    if (seat_num < 1 || seat_num > o_seats) {
        return SEATS_ERR_INVALID_SEATNUM;
    }

    lock(seat_num);
    int ret = 0;

    if (is_reserved(seat_num)) {
        free_seat(seat_num, client_id);
    } else {
        ret = SEAT_ERR_SEAT_NOT_RESERVED;
    }

    DELAY();
    unlock(seat_num);
    return ret;
}

bool empty_house() {
    return seats_reserved == 0;
}

bool full_house() {
    return seats_reserved == seats_size;
}