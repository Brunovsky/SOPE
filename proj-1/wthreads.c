#include "wthreads.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#define WTHREADS_INIT_SIZE 2

// Take into account that this process might be the
// child of another multithreaded process.
// In that case every one of these fields needs to be
// reset first, with free_wthread.
static bool wthread_initialised = false;
static bool atexit_set = false;
static pthread_t* wthreads = NULL;
static int* wthread_rets = NULL;
static int wthread_count = 0, wthreads_size = 0, wthread_joined = 0;

static void free_wthread() {
	if (wthread_initialised) {
		free(wthreads);
		wthreads = NULL;
		free(wthread_rets);
		wthread_rets = NULL;
		wthread_count = wthreads_size = wthread_joined = 0;
		wthread_initialised = false;
	}
}

static void prepare_for_new_thread() {
	if (wthread_count < wthreads_size) return;

	wthreads_size *= 2;
	wthreads = realloc(wthreads, wthreads_size * sizeof(pthread_t));
	wthread_rets = realloc(wthread_rets, wthreads_size * sizeof(int));
}

void setup_new_mainthread() {
	if (wthread_initialised) free_wthread();
	wthreads = malloc(WTHREADS_INIT_SIZE * sizeof(pthread_t));
	wthread_rets = malloc(WTHREADS_INIT_SIZE * sizeof(int));
	wthreads_size = WTHREADS_INIT_SIZE;
	
	wthread_initialised = true;

	if (!atexit_set) {
		atexit(free_wthread);
		atexit_set = true;
	}
}

void launch_thread(wthread_routine routine, wthread_arg arg) {
	// If the wthread utility is not initialised yet,
	// then we're in the main thread and we got to set things up first.
	// We therefore remove from the user the burden of calling this function
	// and pass the burden to the process handling utility.
	if (!wthread_initialised) setup_new_mainthread();

	prepare_for_new_thread();
	pthread_create(&wthreads[wthread_count++], NULL, routine, arg);
}

void set_thread_return(int ret_val) {
	pthread_t pt = pthread_self();
	int count = wthread_count;
	for (int i = 0; i < count; ++i) {
		if (pthread_equal(pt, wthreads[i])) {
			wthread_rets[i] = ret_val;
			break;
		}
	}
	return;
}


void waitall_threads() {
	// Its not a multithreaded process...
	if (!wthread_initialised) return;

	// Join all threads not yet joined.
	while (wthread_joined < wthread_count) {
		pthread_join(wthreads[wthread_joined++], NULL);
	}
}
