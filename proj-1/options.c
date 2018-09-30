#include "options.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <errno.h>
#include <locale.h>
#include <wchar.h>


// <!--- OPTIONS
int show_help = false; // h, help
int show_usage = false; // usage
int show_version = false; // V, version

int icase = false; // i, ignore-case
int listfiles = false; // l, files-with-matches
int linenumbers = false; // n, line-number
int countmatches = false; // c, count
int wordregex = false; // w, word-regex
int recurse = false; // r, recurse

int withfilename = true; // H, with-filename | h, no-filename
int extendedregex = false; // E, extended-regexp
int usecolor = false; // color, colour
int execgrep = false; // execgrep
int multiprocess = false; // P, fork, multiprocess
int multithreading = false; // T, threads, multithreads
int dirtraversal = false; // dir, fts

char* pattern = NULL;
char** files = NULL;
size_t number_of_files = 0;
// ----> END OF OPTIONS



static const struct option long_options[] = {
	// general options
	{HELP_LFLAG,              no_argument, &show_help,       true},
	{USAGE_LFLAG,             no_argument, &show_usage,      true},
	{VERSION_LFLAG,           no_argument, &show_version,    true},

	{ICASE_LFLAG,             no_argument, &icase,           true},
	{LISTFILES_LFLAG,         no_argument, &listfiles,       true},
	{LINENUMBERS_LFLAG,       no_argument, &linenumbers,     true},
	{COUNTMATCHES_LFLAG,      no_argument, &countmatches,    true},
	{WORDREGEX_LFLAG,         no_argument, &wordregex,       true},
	{RECURSE_LFLAG,           no_argument, &recurse,         true},

	{WITHFILENAME_LFLAG,      no_argument, &withfilename,    true},
	{NOFILENAME_LFLAG,        no_argument, &withfilename,   false},
	{EXTENDEDREGEX_LFLAG,     no_argument, &extendedregex,   true},
	{BASICREGEX_LFLAG,        no_argument, &extendedregex,  false},
	{COLOR_LFLAG1,            no_argument, &usecolor,        true},
	{COLOR_LFLAG2,            no_argument, &usecolor,        true},
	{EXECGREP_LFLAG,          no_argument, &execgrep,        true},
	{MULTIPROCESS_LFLAG1,     no_argument, &multiprocess,  true},
	{MULTIPROCESS_LFLAG2,     no_argument, &multiprocess,  true},
	{MULTITHREADING_LFLAG1,   no_argument, &multithreading,  true},
	{MULTITHREADING_LFLAG2,   no_argument, &multithreading,  true},
	{FTSTRAVERSAL_LFLAG,      no_argument, &dirtraversal,   false},
	{DIRTRAVERSAL_LFLAG,      no_argument, &dirtraversal,    true},
	// end of options
	{0, 0, 0, 0}
	// format: {const char* lflag, int has_arg, int* flag, int val}
	//   {lflag, [no|required|optional]_argument, &var, true|false}
	// or
	//   {lflag, [no|required|optional]_argument, NULL, flag}
};

// No + prefix (do not enforce POSIX)
static const char* short_options = "VilncwrHhGETP";
// x for no_argument, x: for required_argument, x:: for optional_argument

static const wchar_t* version = L"FEUP SOPE 2017-2018\n"
	"simgrep 1.1\n"
	"  Bruno Carvalho     up201606517\n"
	"      João Alves     up201605236\n"
	"     João Agulha     up201607930\n"
	"\n";

