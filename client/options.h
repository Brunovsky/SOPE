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
#define SANITIZE_FLAG // NONE
#define SANITIZE_LFLAG "sanitize"
extern int o_sanitize;
// ----> END OF OTHER OPTIONS



// <!--- CINEMA POSITIONAL ARGUMENTS
// (POS #1) [REQUIRED] The number of seats.
// This means the seat numbers are [1...o_seats].
extern int o_timeout;

// (POS #2) [REQUIRED] The number of threads (workers) to launch
extern int o_number;

// (POS #3) [REQUIRED] The server's open time, in seconds
extern int o_time;
// ----> END OF SIMGREP POSITIONAL ARGUMENTS



// Parse command line arguments
int parse_args(int argc, char** argv);

#endif // OPTIONS_H___