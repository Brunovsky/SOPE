#ifndef REGISTER_H___
#define REGISTER_H___

#define ENV_LOGFILENAME "LOGFILENAME"

int open_logfile();

void timestamp_epoch();



// general string
void log_general(const char* str);

// general string array
void log_command(char* const* argv);

// child process created
void log_new_process(int child_pid);

// this process exiting
void log_process_exit();

// fork failed
void log_failed_fork(int err);

// waited child pid
void log_waitpid_resolved(int child_pid, int status, int children_left);

// signal received
void log_signal_received(int signal);

// signal sent to process
void log_signal_sent(int signal, int target_process_pid);

// signal sent to process group
void log_signal_sent_group(int signal, int target_process_pgid);

// opened file
void log_open_file(const char* file);

// closed file
void log_close_file(const char* file);

// failed to open file
void log_failed_open_file(const char* file);

// initiated directory traversal
void log_init_directory(const char* directory);

// finished directory traversal
void log_end_directory(const char* directory);

// failed to init directory traversal
void log_failed_init_directory(const char* directory);

// thread initiating
void log_new_thread();

// thread finished
void log_thread_finished();

#endif // REGISTER_H___
