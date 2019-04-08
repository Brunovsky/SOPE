#include "processes.h"
#include "wthreads.h"
#include "register.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>

#define CHILD 0
#define NO_FORK -1
#define PARENT
#define MAX_FORK_ITERATIONS 10

static int child_handlers_set = false;
static int process_children_count = 0;
static bool terminated = false, this_stop = false;

static const char stop_message[] = "Simgrep stopped.";
static const char confirm_term[] = "Are you sure you "
"want to terminate the program? (Y/N) ";



// SIGHUP, SIGQUIT, SIGTERM
static void sighandler_kill(int signum) {
	log_signal_received(signum);
	exit(signum);
}

// SIGABRT
static void sighandler_abort(int signum) {
	log_signal_received(signum);
	abort();
}

// SIGINT
static void sighandler_stop_main(int signum) {
	log_signal_received(signum);
	this_stop = true;

	printf("\n%s", stop_message);
	do {
		printf("\n%s", confirm_term);

		char input[512] = {0};
		if (!scanf("%s", input)) continue;

		char c = input[0];

		if (c == 'y' || c == 'Y') {
			log_signal_sent_group(SIGTERM, getpgrp());
			kill(0, SIGTERM);
			break;
		} else if (c == 'n' || c == 'N') {
			log_signal_sent_group(SIGCONT, getpgrp());
			kill(0, SIGCONT);
			break;
		}
	} while (true);
}

// SIGINT
static void sighandler_stop_child(int signum) {
	log_signal_received(signum);
	this_stop = true;
	while (this_stop) pause();
}

// SIGCONT
static void sighandler_cont(int signum) {
	log_signal_received(signum);
	this_stop = false;
	// It continues certainly, as commanded by the OS ...
}

// SIGCHLD
static void sighandler_child(int signum) {
	pid_t pid;
	int status;
	log_signal_received(signum);

	while ((process_children_count > 0)
			&& (pid = waitpid(-1, &status, WNOHANG)) > 0) {
		if (WIFSTOPPED(status) || WIFCONTINUED(status)) {
			log_general("SIGCHLD STOP OR CONT");
		} else { // WIFEXITED, WIFSIGNALED
			--process_children_count;
			log_waitpid_resolved(pid, status, process_children_count);
		}
	}
}

/**
 * Set the main process's signal handlers and overall dispositions,
 * to be inherited by all child processes and threads.
 */
int set_main_signal_handlers() {
	struct sigaction action;
	sigset_t sigmask, current;

	// Get current sigmask
	int s = sigprocmask(SIG_SETMASK, NULL, &current);
	if (s != 0) {
		int err = errno;
		printf("simgrep: signals: Error getting process signal mask: %s\n",
			strerror(err));
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
		printf("simgrep: signals: Error setting handler for SIGHUP: %s\n",
			strerror(err));
		return err;
	}
	s = sigaction(SIGQUIT, &action, NULL);
	if (s != 0) {
		int err = errno;
		printf("simgrep: signals: Error setting handler for SIGQUIT: %s\n",
			strerror(err));
		return err;
	}
	s = sigaction(SIGTERM, &action, NULL);
	if (s != 0) {
		int err = errno;
		printf("simgrep: signals: Error setting handler for SIGTERM: %s\n",
			strerror(err));
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
		printf("simgrep: signals: Error setting handler for SIGABRT: %s\n",
			strerror(err));
		return err;
	}

	// Set stop (main) handler
	sigmask = current;
	action.sa_handler = sighandler_stop_main;
	action.sa_mask = sigmask;
	action.sa_flags = SA_RESTART;
	s = sigaction(SIGINT, &action, NULL);
	if (s != 0) {
		int err = errno;
		printf("simgrep: signals: Error setting handler for SIGINT: %s\n",
			strerror(err));
		return err;
	}

	// Set cont handler
	sigmask = current;
	action.sa_handler = sighandler_cont;
	action.sa_mask = sigmask;
	action.sa_flags = SA_RESTART;
	s = sigaction(SIGCONT, &action, NULL);
	if (s != 0) {
		int err = errno;
		printf("simgrep: signals: Error setting handler for SIGCONT: %s\n",
			strerror(err));
		return err;
	}

	// Set child handler
	sigmask = current;
	action.sa_handler = sighandler_child;
	action.sa_mask = sigmask;
	action.sa_flags = SA_RESTART;
	s = sigaction(SIGCHLD, &action, NULL);
	if (s != 0) {
		int err = errno;
		printf("simgrep: signals: Error setting handler for SIGCHLD: %s\n",
			strerror(err));
		return err;
	}

	return 0;
}

