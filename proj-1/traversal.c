#include "traversal.h"
#include "simgrep.h"
#include "options.h"
#include "processes.h"
#include "register.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <dirent.h>

#define CHILD 0
#define NO_FORK -1
#define PARENT
#define MAX_ITERATIONS 5
#define DOT "."
#define DOTDOT ".."



/**
 * Minimally generic implementation of a path concatenator for UNIX paths.
 * If file starts with '/', assume it is a full path, and therefore
 * return a duplicate of it.
 * Otherwise return directory / file, taking the care
 * not to repeat to interior '/'.
 */
static char* join_path(const char* directory, const char* file) {
	// assume file is an absolute path
	if (file[0] == '/') return strdup(file);

	size_t d_len = strlen(directory), f_len = strlen(file);
	
	if (directory[d_len - 1] == '/') --d_len;

	char* concat = malloc((d_len + f_len + 2) * sizeof(char));

	strncpy(concat, directory, d_len);
	concat[d_len] = '/';
	strcpy(concat + d_len + 1, file);

	return concat;
}

static int ftsent_cmp(const FTSENT** one, const FTSENT** two){
	return strcmp((*one)->fts_name, (*two)->fts_name);
}

static int directory_traversal_fts_routine(void* void_directory) {
	char* directory = (char*)void_directory;
	return directory_traversal_fts(directory);
}

static int directory_traversal_dir_routine(void* void_directory) {
	char* directory = (char*)void_directory;
	return directory_traversal_fts(directory);
}

static int fork_directory_traversal_fts(char* directory) {
	return spawn_process(directory_traversal_fts_routine, (void*)directory);
}

static int fork_directory_traversal_dir(char* directory) {
	return spawn_process(directory_traversal_dir_routine, (void*)directory);
}

int directory_traversal_fts(char* directory) {
	char* const directories[] = {directory, NULL};
	FTS* fts = fts_open(directories, FTS_NOCHDIR | FTS_PHYSICAL, &ftsent_cmp);
	if (fts == NULL) {
		log_failed_init_directory(directory);
		return 1;
	}
	log_init_directory(directory);

	FTSENT* ftsentry;

	while ((ftsentry = fts_read(fts))) {
		unsigned short info = ftsentry->fts_info;
		char* filename = ftsentry->fts_accpath;

		if (ftsentry->fts_level != 1) continue;

		switch (info) {
		case FTS_D: // Directory visited in preorder
			fork_directory_traversal_fts(filename);
			break;
		case FTS_F: // File visited
			grep_file(filename);
			break;
		default: // Skip everything else
			break;
		}
	}

	log_end_directory(directory);
	fts_close(fts);
	return 0;
}

int directory_traversal_dir(char* directory) {
	DIR* dir = opendir(directory);
	if (dir == NULL) {
		log_failed_init_directory(directory);
		return 1;
	}
	log_init_directory(directory);

	struct dirent* direntry;

	while ((direntry = readdir(dir)) != NULL) {
		char* filename = direntry->d_name;

		if (strcmp(filename, DOT) == 0 || strcmp(filename, DOTDOT) == 0) {
			continue;
		}

		char* pass = join_path(directory, filename);

		struct stat statbuf;
		if (stat(pass, &statbuf) != 0) {
			printf("simgrep: %s: %s\n", pass, strerror(errno));
		} else {
			if (S_ISDIR(statbuf.st_mode)) {
				fork_directory_traversal_dir(pass);
			} else if (S_ISREG(statbuf.st_mode)) {
				grep_file(pass);
			}
		}

		free((void*)pass);
	}

	log_end_directory(directory);
	closedir(dir);
	return 0;
}

int directory_traversal_fts_singleprocess(char* directory) {
	char* const directories[] = {directory, NULL};
	FTS* fts = fts_open(directories, FTS_NOCHDIR | FTS_PHYSICAL, &ftsent_cmp);
	if (fts == NULL) {
		log_failed_init_directory(directory);
		return 1;
	}
	log_init_directory(directory);

	FTSENT* ftsentry;

	while ((ftsentry = fts_read(fts))) {
		unsigned short info = ftsentry->fts_info;

		if (ftsentry->fts_level == 0) continue;

		char* filename = ftsentry->fts_accpath;

		switch (info) {
		case FTS_F: // File visited
			grep_file(filename);
			break;
		default: // Skip everything else
			break;
		}
	}

	log_end_directory(directory);
	fts_close(fts);
	return 0;
}

