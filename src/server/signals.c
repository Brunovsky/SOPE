#include "signals.h"
#include "options.h"
#include "log.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>



static volatile sig_atomic_t alarmed = 0;

// SIGHUP, SIGQUIT, SIGTERM, SIGINT
static void sighandler_kill(int signum) {
    static const char* const str = "Terminating...\n";
    write(STDOUT_FILENO, str, strlen(str));
    exit(1);
}

// SIGABRT
static void sighandler_abort(int signum) {
    static const char* const str = "Aborting...\n";
    write(STDOUT_FILENO, str, strlen(str));
    abort();
}

// SIGPIPE
static void sighandler_pipe(int signum) {
    static const char* const str = "Caught SIGPIPE\n";
    write(STDOUT_FILENO, str, strlen(str));
    // Ignore at the moment...
}

// SIGALRM
static void sighandler_alarm(int signum) {
    alarmed = 1;
}

/**
 * Set the process's signal handlers and overall dispositions,
 * to be inherited by all threads.
 */
int set_signal_handlers() {
    struct sigaction action;
    sigset_t sigmask, current;

    // Get current sigmask
    int s = sigprocmask(SIG_SETMASK, NULL, &current);
    if (s != 0) {
        int err = errno;
        printf("Error getting process signal mask: %s\n", strerror(err));
        return err;
    }

    // Set kill handler
    sigmask = current;
    action.sa_handler = sighandler_kill;
    action.sa_mask = sigmask;
    action.sa_flags = SA_RESETHAND;
    s = sigaction(SIGHUP, &action, NULL);
    if (s != 0) {
        int err = errno;
        printf("Error setting handler for SIGHUP: %s\n", strerror(err));
        return err;
    }
    s = sigaction(SIGQUIT, &action, NULL);
    if (s != 0) {
        int err = errno;
        printf("Error setting handler for SIGQUIT: %s\n", strerror(err));
        return err;
    }
    s = sigaction(SIGTERM, &action, NULL);
    if (s != 0) {
        int err = errno;
        printf("Error setting handler for SIGTERM: %s\n", strerror(err));
        return err;
    }
    s = sigaction(SIGINT, &action, NULL);
    if (s != 0) {
        int err = errno;
        printf("Error setting handler for SIGINT: %s\n", strerror(err));
        return err;
    }

    // Set abort handler
    sigmask = current;
    action.sa_handler = sighandler_abort;
    action.sa_mask = sigmask;
    action.sa_flags = SA_RESETHAND;
    s = sigaction(SIGABRT, &action, NULL);
    if (s != 0) {
        int err = errno;
        printf("Error setting handler for SIGABRT: %s\n", strerror(err));
        return err;
    }

    // Set pipe handler
    sigmask = current;
    action.sa_handler = sighandler_pipe;
    action.sa_mask = sigmask;
    action.sa_flags = SA_RESTART;
    s = sigaction(SIGPIPE, &action, NULL);
    if (s != 0) {
        int err = errno;
        printf("Error setting handler for SIGPIPE: %s\n", strerror(err));
        return err;
    }

    // Set alarm handler
    sigmask = current;
    action.sa_handler = sighandler_alarm;
    action.sa_mask = sigmask;
    action.sa_flags = SA_RESETHAND;
    s = sigaction(SIGALRM, &action, NULL);
    if (s != 0) {
        int err = errno;
        printf("Error setting handler for SIGALRM: %s\n", strerror(err));
        return err;
    }

    return 0;
}

void set_alarm() {
    alarm(o_time);
}

int alarm_timeout() {
    return (int)alarmed;
}