static const wchar_t* usage = L"usage: simgrep [option]... pattern [path]...\n"
	"A simple version of GNU grep.\n"
	"Search for a pattern in each line of the given files,\n"
	"and print all matching lines.\n"
	"\n"
	"General:\n"
	"      --help,           \n"
	"      --usage           Show this message and exit\n"
	"  -V, --version         Show 'version' message and exit\n"
	"\n"
	"Options:\n"
	"  -i, --ignore-case     Perform case insensitive regex search\n"
	"  -l, --files-with-matches\n"
	"                        Print only the names of FILEs\n"
	"                        containing matches\n"
	"  -n, --line-number     Print line number with output lines\n"
	"  -c, --count           Print only a count of matching lines per FILE\n"
	"  -w, --word-regex      Force the PATTERN to match only whole words\n"
	"  -r, --recurse         Recurse downwards all directories\n"
	"  -h, --no-filename     Suppress the file name prefix on output\n"
	"  -H, --with-filename   Print the filename for each match (default)\n"
	"  -E, --extended-regex  Use extended regular expression syntax\n"
	"  -G, --basic-regex     Use basic regular expression syntax (default)\n"
	"      --color,          \n"
	"      --colour          Print output colored, similar to grep.\n"
	"      --execgrep        Make a call to the GNU grep utility instead\n"
	"                        of using our internal grep function\n"
	"  -P, --fork,\n"
	"      --multiprocess    Spawn a new process for each subdirectory\n"
	"                        This has no affect without -r.\n"
	"  -T, --threads,        \n"
	"      --multithreads    Launch a new thread for each directory entry\n"
	"      --dir             Use the DIR API for traversal\n"
	"      --fts             Use the FTS API for traversal (default)\n"
	"\n"
	"The fastest general configuration uses -T and --fts.\n"
	"\n";

/**
 * Free all resources allocated to contain options by parse_args.
 */
static void clear_options() {
	free((void*)pattern);

	for (int i = 0; i < number_of_files; ++i) {
		free((void*)files[i]);
	}

	free((void*)files);
}

/**
 * Standard unix main's argument parsing function. Allocates resources
 * that are automatically freed at exit.
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
		case VERSION_FLAG:
			show_version = true;
			break;
		case ICASE_FLAG:
			icase = true;
			break;
		case LISTFILES_FLAG:
			listfiles = true;
			break;
		case LINENUMBERS_FLAG:
			linenumbers = true;
			break;
		case COUNTMATCHES_FLAG:
			countmatches = true;
			break;
		case WORDREGEX_FLAG:
			wordregex = true;
			break;
		case RECURSE_FLAG:
			recurse = true;
			break;
		case WITHFILENAME_FLAG:
			withfilename = true;
			break;
		case NOFILENAME_FLAG:
			withfilename = false;
			break;
		case BASICREGEX_FLAG:
			extendedregex = false;
			break;
		case EXTENDEDREGEX_FLAG:
			extendedregex = true;
			break;
		case MULTIPROCESS_FLAG:
			multiprocess = true;
			break;
		case MULTITHREADING_FLAG:
			multithreading = true;
			break;
		case '?':
		default:
			// getopt_long already printed an error message.
			print_usage();
			return EINVAL;
		}
	} // End [options] while

	if (show_help || show_usage) {
		print_usage();
		return -1;
	}

	if (show_version) {
		print_version();
		return -1;
	}

	if (optind < argc) {
		// <!--- POS #1 pattern
		size_t alloc_size = strlen(argv[optind]) + 1;
		pattern = malloc(alloc_size * sizeof(char));
		strcpy(pattern, argv[optind++]);
		// ----> End of POS #1

		if (optind < argc) {
			number_of_files = argc - optind;
			files = calloc(number_of_files + 1, sizeof(char*));
			
			int pos = 0;

			// <!--- POS ...
			do {
				alloc_size = strlen(argv[optind]) + 1;
				files[pos] = malloc(alloc_size * sizeof(char));
				strcpy(files[pos++], argv[optind++]);
			} while (optind < argc);
			// ----> End of POS ...
		}
	} else {
		print_nopattern();
		return EINVAL;
	}

	atexit(clear_options);

	return 0;
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
 * Prints the pattern error message to standard out.
 */
void print_nopattern() {
	setlocale(LC_ALL, "");
	wprintf(L"Expected a pattern argument, not supplied.\n%S", usage);
}
