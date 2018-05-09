#include "options.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <locale.h>
#include <wchar.h>
#include <math.h>
#include <limits.h>

// <!--- OPTIONS (Resolve externals of options.h)
int o_show_help = false; // h, help
int o_show_usage = false; // usage
int o_show_version = false; // V, version

int o_max_seats = MAX_ROOM_SEATS; // s, seats-max
int o_max_client = MAX_CLI_SEATS; // c, client-max

// width of field Ti in SLOG
int o_twidth = 2;
// width of field seat in SLOG, SBOOK.
int o_seatwidth = WIDTH_SEAT;
// width of field pid in SLOG
int o_pidwidth = WIDTH_PID;
// width of field N in SLOG
int o_nwidth = 2;
// total width of field preferred in SLOG
int o_prefwidth = WIDTH_SEAT * MAX_CLI_SEATS + (MAX_CLI_SEATS - 1);

int o_seats;
int o_workers;
int o_time;
// ----> END OF OPTIONS



static const struct option long_options[] = {
	// general options
	{HELP_LFLAG,              no_argument, &o_show_help,       true},
	{USAGE_LFLAG,             no_argument, &o_show_usage,      true},
	{VERSION_LFLAG,           no_argument, &o_show_version,    true},
	// other options
	{SEATSMAX_LFLAG,    required_argument, NULL,      SEATSMAX_FLAG},
	{CLIENTMAX_LFLAG,   required_argument, NULL,     CLIENTMAX_FLAG},
	// end of options
	{0, 0, 0, 0}
	// format: {const char* lflag, int has_arg, int* flag, int val}
	//   {lflag, [no|required|optional]_argument, &var, true|false}
	// or
	//   {lflag, [no|required|optional]_argument, NULL, flag}
};

// enforce POSIX with +
static const char* short_options = "+hVs:c:";
// x for no_argument, x: for required_argument, x:: for optional_argument

static const wchar_t* version = L"FEUP SOPE 2017-2018\n"
	"cinema 1.0 -- (server)\n"
	"  Bruno Carvalho     up201606517\n"
	"      João Alves     up201605236\n"
	"     João Agulha     up201607930\n"
	"\n";

static const wchar_t* usage = L"usage: server [option]... seats threads time\n"
	"\n"
	"General:\n"
	"  -h, --help,           \n"
	"      --usage           Show this message and exit\n"
	"  -V, --version         Show 'version' message and exit\n"
	"\n"
	"Options:\n"
	"  -s, --seats-max=N     Set MAX_ROOM_SEATS to N\n"
	"  -c, --client-max=N    Set MAX_CLI_SEATS to N\n"
	"\n";

static void clear_options() {
	// At the moment no end-of-program cleanup is required here.
}

static void print_all() {
	setlocale(LC_ALL, "");
	wprintf(usage);
	wprintf(version);
	exit(EXIT_SUCCESS);
}

static void print_usage() {
	setlocale(LC_ALL, "");
	wprintf(usage);
	exit(EXIT_SUCCESS);
}

static void print_version() {
	setlocale(LC_ALL, "");
	wprintf(version);
	exit(EXIT_SUCCESS);
}

static void print_numpositional(int n) {
	setlocale(LC_ALL, "");
	wprintf(L"server: Error: Expected 3 positional arguments, but got %d.\n%S", n, usage);
	exit(EXIT_SUCCESS);
}

static void print_badpositional(int i) {
	setlocale(LC_ALL, "");
	wprintf(L"server: Error: Positional argument #%d is invalid.\n%S", i, usage);
	exit(EXIT_SUCCESS);
}

static void print_badarg(const char* opt) {
	setlocale(LC_ALL, "");
	wprintf(L"server: Error: Bad argument for options %s.\n%S", opt, usage);
	exit(EXIT_SUCCESS);
}

static int parse_int(const char* str, int* store) {
	char* endp;
	long result = strtol(str, &endp, 10);

	if (endp == str || errno == ERANGE || result >= INT_MAX || result <= INT_MIN) {
		return 1;
	} else {
		*store = (int)result;
		return 0;
	}
}

/**
 * Standard unix main's argument parsing function.
 */
int parse_args(int argc, char** argv) {
	// Uncomment to disable auto-generated error messages for options:
	// opterr = 0;

	// If there are no args, print usage and version messages and exit
	if (argc == 1) {
		print_all();
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
		case SEATSMAX_FLAG:
			if (parse_int(optarg, &o_max_seats) != 0) {
				print_badarg(SEATSMAX_LFLAG);
			}
			break;
		case CLIENTMAX_FLAG:
			if (parse_int(optarg, &o_max_client) != 0) {
				print_badarg(CLIENTMAX_LFLAG);
			}
			break;
		case '?':
		case ':':
		default: // getopt_long already printed an error message.
			print_usage();
		}
	} // End [Options Loop] while

	if (o_show_help || o_show_usage) {
		print_usage();
	}

	if (o_show_version) {
		print_version();
	}

	// Exactly 3 positional parameters are expected
	int num_positional = argc - optind;

	if (num_positional == 3) {
		if (parse_int(argv[optind++], &o_seats) != 0) {
			print_badpositional(1);
		}
		if (parse_int(argv[optind++], &o_workers) != 0) {
			print_badpositional(2);
		}
		if (parse_int(argv[optind++], &o_time) != 0) {
			print_badpositional(3);
		}
		if (o_seats > o_max_seats || o_seats <= 0) {
			print_badpositional(1);
		}
		if (o_workers <= 0) {
			print_badpositional(2);
		}
		if (o_time <= 0) {
			print_badpositional(3);
		}
	} else {
		print_numpositional(num_positional);
	}

	o_twidth = (int)fmax(2.0, floor(log10(o_workers) + 1.0));
	o_nwidth = (int)fmax(2.0, floor(log10(o_max_client) + 1.0));
	o_seatwidth = (int)fmax(o_seatwidth, floor(log10(o_seats) + 1.0));
    o_prefwidth = o_seatwidth * o_max_client + (o_max_client - 1);

	atexit(clear_options);
	return 0;
}
