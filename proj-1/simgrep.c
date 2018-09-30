#include "simgrep.h"
#include "options.h"
#include "register.h"
#include "processes.h"
#include "make_grep.h"
#include "wthreads.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <regex.h>

#define CHILD 0
#define NO_FORK -1
#define PARENT
#define MAX_ITERATIONS 5

#define ANSI_COLOR_FILENAME     "\x1b[35m" // Purple, Magenta
#define ANSI_COLOR_LINENUMBER   "\x1b[32m" // Green
#define ANSI_COLOR_COLON        "\x1b[34m" // Blue
#define ANSI_COLOR_MATCH        "\x1b[31m" // Red
#define ANSI_COLOR_RESET        "\x1b[0m"



static void apply_wordregex(char** pattern) {
	char* new_pattern = calloc(10 + strlen(*pattern), sizeof(char));
	sprintf(new_pattern, "\\b%s\\b", *pattern);
	free(*pattern);
	*pattern = new_pattern;
}



static void write_filename(const char* file) {
	ssize_t file_len = strlen(file);

	char* str = calloc(file_len + 300, sizeof(char));
	if (usecolor)
		sprintf(str, ANSI_COLOR_FILENAME "%s\n" ANSI_COLOR_RESET, file);
	else
		sprintf(str, "%s\n", file);

	ssize_t str_len = strlen(str); // file + 1
	
	ssize_t written = write(STDOUT_FILENO, str, str_len);
	if (written != file_len) {
		// TODO ...
	}

	free((void*)str);
}

static void write_filename_line(const char* file, const char* line) {
	ssize_t file_len = strlen(file);
	ssize_t line_len = strlen(line);
	
	char* str = calloc(file_len + line_len + 300, sizeof(char));
	if (usecolor)
		sprintf(str, ANSI_COLOR_FILENAME "%s" ANSI_COLOR_COLON
			":" ANSI_COLOR_RESET "%s", file, line); // line includes newline
	else
		sprintf(str, "%s:%s", file, line); // line includes newline

	ssize_t str_len = strlen(str); // file + 1 + line (+1)
	
	ssize_t written = write(STDOUT_FILENO, str, str_len);
	if (written != str_len) {
		// TODO ...
	}

	free((void*)str);
}

static void write_filename_linenumber_line(const char* file,
	size_t linenumber, const char* line) {
	ssize_t file_len = strlen(file);
	ssize_t line_len = strlen(line);
	
	char* str = calloc(file_len + line_len + 300, sizeof(char));
	if (usecolor)
		sprintf(str, ANSI_COLOR_FILENAME "%s" ANSI_COLOR_COLON
			":" ANSI_COLOR_LINENUMBER "%lu" ANSI_COLOR_COLON
			":" ANSI_COLOR_RESET "%s", file, linenumber, line); // line includes newline
	else
		sprintf(str, "%s:%lu:%s", file, linenumber, line); // line includes newline

	ssize_t str_len = strlen(str); // file + 1 + log10(linenumber) + 1 + line (+1)
	
	ssize_t written = write(STDOUT_FILENO, str, str_len);
	if (written != str_len) {
		// TODO ...
	}

	free((void*)str);
}

static void write_filename_count(const char* file, size_t count) {
	ssize_t file_len = strlen(file);
	
	char* str = calloc(file_len + 300, sizeof(char));
	if (usecolor)
		sprintf(str, ANSI_COLOR_FILENAME "%s" ANSI_COLOR_COLON
			":" ANSI_COLOR_RESET "%lu\n", file, count);
	else
		sprintf(str, "%s:%lu\n", file, count);
	
	ssize_t str_len = strlen(str);
	
	ssize_t written = write(STDOUT_FILENO, str, str_len);
	if (written != str_len) {
		// TODO ...
	}

	free((void*)str);
}

static void write_line(const char* line) {
	ssize_t line_len = strlen(line);

	char* str = calloc(line_len + 300, sizeof(char));
	sprintf(str, "%s", line); // line includes newline

	ssize_t str_len = strlen(str); // line + 1
	
	ssize_t written = write(STDOUT_FILENO, str, str_len);
	if (written != line_len) {
		// TODO ...
	}

	free((void*)str);
}

static void write_linenumber_line(size_t linenumber, const char* line) {
	ssize_t line_len = strlen(line);
	
	char* str = calloc(line_len + 300, sizeof(char));
	if (usecolor)
		sprintf(str, ANSI_COLOR_LINENUMBER "%lu" ANSI_COLOR_COLON
			":" ANSI_COLOR_RESET "%s", linenumber, line); // line includes newline
	else
		sprintf(str, "%lu:%s", linenumber, line); // line includes newline

	ssize_t str_len = strlen(str); //  log10(linenumber) + 1 + line (+1)
	
	ssize_t written = write(STDOUT_FILENO, str, str_len);
	if (written != str_len) {
		// TODO ...
	}

	free((void*)str);
}

static void write_count(size_t count) {	
	char* str = calloc(300, sizeof(char));
	sprintf(str, "%lu\n", count);

	ssize_t str_len = strlen(str);
	
	ssize_t written = write(STDOUT_FILENO, str, str_len);
	if (written != str_len) {
		// TODO ...
	}

	free((void*)str);
}



static int grep_gnugrep_process_routine(void* void_file) {
	char* const* argv = make_grep((char*)void_file);
	log_command(argv);
	execvp("grep", argv);
	// We should not arrive here
	return errno;
}

static int grep_execute_gnugrep(char* file) {
	return spawn_process(grep_gnugrep_process_routine, (void*)file);
}

