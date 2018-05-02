#include "workers.h"
#include "options.h"
#include "wthreads.h"
#include "seats.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#define QUEUE_SIZE 1

typedef pthread_mutex_t mutex_t;

// The message queue is a dynamically allocated array that loops
// around itself to the beginning once messages written have flown
// past its allocated size.
// All messages written are strings dinamically allocated by some
// other module, but freed in this module.

// The semaphores control how many unread and unprocessed messages
// there are in message_queue.
// There is nothing to read -- controlled by not_empty_sem
// There is no space to write more -- controlled by not_full_sem
static bool queue_initialised = false;
static bool queue_atexit_set = false;
static const char** message_queue = NULL;
static int queue_size = 0, queue_read_p = 0, queue_write_p = 0, queue_unread = 0;

static sem_t not_full_sem, not_empty_sem;
static mutex_t worker_access_queue;

static void free_queue() {
    if (queue_initialised) {
        free_semaphores();
        free_mutexes();
        while (queue_unread-- > 0) {
            free(message_queue[queue_read_p]);
            queue_read_p = (queue_read_p + 1) % queue_size;
        }
        free(message_queue);
        message_queue = NULL;
        queue_size = queue_read_p = queue_write_p = queue_unread = 0;
        queue_initialised = false;
    }
}

static void free_semaphores() {
    sem_destroy(&not_full_sem);
    sem_destroy(&not_empty_sem);
}

static void init_semaphores() {
    sem_init(&not_full_sem, 0, ?);
    sem_init(&not_empty_sem, 0, ?);
}

static void free_mutexes() {
    pthread_mutex_destroy(&worker_access_queue);
}

static void init_mutexes() {
    pthread_mutex_init(&worker_access_queue, NULL);
}

static int worker_ready() {
    pthread_mutex_lock(&worker_access_queue);
}

static int worker_done() {
    pthread_mutex_unlock(&worker_access_queue);
    sem_post(&not_full_sem);
}

static int wait_for_message() {
    sem_wait(&not_empty_sem);
}

static int wait_for_space() {
    sem_wait(&not_full_sem);
}

static void read_message(const char** message_ptr) {
    worker_ready();
    wait_for_message();
    *message_ptr = message_queue[queue_read_p];
    queue_read_p = (queue_read_p + 1) % queue_size;
}

static void write_message(const char* message) {
    worker_ready();
    wait_for_space();

    message_queue[queue_write_p] = message;
    queue_write_p = (queue_write_p + 1) % queue_size;

    worker_done();
}

static int worker(int id) {
    while (true) {
        const char* message;
        read_message(&message);

        // SR
        if (is_exit_command(message)) break;
        
        process_message(message);
    }

    return 0;
}

static void init_queue() {
    init_semaphores();
    init_mutexes();
}

void setup_queue() {
    if (queue_initialised) free_queue();

    message_queue = malloc(QUEUE_SIZE * sizeof(const char*));
    queue_size = QUEUE_SIZE;
    init_queue();
    
    if (!queue_atexit_set) {
        atexit(free_queue);
        queue_atexit_set = true;
    }
}

int add_new_message(const char* message) {
    if (!queue_initialised) setup_queue();

    write_message(message);

    return 0;
}