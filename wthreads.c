#include "wthreads.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct {
	wthread_routine routine;
	wthread_arg arg;
	int index;
} wthread_wrapper_t;

#define WTHREADS_INIT_SIZE 4

static bool wthreads_initialised = false;
static bool wthreads_atexit_set = false;
static pthread_t* wthreads_ids = NULL;
static wthread_ret* wthreads_rets = NULL;
static int wthreads_count = 0, wthreads_size = 0, wthreads_joined = 0;

static void free_wthreads() {
	if (wthreads_initialised) {
		free(wthreads_ids);
		wthreads_ids = NULL;
		free(wthreads_rets);
		wthreads_rets = NULL;
		wthreads_count = wthreads_size = wthreads_joined = 0;
		wthreads_initialised = false;
	}
}

static void prepare_for_new_wthread() {
	if (wthreads_count < wthreads_size) return;

	wthreads_size *= 2;
	wthreads_ids = realloc(wthreads_ids, wthreads_size * sizeof(pthread_t));
	wthreads_rets = realloc(wthreads_rets, wthreads_size * sizeof(int));
	// Not much can be done at the moment if the reallocs fail...
}

static void* wthread(void* void_wrapper) {
	wthread_wrapper_t* wrapper = (wthread_wrapper_t*)void_wrapper;
	wthread_routine routine = wrapper->routine;
	wthread_arg arg = wrapper->arg;
	int index = wrapper->index;
	free((void*)wrapper);

	/* TODO Pre-routine*/

	wthread_ret ret = routine(arg);
	wthreads_rets[index] = ret;

	/* TODO Post-routine */

	return NULL;
}

static void* bind(wthread_routine routine, wthread_arg arg, int index) {
	wthread_wrapper_t* wrapper = malloc(sizeof(wthread_wrapper_t));
	wrapper->routine = routine;
	wrapper->arg = arg;
	wrapper->index = index;
	return (void*)wrapper;
}

void setup_new_main() {
	if (wthreads_initialised) free_wthreads();
	wthreads_ids = malloc(WTHREADS_INIT_SIZE * sizeof(pthread_t));
	wthreads_rets = malloc(WTHREADS_INIT_SIZE * sizeof(int));
	wthreads_size = WTHREADS_INIT_SIZE;
	
	wthreads_initialised = true;

	if (!wthreads_atexit_set) {
		atexit(free_wthreads);
		wthreads_atexit_set = true;
	}
}

int launch_wthread(wthread_routine routine, wthread_arg arg) {
	if (!wthreads_initialised) setup_new_main();

	prepare_for_new_wthread();
	pthread_create(&wthreads_ids[wthreads_count], NULL, wthread, bind(routine, arg, wthreads_count));
	return wthreads_count++;
}

void waitall_wthreads() {
	// Its not a multithreaded process?...
	if (!wthreads_initialised) return;

	// Join all threads not yet joined.
	while (wthreads_joined < wthreads_count) {
		pthread_join(wthreads_ids[wthreads_joined++], NULL);
	}
}

wthread_ret wthread_return(int index) {
	return wthreads_rets[index];
}