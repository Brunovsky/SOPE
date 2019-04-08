#include "options.h"
#include "register.h"
#include "make_grep.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/syscall.h>

#ifdef SYS_gettid
#define gettid() syscall(SYS_gettid)
#else
#define gettid() -1
#endif

static struct timespec epoch;

static FILE* logfile = NULL;
static const char* logfilename = NULL;
static int logfileno;

static const char* sys_signame[] = {
	0,
	"SIGHUP",
	"SIGINT",
	"SIGQUIT",
	"SIGILL",
	"SIGTRAP",
	"SIGABRT",
	"SIGBUS",
	"SIGFPE",
	"SIGKILL",
	"SIGUSR1",
	"SIGSEGV",
	"SIGUSR2",
	"SIGPIPE",
	"SIGALRM",
	"SIGTERM",
	"SIGSTKFLT",
	"SIGCHLD",
	"SIGCONT",
	"SIGSTOP",
	"SIGTSTP",
	"SIGTTIN",
	"SIGTTOU",
	"SIGURG",
	"SIGXCPU",
	"SIGXFSZ",
	"SIGVTALRM",
	"SIGPROF",
	"SIGWINCH",
	"SIGIO",
	"SIGPWR",
	"SIGSYS",
	"SIGRTMIN",
	"SIGRTMIN+1",
	"SIGRTMIN+2",
	"SIGRTMIN+3",
	"SIGRTMIN+4",
	"SIGRTMIN+5",
	"SIGRTMIN+6",
	"SIGRTMIN+7",
	"SIGRTMIN+8",
	"SIGRTMIN+9",
	"SIGRTMIN+10",
	"SIGRTMIN+11",
	"SIGRTMIN+12",
	"SIGRTMIN+13",
	"SIGRTMIN+14",
	"SIGRTMIN+15",
	"SIGRTMAX-14",
	"SIGRTMAX-13",
	"SIGRTMAX-12",
	"SIGRTMAX-11",
	"SIGRTMAX-10",
	"SIGRTMAX-9",
	"SIGRTMAX-8",
	"SIGRTMAX-7",
	"SIGRTMAX-6",
	"SIGRTMAX-5",
	"SIGRTMAX-4",
	"SIGRTMAX-3",
	"SIGRTMAX-2",
	"SIGRTMAX-1",
	"SIGRTMAX"
};



void timestamp_epoch() {
	clock_gettime(CLOCK_REALTIME, &epoch);
}



/**
 * Return number of milliseconds since main epoch
 */
static double inst() {
	// printf("%8.2lf")
	struct timespec current;
	clock_gettime(CLOCK_REALTIME, &current);
	double ms = 1000.0 * (double)(current.tv_sec - epoch.tv_sec);
	ms += (double)(current.tv_nsec - epoch.tv_nsec) / 1000000.0;
	return ms;
}

/**
 * Return this process's pid
 */
static int pid() {
	// printf("%8d")
	return (int)getpid();
}

/**
 * Return this thread's tid
 */
static int tid() {
	return (int)gettid();
}

/**
 * Close the logfile for this process
 */
static void close_logfile() {
	log_close_file(logfilename);
	fclose(logfile);
}

/**
 * Open the logfile from path in the environment variable ENV_LOGFILENAME.
 * Register a function to close it at exit.
 */
int open_logfile() {
	const char* name = getenv(ENV_LOGFILENAME);
	
	if (name != NULL) {
		logfile = fopen(name, "w");
		if (logfile == NULL) {
			int err = errno;
			printf("simgrep: logfile: Failed to open registry file: %s\n",
				strerror(err));
			return err;
		}
		logfilename = name;
		logfileno = fileno(logfile);
		log_open_file(logfilename);
		atexit(close_logfile);
		atexit(log_process_exit);
		return 0;
	} else {
		printf("simgrep: logfile: Environment variable %s not present.\n",
			ENV_LOGFILENAME);
		return 1;
	}
}



void log_general(const char* gen) {
	char str[2048];

	sprintf(str, "%8.2lf - %8d - %s\n",
		inst(), pid(), gen);

	write(logfileno, str, strlen(str));
}

void log_command(char* const* argv) {
	const char* cmd = flatten_command(argv);
	char str[1024];

	sprintf(str, "%8.2lf - %8d - SHELL:  %s\n",
		inst(), pid(), cmd);

	write(logfileno, str, strlen(str));
	free((void*)cmd);
}

