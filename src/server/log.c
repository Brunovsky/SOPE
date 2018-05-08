#include "log.h"
#include "requests.h"
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

char* stringify_intarray(const int* array, int size) {
	char* str = calloc(10 * size, sizeof(char));
	char tmp[16];

	for (int i = 0; i < size; ++i) {
		if (i > 0) sprintf(tmp, " %04d", array[i]);
		else sprintf(tmp, "%04d", array[i]);
		strcat(str, tmp);
	}

	return str;
}

void slog_worker_open(int worker) {
	char str[64];

	sprintf(str, "%05d-OPEN\n", worker);

	write(slogno, str, strlen(str));
}

void slog_worker_exit(int worker) {
	char str[64];

	sprintf(str, "%05d-CLOSED\n", worker);

	write(slogno, str, strlen(str));
}

static void slog_success(const request_t* request) {
	char* preferred_str = stringify_intarray(request->preferred, request->total);
	char* reserved_str = stringify_intarray(request->reserved, request->number);
	char* str = malloc((64 + strlen(preferred_str) + strlen(reserved_str)) * sizeof(char));

	sprintf(str, "%05d-%04d-%04d: %s: %s\n",
		request->worker, request->client, request->number,
		preferred_str, reserved_str);

	free(preferred_str);
	free(reserved_str);

	write(slogno, str, strlen(str));
	free(str);
}

static void slog_failure(const request_t* request) {
	char* preferred_str = stringify_intarray(request->preferred, request->total);
	char* str = malloc((64 + strlen(preferred_str) + 3) * sizeof(char));

	sprintf(str, "%05d-%04d-%04d: %s: %s\n",
		request->worker, request->client, request->number,
		preferred_str, error_string(request->error));

	free(preferred_str);

	write(slogno, str, strlen(str));
	free(str);
}

static void slog_hard_failure(const request_t* request) {
	char* str = malloc((64 + strlen(request->rest) + 3) * sizeof(char));

	sprintf(str, "%05d-%04d-%04d: %s: %s\n",
		request->worker, request->client, request->number,
		request->rest, error_string(request->error));

	write(slogno, str, strlen(str));
	free(str);
}

void slog_request(const request_t* request) {
	if (request->error == 0) {
		slog_success(request);
	} else if (request->preferred != NULL) {
		slog_failure(request);
	} else {
		slog_hard_failure(request);
	}
}

int sbook_log(const int* array, int size) {
	int s = open_sbook();
	if (s != 0) return s;

	for (int i = 0; i < size; ++i) {
		fprintf(sbookfile, "%04d\n", array[i]);
	}

	return 0;
}