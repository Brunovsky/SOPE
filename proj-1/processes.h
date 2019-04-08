#ifndef PROCESSES_H___
#define PROCESSES_H___

typedef void* process_arg;
typedef int (*process_routine)(process_arg);

int spawn_process(process_routine, process_arg);

int set_main_signal_handlers();

void waitall_children();

#endif // PROCESSES_H___
