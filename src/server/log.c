#include "log.h"
#include "requests.h"
#include "options.h"
#include "debug.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>

#define SLOG_NAME   "slog.txt"
#define SBOOK_NAME "sbook.txt"
#define CLOG_NAME   "clog.txt"
#define CBOOK_NAME "cbook.txt"

#define CLIENT_FILE_PERMISSIONS 0660

static bool slog_isopen = false;
static bool slog_atexit_set = false;
static FILE* slogfile = NULL;
static int slogno;

static bool sbook_isopen = false;
static bool sbook_atexit_set = false;
static FILE* sbookfile = NULL;
static int sbookno;

static inline void close_slog() {
	fclose(slogfile);
	slog_isopen = false;
}

static inline void close_sbook() {
	fclose(sbookfile);
	sbook_isopen = false;
}

int open_slog() {
	if (slog_isopen) return 0;

	slogfile = fopen(SLOG_NAME, "w");
	if (slogfile == NULL) {
		printf("server: Failed to open server log file %s: %s\n", SLOG_NAME, strerror(errno));
		exit(EXIT_FAILURE);
	}
	slogno = fileno(slogfile); // does not fail
	slog_isopen = true;

	if (!slog_atexit_set) {
		atexit(close_slog);
		slog_atexit_set = true;
	}

	return 0;
}

static int open_sbook() {
	if (sbook_isopen) return 0;

	sbookfile = fopen(SBOOK_NAME, "w");
	if (sbookfile == NULL) {
		printf("server: Failed to open server log file %s: %s\n", SBOOK_NAME, strerror(errno));
		exit(EXIT_FAILURE);
	}
	sbookno = fileno(sbookfile); // does not fail
	sbook_isopen = true;

	if (!sbook_atexit_set) {
		atexit(close_sbook);
		sbook_atexit_set = true;
	}
	return 0;
}

int clear_client_files() {
	int fd;

	// clear CLOG
	fd = open(CLOG_NAME, O_TRUNC | O_RDWR | O_CREAT, CLIENT_FILE_PERMISSIONS);
	if (fd == -1) {
		printf("server: Failed to clear client log file %s: %s\n", CLOG_NAME, strerror(errno));
		exit(EXIT_FAILURE);
	}
	close(fd);

	// clear CBOOK
	fd = open(CBOOK_NAME, O_TRUNC | O_RDWR | O_CREAT, CLIENT_FILE_PERMISSIONS);
	if (fd == -1) {
		printf("server: Failed to clear client log file %s: %s\n", CBOOK_NAME, strerror(errno));
		exit(EXIT_FAILURE);
	}
	close(fd);

	return 0;
}

char* stringify_intarray(const int* array, int size) {
	assert(array != NULL);

	char* str = calloc(10 * size, sizeof(char));
	char tmp[16];

	// Not even remotely the most efficient way to do this
	for (int i = 0; i < size; ++i) {
		if (i > 0) {
			sprintf(tmp, " %0*d", o_seatwidth, array[i]);
		} else {
			sprintf(tmp, "%0*d", o_seatwidth, array[i]);
		}
		strcat(str, tmp);
	}

	return str;
}

void slog_worker_open(int worker) {
	char str[64];

	sprintf(str, "%0*d-OPEN\n", o_twidth, worker);

	write(slogno, str, strlen(str));
}

void slog_worker_exit(int worker) {
	char str[64];

	sprintf(str, "%0*d-CLOSED\n", o_twidth, worker);

	write(slogno, str, strlen(str));
}

static void slog_success(const request_t* request) {
	char* preferred_str = stringify_intarray(request->preferred, request->total);
	char* reserved_str = stringify_intarray(request->reserved, request->number);

	int width = (int)fmax(strlen(preferred_str), o_prefwidth);
	char* str = malloc((64 + width + strlen(reserved_str)) * sizeof(char));

	sprintf(str, "%0*d-%0*d-%0*d: %-*s - %s\n",
		o_twidth, request->worker,
		o_pidwidth, request->client,
		o_nwidth, request->number,
		o_prefwidth, preferred_str,
		reserved_str);

	free(preferred_str);
	free(reserved_str);

	write(slogno, str, strlen(str));
	free(str);
}

static void slog_failure(const request_t* request) {
	char* preferred_str = stringify_intarray(request->preferred, request->total);

	int width = (int)fmax(strlen(preferred_str), o_prefwidth);
	char* str = malloc((64 + width + strlen(error_string(request->error))) * sizeof(char));

	sprintf(str, "%0*d-%0*d-%0*d: %-*s - %s\n",
		o_twidth, request->worker,
		o_pidwidth, request->client,
		o_nwidth, request->number,
		o_prefwidth, preferred_str,
		error_string(request->error));

	free(preferred_str);

	write(slogno, str, strlen(str));
	free(str);
}

static void slog_hard_failure(const request_t* request) {
	int width = (int)fmax(strlen(request->rest), o_prefwidth);
	char* str = malloc((64 + width + 3) * sizeof(char));

	sprintf(str, "%0*d-%0*d-%0*d: %-*s - %s\n",
		o_twidth, request->worker,
		o_pidwidth, request->client,
		o_nwidth, request->number,
		o_prefwidth, request->rest,
		error_string(request->error));

	write(slogno, str, strlen(str));
	free(str);
}

void slog_request(const request_t* request) {
	if (request->error == 0 && request->preferred != NULL && request->reserved != NULL) {
		slog_success(request);
	} else if (request->error != 0 && request->preferred != NULL) {
		slog_failure(request);
	} else if (PDEBUG) {
		slog_hard_failure(request);
	}
}

int sbook_log(const int* array, int size) {
	open_sbook();

	for (int i = 0; i < size; ++i) {
		fprintf(sbookfile, "%0*d\n", o_seatwidth, array[i]);
	}

	return 0;
}