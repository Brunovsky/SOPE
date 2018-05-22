#ifndef WTHREADS_H___
#define WTHREADS_H___

// You can change types wthread_arg and wthread_ret to whatever you want to use.
typedef int wthread_ret;
typedef int wthread_arg;
typedef wthread_ret (*wthread_routine)(wthread_arg);

void setup_wthreads();

int launch_wthread(wthread_routine routine, wthread_arg arg);

void joinall_wthreads();

wthread_ret wthread_return(int id);

#endif // WTHREADS_H___
