#ifndef SEATS_H___
#define SEATS_H___

#include <stdbool.h>

#define ERR_MAX -1
#define ERR_NST -2
#define ERR_IID -3
#define ERR_ERR -4
#define ERR_NAV -5
#define ERR_FUL -6

#define SEATS_ERR_INVALID_SEATNUM    1
#define SEATS_ERR_SEAT_IS_RESERVED   2
#define SEATS_ERR_SEAT_NOT_RESERVED  3

void setup_seats();

bool is_seat_free(int seat_num);

int book_seat(int seat_num, int client_id);

int free_seat(int seat_num);

#endif // SEATS_H___