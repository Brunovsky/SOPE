#ifndef OPTIONS_H___
#define OPTIONS_H___

// NOTE: All externs are resolved in options.c
// Too add an option, do:
//      1. Add a block entry here
//      2. Resolve the extern in options.c
//      3. Update short_options and long_options in options.c
//      3. Update the usage string in options.c
//      4. Update parse_args in options.c
// then go on to add the option's functionality...



// <!--- REQUIRED MACROS
// Maximum value of the seats argument
// This can be redefined by command line option -s, --seats-max
#define MAX_ROOM_SEATS   9999

// Maximum number of seats that may be requested by one single client.
// This can be redefined by the command line option -c, --client-max
#define MAX_CLI_SEATS    15

// Register widths
#define WIDTH_PID        5
#define WIDTH_SEAT       4

// Delay for bookSeat and friends
#include <time.h>
#define DELAY() do { nanosleep(&(struct timespec){.tv_sec = 0, .tv_nsec = 1e6}, NULL); } while (0)
// ----> END OF REQUIRED MACROS



// <!--- GENERAL OPTIONS
// Show help/usage message and exit
#define HELP_FLAG 'h'
#define HELP_LFLAG "help"
extern int o_show_help;

// Show help/usage message and exit
#define USAGE_FLAG // NONE
#define USAGE_LFLAG "usage"
extern int o_show_usage;

// Show version message and exit
#define VERSION_FLAG 'V'
#define VERSION_LFLAG "version"
extern int o_show_version;
// ----> END OF GENERAL OPTIONS



// <!--- OTHER OPTIONS
// Set MAX_ROOM_SEATS from the command line
#define SEATSMAX_FLAG 's'
#define SEATSMAX_LFLAG "seats-max"
extern int o_max_seats;

// Set MAX_CLI_SEATS from the command line
#define CLIENTMAX_FLAG 'c'
#define CLIENTMAX_LFLAG "client-max"
extern int o_max_client;
// ----> END OF OTHER OPTIONS



// <!--- COMPUTED GLOBALS
extern int o_twidth;
extern int o_seatwidth;
extern int o_pidwidth;
extern int o_nwidth;
extern int o_prefwidth;
// ----> END OF COMPUTED GLOBALS



// <!--- SERVER POSITIONAL ARGUMENTS
// (POS #1) [REQUIRED] The number of seats. This means the seat numbers are [1...o_seats].
extern int o_seats;

// (POS #2) [REQUIRED] The number of threads (workers) to launch
extern int o_workers;

// (POS #3) [REQUIRED] The server's open time, in seconds
extern int o_time;
// ----> END OF SERVER POSITIONAL ARGUMENTS



// Parse command line arguments
int parse_args(int argc, char** argv);

#endif // OPTIONS_H___