/**
 * Set a new child's signal handlers and overall disposition, which may
 * or may not differ from those of its parent.
 */
static int set_child_signal_handlers() {
	struct sigaction action;
	sigset_t sigmask, current;

	// Get current sigmask
	int s = sigprocmask(SIG_SETMASK, NULL, &current);
	if (s != 0) {
		int err = errno;
		printf("simgrep: signals: Error getting process signal mask: %s\n",
			strerror(err));
		return err;
	}

	// Set stop (child) handler
	sigmask = current;
	action.sa_handler = sighandler_stop_child;
	action.sa_mask = sigmask;
	action.sa_flags = 0;
	s = sigaction(SIGINT, &action, NULL);
	if (s != 0) {
		int err = errno;
		printf("simgrep: signals: Error setting handler for SIGINT: %s\n",
			strerror(err));
		return err;
	}

	return 0;
}





/**
 * Setup this newly created child process in preparation to be managed
 * by this very utility, including children counting and new signal handlers.
 */
static void setup_new_child() {
	process_children_count = 0;
	terminated = false;
	setup_new_mainthread();
	if (!child_handlers_set) set_child_signal_handlers();
}

/**
 * Indicate a new child was created.
 */
static void add_new_child(int child_pid) {
	++process_children_count;
	log_new_process(child_pid);
}

/**
 * Wait for all children still alive to terminate before returning.
 */
void waitall_children() {
	terminated = true;
	pid_t pid;
	int status;

	waitall_threads();

	while (process_children_count > 0) {
		while ((pid = waitpid(-1, &status, 0)) > 0) {
			if (WIFSTOPPED(status) || WIFCONTINUED(status)) {
				log_general("RECEIVED CHILD NOTIFICATION: CHILD STOP OR CONT");
				continue;
			} else { // WIFEXITED, WIFSIGNALED
				--process_children_count;
				log_waitpid_resolved(pid, status, process_children_count);
			}
		}
	}
}






/**
 * Auxiliary sleep function for spawn_process_int
 */
static void quick_sleep(unsigned long long microseconds) {
	struct timespec required, remaining;
	required.tv_sec = microseconds / 1000000ULL;
	required.tv_nsec = (1000ULL * microseconds) % 1000000000ULL;
	
	int e = nanosleep(&required, &remaining);
	while (e != 0) { // EINTR
		required = remaining;
		e = nanosleep(&required, &remaining);
	}
}

static int spawn_process_int(process_routine routine, process_arg arg,
		int iteration) {
	pid_t pid = fork();

	if (pid == CHILD) {
		setup_new_child();
		routine(arg);
		waitall_children();
		exit(0);
	} else if (pid == NO_FORK) {
		if (errno == EAGAIN && iteration++ < MAX_FORK_ITERATIONS) {
			// Wait 5ms, then 10ms, 15ms, ..., 45ms.
			quick_sleep(5000ULL * (unsigned long long)iteration);
			return spawn_process_int(routine, arg, iteration);
		} else {
			log_failed_fork(errno);
			return 1;
		}
	} else PARENT {
		add_new_child(pid);
		return 0;
	}
}

int spawn_process(process_routine routine, process_arg arg) {
	return spawn_process_int(routine, arg, 0);
}
