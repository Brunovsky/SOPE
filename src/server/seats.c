#include "seats.h"
#include "options.h"
#include "log.h"
#include "debug.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

#define NO_CLIENT_CODE -1

typedef pthread_mutex_t mutex_t;

struct seat_descriptor {
    int num;
    client_t client;
    bool reserved;
};

typedef struct seat_descriptor Seat;
typedef struct seat_descriptor seat_t;

static bool seats_initialised = false;
static bool seats_atexit_set = false;
static seat_t* seats = NULL;
static mutex_t* mutexes = NULL;
static mutex_t reserved_mutex;
static int seats_reserved = 0, seats_size = 0;

static void destroy_mutexes() {
    for (int i = 0; i < seats_size; ++i) {
        pthread_mutex_destroy(&mutexes[i]);
    }
    pthread_mutex_destroy(&reserved_mutex);
}

static void init_mutexes() {
    for (int i = 0; i < seats_size; ++i) {
        pthread_mutex_init(&mutexes[i], NULL);
    }
    pthread_mutex_init(&reserved_mutex, NULL);
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

    mutexes = malloc(o_seats * sizeof(mutex_t));
    init_mutexes();

    seats_initialised = true;
}

static void log_reserved_seats() {
    int* array = malloc(seats_size * sizeof(int));
    int j = 0;

    for (int i = 0; i < seats_size; ++i) {
        if (seats[i].reserved) {
            array[j++] = seats[i].num;
        }
    }

    sbook_log(array, j);
    free(array);
}

void setup_seats() {
    if (seats_initialised) return;

    init_seats();

    if (!seats_atexit_set) {
        atexit(free_seats);
        atexit(log_reserved_seats);
        seats_atexit_set = true;
    }
}

bool is_valid_seat(int seat_num) {
    return seat_num >= 1 && seat_num <= o_seats;
}

static inline void lock_seat(int seat_num) {
    assert(is_valid_seat(seat_num));
    int i = seat_num - 1;
    pthread_mutex_lock(&mutexes[i]);
}

static inline void unlock_seat(int seat_num) {
    assert(is_valid_seat(seat_num));
    int i = seat_num - 1;
    pthread_mutex_unlock(&mutexes[i]);
}

static inline void lock_counter() {
    pthread_mutex_lock(&reserved_mutex);
}

static inline void unlock_counter() {
    pthread_mutex_unlock(&reserved_mutex);
}

static int full_house() {
    int ret;
    
    lock_counter();

    ret = seats_reserved == o_seats;
    
    unlock_counter();
    
    return ret;
}

/**
 * REQUIRED
 * The entry point is is_seat_free, which takes hold of the seat's
 * mutex, validates the argument seat_num, verifies the seat is
 * indeed available and then forwards variable seats which is a
 * global in this unit - just to match the required prototype -
 * to this function.
 * 
 * @returns true (1) if the seat is free,
 *          false (0) otherwise,
 *          after blocking with DELAY().
 * 
 * It is not this function's responsibility to validate the
 * arguments or avoid race conditions (@see is_seat_free) -- its
 * responsibility is to simulate complexity with DELAY().
 */
static int isSeatFree(Seat* seats, int seat_num) {
    int i = seat_num - 1;
    int ret = seats[i].reserved ? SEAT_IS_RESERVED : SEAT_IS_FREE;

    if (ret == SEAT_IS_RESERVED && full_house()) ret = SEAT_FULL_HOUSE;

    DELAY();

    return ret;
}

/**
 * REQUIRED
 * The entry point is book_seat, which takes hold of the seat's
 * mutex, validates the argument seat_num, verifies the seat is
 * indeed available and then forwards variable seats which is a
 * global in this unit - just to match the required prototype -
 * to this function.
 * 
 * @brief Effectively books the seat seat_num to client client_id
 *        and blocks with DELAY().
 * 
 * It is not this function's responsibility to validate the
 * arguments or avoid race conditions (@see book_seat) -- its
 * responsibility is to simulate complexity with DELAY().
 */
static void bookSeat(Seat* seats, int seat_num, int client_id) {
    int i = seat_num - 1;
    seats[i].client = client_id;
    seats[i].reserved = true;

    lock_counter();
    ++seats_reserved;
    unlock_counter();

    DELAY();
}

/**
 * REQUIRED
 * The entry point is free_seat, which takes hold of the seat's
 * mutex, validates the argument seat_num, verifies the seat is
 * indeed  reserved and then forwards variable seats which is a
 * global in this unit - just to match the required prototype -
 * to this function.
 * 
 * @brief Effectively frees the seat seat_num from a client
 *        and blocks with DELAY().
 * 
 * It is not this function's responsibility to validate the
 * arguments or avoid race conditions (@see free_seat) -- its
 * responsibility is to simulate complexity with DELAY().
 */
static void freeSeat(Seat* seats, int seat_num) {
    int i = seat_num - 1;
    seats[i].client = NO_CLIENT_CODE;
    seats[i].reserved = false;

    lock_counter();
    --seats_reserved;
    unlock_counter();

    DELAY();
}

int is_seat_free(int seat_num) {
    assert(is_valid_seat(seat_num));

    lock_seat(seat_num);

    int ret = isSeatFree(seats, seat_num);

    unlock_seat(seat_num);

    return ret;
}

int book_seat(int seat_num, client_t client_id) {
    assert(is_valid_seat(seat_num));

    lock_seat(seat_num);

    int i = seat_num - 1;
    bool b = seats[i].reserved;
    int ret;

    if (b) {
        if (full_house()) {
            ret = SEAT_FULL_HOUSE;
        } else {
            ret = SEAT_IS_RESERVED;
        }
    } else {
        bookSeat(seats, seat_num, client_id);
        ret = SEAT_BOOKED;
    }

    unlock_seat(seat_num);

    return ret;
}

int free_seat(int seat_num) {
    assert(is_valid_seat(seat_num));

    lock_seat(seat_num);

    int i = seat_num - 1;
    bool b = seats[i].reserved;
    int ret;

    if (b) {
        freeSeat(seats, seat_num);
        ret = SEAT_FREED;
    } else {
        ret = SEAT_IS_FREE;
    }

    unlock_seat(seat_num);

    return ret;
}
