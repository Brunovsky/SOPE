#include "queue.h"
#include "options.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#define QUEUE_SIZE 16

typedef pthread_mutex_t mutex_t;

// The message queue is a dynamically allocated looped array of
// fixed size (the specs say size 1) containing strings consisting
// of messages. The messages are written in indices 0, 1, ...
// and loop back around to the beginning.
// 
// Of course if QUEUE_SIZE is set to 1 none of this is relevant,
// but it works in exactly the same way.
// 
// All messages written are strings dinamically allocated and owned
// by some other module A, then given to this module, then assigned
// to the collecting module B. No char* are allocated or freed here.

static bool queue_initialised = false;
static bool queue_atexit_set = false;
static const char** message_queue = NULL;
static int queue_size = 0, queue_read_p = 0, queue_write_p = 0, queue_unread = 0;

static sem_t not_full_sem, not_empty_sem;
static mutex_t read_access_mutex, write_access_mutex, unread_access_mutex;

static void free_queue() {
    if (queue_initialised) {
        sem_destroy(&not_full_sem);
        sem_destroy(&not_empty_sem);
        pthread_mutex_destroy(&read_access_mutex);
        pthread_mutex_destroy(&write_access_mutex);
        pthread_mutex_destroy(&unread_access_mutex);

        while (queue_unread-- > 0) {
            free((char*)message_queue[queue_read_p]);
            queue_read_p = (queue_read_p + 1) % queue_size;
        }
        
        free(message_queue);
        message_queue = NULL;
        queue_size = queue_read_p = queue_write_p = queue_unread = 0;
        queue_initialised = false;
    }
}

static inline void lock_read_access() {
    pthread_mutex_lock(&read_access_mutex);
}

static inline void lock_write_access() {
    pthread_mutex_lock(&write_access_mutex);
}

static inline void unlock_read_access() {
    pthread_mutex_unlock(&read_access_mutex);
}

static inline void unlock_write_access() {
    pthread_mutex_unlock(&write_access_mutex);
}

static inline void wait_for_message() {
    sem_wait(&not_empty_sem);
}

static inline void wait_for_space() {
    sem_wait(&not_full_sem);
}

static inline void post_new_message() {
    sem_post(&not_empty_sem);
}

static inline void post_new_space() {
    sem_post(&not_full_sem);
}

static inline void lock_unread() {
    pthread_mutex_lock(&unread_access_mutex);
}

static inline void unlock_unread() {
    pthread_mutex_unlock(&unread_access_mutex);
}

static void _read_message(const char** message_ptr) {
    lock_read_access();
    wait_for_message();

    *message_ptr = message_queue[queue_read_p];
    queue_read_p = (queue_read_p + 1) % queue_size;

    post_new_space();
    unlock_read_access();

    lock_unread();
    --queue_unread;
    unlock_unread();
}

static void _write_message(const char* message) {
    lock_write_access();
    wait_for_space();

    message_queue[queue_write_p] = message;
    queue_write_p = (queue_write_p + 1) % queue_size;

    post_new_message();
    unlock_write_access();

    lock_unread();
    ++queue_unread;
    unlock_unread();
}

static void init_queue() {
    message_queue = malloc(QUEUE_SIZE * sizeof(const char*));
    queue_size = QUEUE_SIZE;

    sem_init(&not_full_sem, 0, queue_size);
    sem_init(&not_empty_sem, 0, 0);
    pthread_mutex_init(&read_access_mutex, NULL);
    pthread_mutex_init(&write_access_mutex, NULL);
    pthread_mutex_init(&unread_access_mutex, NULL);

    queue_initialised = true;
}

void setup_queue() {
    if (queue_initialised) free_queue();

    init_queue();
    
    if (!queue_atexit_set) {
        atexit(free_queue);
        queue_atexit_set = true;
    }
}

int read_message(const char** message_p) {
    if (!queue_initialised) return 1;

    _read_message(message_p);

    return 0;
}

int write_message(const char* message) {
    if (!queue_initialised) return 1;

    _write_message(message);

    return 0;
}