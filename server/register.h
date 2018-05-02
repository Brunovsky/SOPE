#ifndef REGISTER_H___
#define REGISTER_H___

#define ENV_LOGFILENAME "LOGFILENAME"

int open_slog();

void timestamp_epoch();



// new worker open
void slog_worker_open();

// worker closing
void slog_worker_close();

// successful reservation
void slog_success(/* ... */);

// failure reservation
void slog_failure(/* ... */);

#endif // REGISTER_H___