int directory_traversal_dir_singleprocess(char* directory) {
	DIR* dir = opendir(directory);
	if (dir == NULL) {
		log_failed_init_directory(directory);
		return 1;
	}
	log_init_directory(directory);

	struct dirent* direntry;

	while ((direntry = readdir(dir)) != NULL) {
		char* filename = direntry->d_name;

		if (strcmp(filename, DOT) == 0 || strcmp(filename, DOTDOT) == 0) {
			continue;
		}

		char* pass = join_path(directory, filename);

		struct stat statbuf;
		if (stat(pass, &statbuf) != 0) {
			printf("simgrep: %s: %s\n", pass, strerror(errno));
		} else {
			if (S_ISDIR(statbuf.st_mode)) {
				directory_traversal_dir(pass);
			} else if (S_ISREG(statbuf.st_mode)) {
				grep_file(pass);
			}
		}

		free((void*)pass);
	}

	log_end_directory(directory);
	closedir(dir);
	return 0;
}

int init_traversal_fts() {
	FTS* fts = fts_open(files, FTS_NOCHDIR | FTS_PHYSICAL, &ftsent_cmp);
	if (fts == NULL) {
		log_general("FAILED TO INIT FTS TRAVERSAL OF ALL FILES");
		return 1;
	}
	log_general("BEGIN TRAVERSAL OF ALL FILES");

	FTSENT* ftsentry;

	while ((ftsentry = fts_read(fts))) {
		unsigned short info = ftsentry->fts_info;
		char* filename = ftsentry->fts_accpath;

		if (ftsentry->fts_level != 0) continue;

		switch (info) {
		case FTS_D: // Directory visited in preorder
			if (recurse)
				fork_directory_traversal_fts(filename);
			else
				printf("simgrep: %s: %s\n", filename, strerror(EISDIR));
			break;
		case FTS_F: // File visited
			grep_file(filename);
			break;
		default: // Skip everything else
			break;
		}
	}

	log_general("END TRAVERSAL OF ALL FILES");
	fts_close(fts);
	return 0;
}

int init_traversal_dir() {
	for (size_t i = 0; i < number_of_files; ++i) {
		struct stat statbuf;
		if (stat(files[i], &statbuf) != 0) {
			printf("simgrep: %s: %s\n", files[i], strerror(errno));
		} else {
			if (S_ISDIR(statbuf.st_mode)) {
				if (recurse) // [[OPTIONS]] recurse
					fork_directory_traversal_dir(files[i]);
				else
					printf("simgrep: %s: %s\n", files[i], strerror(EISDIR));
			} else if (S_ISREG(statbuf.st_mode)) {
				grep_file(files[i]);
			}
		}
	}
	return 0;
}

int init_traversal_fts_singleprocess() {
	FTS* fts = fts_open(files, FTS_NOCHDIR | FTS_PHYSICAL, &ftsent_cmp);
	if (fts == NULL) {
		log_general("FAILED TO INIT FTS TRAVERSAL OF ALL FILES");
		return 1;
	}
	log_general("BEGIN TRAVERSAL OF ALL FILES");

	FTSENT* ftsentry;

	while ((ftsentry = fts_read(fts))) {
		unsigned short info = ftsentry->fts_info;
		char* filename = ftsentry->fts_accpath;

		switch (info) {
		case FTS_F: // File visited
			grep_file(filename);
			break;
		default: // Skip everything else
			break;
		}
	}

	log_general("END TRAVERSAL OF ALL FILES");
	fts_close(fts);
	return 0;
}

int init_traversal_dir_singleprocess() {
	for (size_t i = 0; i < number_of_files; ++i) {
		struct stat statbuf;
		if (stat(files[i], &statbuf) != 0) {
			printf("simgrep: (stat) %s: %s\n", files[i], strerror(errno));
		} else {
			if (S_ISDIR(statbuf.st_mode)) {
				if (recurse) // [[OPTIONS]] recurse
					directory_traversal_dir(files[i]);
				else
					printf("simgrep: %s: %s\n", files[i], strerror(EISDIR));
			} else if (S_ISREG(statbuf.st_mode)) {
				grep_file(files[i]);
			}
		}
	}
	return 0;
}

int init_traversal_here_fts() {
	return directory_traversal_fts(".");
}

int init_traversal_here_dir() {
	return directory_traversal_dir(".");
}

int init_traversal_here_fts_singleprocess() {
	return directory_traversal_fts_singleprocess(".");
}

int init_traversal_here_dir_singleprocess() {
	return directory_traversal_dir_singleprocess(".");
}
