#include "seats.h"
#include "options.h"

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define NO_CLIENT_CODE -1

typedef pthread_mutex_t mutex_t;

struct seat_descriptor {
    int num;
    int client; // pid
    bool reserved;
};

typedef struct seat_descriptor Seat;
typedef struct seat_descriptor seat_t;

static bool seats_initialised = false;
static bool seats_atexit_set = false;
static seat_t* seats = NULL;
static mutex_t* mutexes = NULL;
static int seats_reserved = 0, seats_size = 0;

static void destroy_mutexes() {
    for (int i = 0; i < seats_size; ++i) {
        pthread_mutex_destroy(&mutexes[i]);
    }
}

static void init_mutexes() {    
    for (int i = 0; i < seats_size; ++i) {
        pthread_mutex_init(&mutexes[i], NULL);
    }
}

static void free_seats() {
    if (seats_initialised) {
        destroy_mutexes();
        free(mutexes);
        mutexes = NULL;
        free(seats);
        seats = NULL;
        seats_reserved = seats_size = 0;
        seats_initialised = false;
    }
}

static void init_seats() {
    seats = malloc(o_seats * sizeof(seat_t));
    seats_size = o_seats;

    for (int i = 0; i < seats_size; ++i) {
        seats[i].num = i + 1;
        seats[i].client = NO_CLIENT_CODE;
        seats[i].reserved = false;
    }

    mutexes = malloc(o_seats * sizeof(seat_t));
    init_mutexes();
}

void setup_seats() {
    if (seats_initialised) free_seats();

    init_seats();

    if (!seats_atexit_set) {
        atexit(free_seats);
        seats_atexit_set = true;
    }
}

static inline void lock_seat(Seat* seats, int seat_num) {
    int i = seat_num - 1;
    pthread_mutex_lock(&seats[i].mutex);
}

static inline void unlock_seat(Seat* seats, int seat_num) {
    int i = seat_num - 1;
    pthread_mutex_unlock(&seats[i].mutex);
}

/**
 * REQUIRED
 * @param  seats    [description]
 * @param  seat_num [description]
 * @return          [description]
 */
static int isSeatFree(Seat* seats, int seat_num) {
    // the entry point is is_seat_free, which validates seat_num and
    // redundantly passes in variable seats which is a global in this unit.
    
    lock_seat(seats, seat_num);

    int i = seat_num - 1;
    bool b = seats[i].reserved;

    int ret = b;

    DELAY();

    unlock_seat(seats, seat_num);

    return ret;
}

/**
 * REQUIRED
 * @param seats    [description]
 * @param seat_num [description]
 */
static void bookSeat(Seat* seats, int seat_num) {
    // the entry point is book_seat, which validates seat_num and
    // redundantly passes in variable seats which is a global in this unit.
    
    lock_seat(seats, seat_num);

    int i = seat_num - 1;
    bool b = seats[i].reserved;

    int ret = 0;

    if (b) {
        ret = SEATS_ERR_SEAT_IS_RESERVED;
    } else {
        // Effectively book seat
        int i = seat_num - 1;
        seats[i].client = client_id;
        seats[i].reserved = true;
    }

    DELAY();

    unlock_seat(seats, seat_num);

    //return ret;
}

/**
 * REQUIRED
 * @param seats    [description]
 * @param seat_num [description]
 */
static void freeSeat(Seat* seats, int seat_num) {
    // the entry point is free_seat, which validates seat_num and
    // redundantly passes in variable seats which is a global in this unit.
    
    lock_seat(seats, seat_num);

    int i = seat_num - 1;
    bool b = seats[i].reserved;

    int ret = 0;

    if (b) {
        ret = SEATS_ERR_SEAT_NOT_RESERVED;
    } else {
        // Effectively free seat
        int i = seat_num - 1;
        seats[i].client = NO_CLIENT_CODE;
        seats[i].reserved = false;
    }

    DELAY();

    unlock_seat(seats, seat_num);

    //return ret;
}

// Entry point for isSeatFree()
bool is_seat_free(int seat_num) {
    if (seat_num < 1 || seat_num > o_seats) {
        return SEATS_ERR_INVALID_SEATNUM;
    }

    return isSeatFree(seats, seat_num);
}

// Entry point for bookSeat()
int book_seat(int seat_num, int client_id) {
    if (seat_num < 1 || seat_num > o_seats) {
        return SEATS_ERR_INVALID_SEATNUM;
    }

    bookSeat(seats, seat_num, client_id);
    return 0;
}

// Entry point for freeSeat()
int free_seat(int seat_num) {
    if (seat_num < 1 || seat_num > o_seats) {
        return SEATS_ERR_INVALID_SEATNUM;
    }

    freeSeat(seats, seat_num);
    return 0;
}