#ifndef LOG_H___
#define LOG_H___

#include "requests.h"

int open_slog();

int clear_client_files();

char* stringify_intarray(const int* array, int size);

void slog_worker_open(int id);

void slog_worker_exit(int id);

void slog_request(const request_t* request);

int sbook_log(const int* array, int size);

#endif // LOG_H___