static int grep_execute(char* file) {
	// ***** Initiate Regex Units
	char* regex_pattern = strdup(pattern);

	// ** Regex flags
	int cflags = 0; // [[OPTIONS]] basicregex
	if (extendedregex) cflags |= REG_EXTENDED; // [[OPTIONS]] extendedregex
	if (icase) cflags |= REG_ICASE; // [[OPTIONS]] icase
	if (wordregex) apply_wordregex(&regex_pattern); // [[OPTIONS]] wordregex

	// ** Compile Regex, already validated
	regex_t regex;
	regcomp(&regex, regex_pattern, cflags);
	
	// ***** Initiate File
	FILE* fp = fopen(file, "r");
	if (fp == NULL) {
		log_failed_open_file(file);
		return 1;
	} else {
		log_open_file(file);
	}
	char* line = NULL;
	size_t len = 0, linenumber = 0, count = 0;
	ssize_t read;

	// ***** Main Loop
	while ((read = getline(&line, &len, fp)) != -1) {
		++linenumber;
		regmatch_t match[1];

		if (regexec(&regex, line, 1, match, 0) == 0) {
			if (listfiles) {// [[OPTIONS]] listfiles
				write_filename(file);
				break;
			}
			if (countmatches) { // [[OPTIONS]] countmatches
				++count;
				continue;
			}
			if (linenumbers) { // [[OPTIONS]] linenumbers
				if (withfilename)
					write_filename_linenumber_line(file, linenumber, line);
				else
					write_linenumber_line(linenumber, line);
				continue;
			}

			if (withfilename)
				write_filename_line(file, line);
			else
				write_line(line);
		}
	}

	if (countmatches) {
		if (withfilename)
			write_filename_count(file, count);
		else
			write_count(count);
	}

	// ***** Release and Exit
	free(line);
	fclose(fp);
	log_close_file(file);
	free(regex_pattern);
	regfree(&regex);
	return 0;
}



static void* grep_thread_routine(void* void_file) {
	log_new_thread();
	char* file = (char*)void_file;
	int ret = grep_execute(file);
	set_thread_return(ret);
	log_thread_finished();
	free((void*)file);
	return NULL;
}

static void* grep_thread_routine_gnugrep(void* void_file) {
	log_new_thread();
	char* file = (char*)void_file;
	int ret = grep_execute_gnugrep((void*)file);
	// We should get past this point.
	set_thread_return(ret);
	log_thread_finished();
	free((void*)file);
	return NULL;
}



static int grep_threadify(char* file) {
	char* filename = strdup(file);
	launch_thread(grep_thread_routine, (void*)filename);
	return 0;
}

static int grep_threadify_gnugrep(char* file) {
	char* filename = strdup(file);
	launch_thread(grep_thread_routine_gnugrep, (void*)filename);
	return 0;
}



int grep_stdin() {
	// ***** Initiate Regex Units
	char* regex_pattern = strdup(pattern);

	// ** Regex flags
	int cflags = 0; // [[OPTIONS]] basicregex
	if (extendedregex) cflags |= REG_EXTENDED; // [[OPTIONS]] extendedregex
	if (icase) cflags |= REG_ICASE; // [[OPTIONS]] icase
	if (wordregex) apply_wordregex(&regex_pattern); // [[OPTIONS]] wordregex

	// ** Compile Regex, already validated
	regex_t regex;
	regcomp(&regex, regex_pattern, cflags);
	
	// ***** Initiate File
	char* line = NULL;
	size_t len = 0, linenumber = 0, count = 0;
	ssize_t read;

	// ***** Main Loop
	while ((read = getline(&line, &len, stdin)) != -1) {
		++linenumber;
		regmatch_t match[1];

		if (regexec(&regex, line, 1, match, 0) == 0) {
			if (listfiles) {// [[OPTIONS]] listfiles
				write_filename("(standard input)");
				break;
			}
			if (countmatches) { // [[OPTIONS]] countmatches
				++count;
				continue;
			}
			if (linenumbers) { // [[OPTIONS]] linenumbers
				write_linenumber_line(linenumber, line);
				continue;
			}

			write_line(line);
		}
	}

	if (countmatches) {
		write_count(count);
	}

	// ***** Release and Exit
	free(line);
	free(regex_pattern);
	regfree(&regex);
	return 0;
}

int validate_regex_pattern() {
	char* regex_pattern = strdup(pattern);

	// ** Regex flags
	int cflags = 0; // [[OPTIONS]] basicregex
	if (extendedregex) cflags |= REG_EXTENDED; // [[OPTIONS]] extendedregex
	if (icase) cflags |= REG_ICASE; // [[OPTIONS]] icase
	if (wordregex) apply_wordregex(&regex_pattern); // [[OPTIONS]] wordregex

	// ** Compile Regex, already validated
	regex_t regex;
	int errcode = regcomp(&regex, regex_pattern, cflags);

	free(regex_pattern);
	if (errcode == 0) {
		return 0;
	} else {
		size_t errbuf_size = regerror(errcode, &regex, NULL, 0);
		char* errbuf = calloc(errbuf_size, sizeof(char));
		regerror(errcode, &regex, errbuf, errbuf_size);
		printf("simgrep: invalid regex: %s\n", errbuf);
		free(errbuf);
		return 1;
	}
}

int grep_file(char* file) {
	if (multithreading) {
		if (execgrep) {
			return grep_threadify_gnugrep(file);
		} else {
			return grep_threadify(file);
		}
	} else {
		if (execgrep) {
			return grep_execute_gnugrep(file);
		} else {
			return grep_execute(file);
		}
	}
}