void log_new_process(int child_pid) {
	char str[256];

	sprintf(str, "%8.2lf - %8d - NEW PROCESS %d\n",
		inst(), pid(), child_pid);

	write(logfileno, str, strlen(str));
}

void log_process_exit() {
	char str[256];

	sprintf(str, "%8.2lf - %8d - EXIT PROCESS %d\n",
		inst(), pid(), pid());

	write(logfileno, str, strlen(str));
}

void log_waitpid_resolved(int child_pid, int status, int children_left) {
	char str[256];

	sprintf(str, "%8.2lf - %8d - WAITPID (PROCESS %d) "
		"(STATUS 0x%04x) (LEFT: %d)\n",
		inst(), pid(), child_pid, status, children_left);

	write(logfileno, str, strlen(str));
}

void log_failed_fork(int err) {
	char str[1024];

	sprintf(str, "%8.2lf - %8d - [ERROR] FAILED FORK (ERROR %d)\n",
		inst(), pid(), err);

	write(logfileno, str, strlen(str));
}

void log_signal_received(int signal) {
	char str[256];

	sprintf(str, "%8.2lf - %8d - SIGNAL IN %s\n",
		inst(), pid(), sys_signame[signal]);

	write(logfileno, str, strlen(str));
}

void log_signal_sent(int signal, int target_process_pid) {
	char str[256];

	sprintf(str, "%8.2lf - %8d - SIGNAL OUT %s (TO %d)\n",
		inst(), pid(), sys_signame[signal], target_process_pid);
	
	write(logfileno, str, strlen(str));
}

void log_signal_sent_group(int signal, int target_process_pgid) {
	char str[256];

	sprintf(str, "%8.2lf - %8d - SIGNAL SENT %s (GROUP %d)\n",
		inst(), pid(), sys_signame[signal], target_process_pgid);
	
	write(logfileno, str, strlen(str));
}

void log_open_file(const char* file) {
	char str[1024];

	if (multithreading)
		sprintf(str, "%8.2lf - %8d - T %6d - OPEN %s\n",
			inst(), pid(), tid(), file);
	else
		sprintf(str, "%8.2lf - %8d - OPEN %s\n",
			inst(), pid(), file);

	write(logfileno, str, strlen(str));
}

void log_close_file(const char* file) {
	char str[1024];

	if (multithreading)
		sprintf(str, "%8.2lf - %8d - T %6d - CLOSE %s\n",
			inst(), pid(), tid(), file);
	else
		sprintf(str, "%8.2lf - %8d - CLOSE %s\n",
			inst(), pid(), file);

	write(logfileno, str, strlen(str));
}

void log_failed_open_file(const char* file) {
	char str[1024];

	if (multithreading)
		sprintf(str, "%8.2lf - %8d - THREAD %6d - [ERROR] FAILED TO OPEN "
			"FILE %s\n", inst(), pid(), tid(), file);
	else
		sprintf(str, "%8.2lf - %8d - [ERROR] FAILED TO OPEN FILE %s\n",
			inst(), pid(), file);

	write(logfileno, str, strlen(str));
}

void log_init_directory(const char* directory) {
	char str[1024];

	sprintf(str, "%8.2lf - %8d - BEGIN TRAVERSAL %s\n",
		inst(), pid(), directory);

	write(logfileno, str, strlen(str));
}

void log_end_directory(const char* directory) {
	char str[1024];

	sprintf(str, "%8.2lf - %8d - END TRAVERSAL %s\n",
		inst(), pid(), directory);

	write(logfileno, str, strlen(str));
}

void log_failed_init_directory(const char* directory) {
	char str[1024];

	sprintf(str, "%8.2lf - %8d - [ERROR] FAILED TO INIT TRAVERSAL %s\n",
		inst(), pid(), directory);

	write(logfileno, str, strlen(str));
}

void log_new_thread() {
	char str[256];

	sprintf(str, "%8.2lf - %8d - T %6d - NEW\n",
		inst(), pid(), tid());

	write(logfileno, str, strlen(str));
}

void log_thread_finished() {
	char str[256];

	sprintf(str, "%8.2lf - %8d - T %6d - FINISHED\n",
		inst(), pid(), tid());

	write(logfileno, str, strlen(str));
}
