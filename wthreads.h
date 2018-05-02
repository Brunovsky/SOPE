#ifndef WTHREADS_H___
#define WTHREADS_H___

// You can change types wthread_arg and wthread_ret to whatever you want to use.
typedef char wthread_ret;
typedef char* wthread_arg;
typedef wthread_ret (*wthread_routine)(wthread_arg);

/**
 * Set this thread as a main thread.
 */
void setup_new_main();

int launch_wthread(wthread_routine routine, wthread_arg arg);

void waitall_wthreads();

void cancelall_wthreads();

void killall_wthreads();

wthread_ret wthread_return(int index);

#endif // WTHREADS_H___