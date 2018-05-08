#include "options.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <locale.h>
#include <wchar.h>


// <!--- OPTIONS (Resolve externals of options.h)
int o_show_help = false; // h, help
int o_show_usage = false; // usage
int o_show_version = false; // V, version

int o_sanitize = false; // sanitize

int o_time;
int o_number;
char* o_preferred = NULL;
// ----> END OF OPTIONS



static const struct option long_options[] = {
    // general options
    {HELP_LFLAG,              no_argument, &o_show_help,       true},
    {USAGE_LFLAG,             no_argument, &o_show_usage,      true},
    {VERSION_LFLAG,           no_argument, &o_show_version,    true},
    // other options
    {SANITIZE_LFLAG,          no_argument, &o_sanitize,        true},
    {NOSANITIZE_LFLAG,        no_argument, &o_sanitize,       false},
    // end of options
    {0, 0, 0, 0}
    // format: {const char* lflag, int has_arg, int* flag, int val}
    //   {lflag, [no|required|optional]_argument, &var, true|false}
    // or
    //   {lflag, [no|required|optional]_argument, NULL, flag}
};

// enforce POSIX with +
static const char* short_options = "+hV";
// x for no_argument, x: for required_argument, x:: for optional_argument

static const wchar_t* version = L"FEUP SOPE 2017-2018\n"
    "cinema 1.0 -- (client)\n"
    "  Bruno Carvalho     up201606517\n"
    "      João Alves     up201605236\n"
    "     João Agulha     up201607930\n"
    "\n";

static const wchar_t* usage = L"usage: client [option]... time number preference_list\n"
    "preference_list is a space-separated strings of ints\n"
    "\n"
    "General:\n"
    "  -h, --help,           \n"
    "      --usage           Show this message and exit\n"
    "  -V, --version         Show 'version' message and exit\n"
    "\n"
    "Options:\n"
    "      --sanitize,       \n"
    "      --no-sanitize     Sanitize preference_list\n"
    "\n";

/**
 * Free all resources allocated to contain options by parse_args.
 */
static void clear_options() {
    free(o_preferred);
}

/**
 * Prints the program's usage message to standard out.
 */
void print_usage() {
    setlocale(LC_ALL, "");
    wprintf(usage);
}

/**
 * Prints the program version message to standard out.
 */
void print_version() {
    setlocale(LC_ALL, "");
    wprintf(version);
}

/**
 * Prints the incorrect number of positional arguments error message.
 */
void print_numpositional(int n) {
    setlocale(LC_ALL, "");
    wprintf(L"Error: Expected 3 positional arguments, but got %d.\n%S", n, usage);
}

/**
 * Standard unix main's argument parsing function.
 */
int parse_args(int argc, char** argv) {
    // Uncomment to disable auto-generated error messages for options:
    // opterr = 0;

    // If there are no args, print usage and version messages and exit
    if (argc == 1) {
        print_usage();
        print_version();
        return -1;
    }

    // Standard getopt_long Options Loop
    while (true) {
        int c, lindex = 0;

        c = getopt_long(argc, argv, short_options,
            long_options, &lindex);

        if (c == -1) break; // No more options

        switch (c) {
        case 0:
            // If this option set a flag, do nothing else now.
            if (long_options[lindex].flag == NULL) {
                // ... Long option with non-null var ...
                // getopt_long already set the flag.
                // Inside this is normally a nested switch
                // *** Access option using long_options[index].*
                // (or struct option opt = long_options[index])
                // optarg - contains value of argument
                // optarg == NULL if no argument was provided
                // to a field with optional_argument.
            }
            break;
        case HELP_FLAG:
            o_show_help = true;
            break;
        case VERSION_FLAG:
            o_show_version = true;
            break;
        case '?':
        case ':':
        default:
            // getopt_long already printed an error message.
            print_usage();
            return -1;
        }
    } // End [Options Loop] while

    if (o_show_help || o_show_usage) {
        print_usage();
        return -1;
    }

    if (o_show_version) {
        print_version();
        return -1;
    }

    // Exactly 3 positional arguments are expected
    int num_positional = argc - optind;

    if (num_positional == 3) {
        o_time    = strtol(argv[optind++], NULL, 10);
        o_number  = strtol(argv[optind++], NULL, 10);
        o_preferred = strdup(argv[optind++]);
        // TODO: Check errno after each call
    } else {
        print_numpositional(num_positional);
        return -1;
    }

    atexit(clear_options);

    return 0;
}
