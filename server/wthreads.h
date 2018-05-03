#ifndef WTHREADS_H___
#define WTHREADS_H___

// You can change types wthread_arg and wthread_ret to whatever you want to use.
typedef int wthread_ret;
typedef int wthread_arg;
typedef wthread_ret (*wthread_routine)(wthread_arg);

/**
 * Call only after a fork()
 */
void setup_wthreads();

/**
 * Launch a thread that executes function routine with argument arg
 * Returns The id of the wthread
 */
int launch_wthread(wthread_routine routine, wthread_arg arg);

/**
 * Wait for wthreads to finish
 */
void joinall_wthreads();

/**
 * Get the return value of wthread with the given id
 * Presumes the wthread has already finished, else the data might be invalid.
 */
wthread_ret wthread_return(int id);

#endif // WTHREADS_H___