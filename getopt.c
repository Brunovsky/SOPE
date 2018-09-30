/**
 * Here I tested the library function getopt_long from <getopt.h>,
 * used in both projects for parsing command line arguments.
 *
 * This is a standalone file, compile it as such.
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

static int verbose_flag = 0;

static int add_flag = 0;
static int append_flag = 0;
static int create_flag = 0;
static int delete_flag = 0;
static int file_flag = 0;
static int hello_flag = 0;

static char* create_arg = NULL;
static char* delete_arg = NULL;
static char* file_arg = NULL;
static char* hello_arg = NULL;

static int wtf_flag = 0;

static struct option long_options[] = {
	{"verbose",      no_argument, &verbose_flag, 1},
	{"brief",        no_argument, &verbose_flag, 0},
	{"wtf",    optional_argument, &wtf_flag,     1},
	{"add",          no_argument, NULL,        'a'},
	{"append",       no_argument, NULL,        'b'},
	{"create", required_argument, NULL,        'c'},
	{"delete", required_argument, NULL,        'd'},
	{"file",   required_argument, NULL,        'f'},
	{"hello",  optional_argument, NULL,        'h'},
	{0, 0, 0, 0}
};

int parse_args(int argc, char **argv) {
	int c;

	while (1) {
		int option_index = 0;

		c = getopt_long(argc, argv, "abc:d:f:h::f:",
			long_options, &option_index);

		if (c == -1) break;

		switch (c) {
			case 0:
				// If this option set a flag, do nothing else now.
				if (long_options[option_index].flag != 0)
					break;
				printf("option %s", long_options[option_index].name);
				if (optarg) printf(" with arg %s", optarg);
				else printf(" with no arg");
				printf("\n");
				break;

			case 'a':
				add_flag = 1;
				puts("option -a (add)");
				break;
			case 'b':
				append_flag = 1;
				puts("option -b (append)");
				break;
			case 'c':
				create_flag = 1;
				create_arg = optarg;
				printf("option -c with value '%s'\n", create_arg);
				break;
			case 'd':
				delete_flag = 1;
				delete_arg = optarg;
				printf("option -d with value '%s'\n", delete_arg);
				break;
			case 'f':
				file_flag = 1;
				file_arg = optarg;
				printf("option -f with value '%s'\n", optarg);
				break;
			case 'h':
				hello_flag = 1;
				hello_arg = optarg;
				printf("option -h with value '%s'\n", optarg);
				break;
			case '?':
				// getopt_long already printed an error message.
				puts("Exited on ?");
				return 1;
			default:
				puts("Exited on default");
				return 2;
		} // End options switch
	} // End options while

	if (verbose_flag) {
		puts("verbose flag is set");
	}
	if (wtf_flag) {
		puts("wtf flag is set");
	}

	if (optind < argc) {
		printf("non-option ARGV-elements:\n");
		while (optind < argc)
			printf("%s\n", argv[optind++]);
	}

	return 0;
}

int main(int argc, char** argv) {
	return parse_args(argc, argv);
}
