#ifndef SEATS_H___
#define SEATS_H___

#include <stdbool.h>

#define SEAT_BOOKED        (1 << 1)    
#define SEAT_FREED         (1 << 2)         
#define SEAT_IS_RESERVED   (1 << 5)
#define SEAT_NOT_RESERVED  (1 << 6)
#define SEAT_FULL_HOUSE    (1 << 9)

typedef int client_t;

void setup_seats();

bool is_valid_seat(int seat_num);

int is_seat_free(int seat_num);

int book_seat(int seat_num, client_t client_id);

int free_seat(int seat_num);

#endif // SEATS_H___