#ifndef REGISTER_H___
#define REGISTER_H___

typedef int client_t;

// (contract) successful reservation
typedef struct {
    client_t pid;
    int number, total;
    int* preferred;
    int* reserved;
} slog_success_t;

// (contract) unsuccessful reservation
typedef struct {
    client_t pid;
    int number, total;
    int* preferred;
    const char* error;
} slog_failure_t;

// (contract) one seat ID is not an integer, or worse
typedef struct {
    client_t pid;
    int number;
    const char* rest;
    const char* error;
} slog_hard_failure_t;

int open_slog();

char* stringify_intarray(int* array, int size);

void slog_worker_open(int id);

void slog_worker_exit(int id);

void slog_success(int id, slog_success_t m);

void slog_failure(int id, slog_failure_t m);

void slog_hard_failure(int id, slog_hard_failure_t m);

int sbook_log(int* array, int size);

#endif // REGISTER_H___