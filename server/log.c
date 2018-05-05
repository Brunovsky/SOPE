#include "log.h"
#include "options.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

static bool slog_isopen = false;
static FILE* slogfile = NULL;
static const char* const slogname = "slog.txt";
static int slogno;

static bool sbook_isopen = false;
static FILE* sbookfile = NULL;
static const char* const sbookname = "sbook.txt";
static int sbookno;

static inline void close_slog() {
	fclose(slogfile);
}

static inline void close_sbook() {
	fclose(sbookfile);
}

/**
 * Open the slog.txt file.
 * Register a function to close it at exit...
 */
int open_slog() {
	if (slog_isopen) return 0;
	slogfile = fopen(slogname, "w");
	if (slogfile == NULL) {
		int err = errno;
		printf("server: Failed to open log file slog.txt: %s\n", strerror(err));
		return err;
	}
	slogno = fileno(slogfile);
	atexit(close_slog);
	slog_isopen = true;
	return 0;
}

/**
 * Open the sbook.txt file.
 * Register a function to close it at exit...
 */
int open_sbook() {
	if (sbook_isopen) return 0;
	sbookfile = fopen(sbookname, "w");
	if (sbookfile == NULL) {
		int err = errno;
		printf("server: Failed to open log file sbook.txt: %s\n", strerror(err));
		return err;
	}
	sbookno = fileno(sbookfile);
	atexit(close_sbook);
	sbook_isopen = true;
	return 0;
}

char* stringify_intarray(int* array, int size) {
	char* str = calloc(10 * size, sizeof(char));
	char tmp[16];

	for (int i = 0; i < size; ++i) {
		if (i > 0) sprintf(tmp, " %06d", array[i]);
		else sprintf(tmp, "%06d", array[i]);
		strcat(str, tmp);
	}

	return str;
}

void slog_worker_open(int id) {
	char str[64];

	sprintf(str, "%08d-OPEN\n", id);

	write(slogno, str, strlen(str));
}

void slog_worker_exit(int id) {
	char str[64];

	sprintf(str, "%08d-CLOSED\n", id);

	write(slogno, str, strlen(str));
}

void slog_success(int id, slog_success_t m) {
	char* preferred_str = stringify_intarray(m.preferred, m.total);
	char* reserved_str = stringify_intarray(m.reserved, m.number);
	char* str = malloc((64 + strlen(preferred_str)
		+ strlen(reserved_str)) * sizeof(char));

	sprintf(str, "%08d-%04d-%04d: %s: %s\n",
		id, m.pid, m.number, preferred_str, reserved_str);

	free(preferred_str);
	free(reserved_str);

	write(slogno, str, strlen(str));
	free(str);
}

void slog_failure(int id, slog_failure_t m) {
	char* preferred_str = stringify_intarray(m.preferred, m.total);
	char* str = malloc((64 + strlen(preferred_str)
		+ strlen(m.error)) * sizeof(char));

	sprintf(str, "%08d-%04d-%04d: %s: %s\n",
		id, m.pid, m.number, preferred_str, m.error);

	free(preferred_str);

	write(slogno, str, strlen(str));
	free(str);
}

void slog_hard_failure(int id, slog_hard_failure_t m) {
	char* str = malloc((64 + strlen(m.rest)
		+ strlen(m.error)) * sizeof(char));

	sprintf(str, "%08d-%04d-%04d: %s: %s\n",
		id, m.pid, m.number, m.rest, m.error);

	write(slogno, str, strlen(str));
	free(str);
}

int sbook_log(int* array, int size) {
	int s = open_sbook();
	if (s != 0) return s;

	for (int i = 0; i < size; ++i) {
		fprintf(sbookfile, "%04d\n", array[i]);
	}

	return 0;
}