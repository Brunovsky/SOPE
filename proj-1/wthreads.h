#ifndef WTHREADS_H___
#define WTHREADS_H___

// wthreads = wrapped threads

typedef void* wthread_arg;
typedef void* (*wthread_routine)(wthread_arg);

void setup_new_mainthread();

void launch_thread(wthread_routine, wthread_arg);

void set_thread_return(int ret_val);

void waitall_threads();

#endif // WTHREADS_H___
