#ifndef SEATS_H___
#define SEATS_H___

#include <stdbool.h>

#define NO_CLIENT_CODE -1

#define ERR_MAX -1
#define ERR_NST -2
#define ERR_IID -3
#define ERR_ERR -4
#define ERR_NAV -5
#define ERR_FUL -6

#define SEATS_ERR_INVALID_SEATNUM    1
#define SEATS_ERR_SEAT_RESERVED      2
#define SEATS_ERR_SEAT_NOT_RESERVED  3

/**
 * Call only after a fork()
 */
void setup_seats();

/**
 * Return 1 if the seat is free, false otherwise.
 * There is no guarantee the seat will remain free if immediately
 * followed by a call to book_set
 */
bool is_seat_free(int seat_num);

/**
 * Try to reserve a given seat for a given client.
 * @return 0 if successful as the seat was available and valid,
 *         otherwise an error code.
 */
int book_seat(int seat_num, int client_id)

/**
 * Free a previously reserved seat.
 * @return 0 if successfully freed,
 *         otherwise an error code.
 */
int free_seat(int seat_num);

/**
 * @return true if no seats have been reserved (empty house)
 */
bool empty_house();

/**
 * @return true if there are no available seats (full house)
 */
bool full_house();

#endif // SEATS_H___