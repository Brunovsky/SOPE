#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <stddef.h>

// NOTE: All externs are resolved in options.c
// Too add an option, do:
//      1. Add a block entry here
//      2. Resolve the extern in options.c
//      3. Update short_options and long_options in options.c
//      3. Update the usage string in options.c
//      4. Update parse_args in options.c
//      5. Update make_grep in make_grep.c
// then go on to add the option's functionality...



// <!--- GENERAL OPTIONS
// Show help/usage message and exit
#define HELP_FLAG // NONE
#define HELP_LFLAG "help"
extern int show_help;

// Show help/usage message and exit
#define USAGE_FLAG // NONE
#define USAGE_LFLAG "usage"
extern int show_usage;

// Show version message and exit
#define VERSION_FLAG 'V'
#define VERSION_LFLAG "version"
extern int show_version;
// ----> END OF GENERAL OPTIONS



// <!--- OPTIONS OF SIMGREP
// Perform case insensitive regex search (set icase flag)
#define ICASE_FLAG 'i'
#define ICASE_LFLAG "ignore-case"
#define ICASE_GREP "-i"
extern int icase; // default false

// Print only the names of FILEs containing matches
#define LISTFILES_FLAG 'l'
#define LISTFILES_LFLAG "files-with-matches"
#define LISTFILES_GREP "-l"
extern int listfiles; // default false

// Print line number with output lines
#define LINENUMBERS_FLAG 'n'
#define LINENUMBERS_LFLAG "line-number"
#define LINENUMBERS_GREP "-n"
extern int linenumbers; // default false

// Print only a count of matching lines per FILE
#define COUNTMATCHES_FLAG 'c'
#define COUNTMATCHES_LFLAG "count"
#define COUNTMATCHES_GREP "-c"
extern int countmatches; // default false

// Force the PATTERN to match only whole words
#define WORDREGEX_FLAG 'w'
#define WORDREGEX_LFLAG "word-regex"
#define WORDREGEX_GREP "-w"
extern int wordregex; // default false

// Recurse downwards all directories
#define RECURSE_FLAG 'r'
#define RECURSE_LFLAG "recurse"
#define RECURSE_GREP "-r"
extern int recurse; // default false

// Print FILE names containing the matches
#define WITHFILENAME_FLAG 'H'
#define WITHFILENAME_LFLAG "with-filename"
#define WITHFILENAME_GREP "-H"
#define NOFILENAME_FLAG 'h'
#define NOFILENAME_LFLAG "no-filename"
#define NOFILENAME_GREP "-h"
extern int withfilename; // default true

// Use extended regular expression syntax
#define EXTENDEDREGEX_FLAG 'E'
#define EXTENDEDREGEX_LFLAG "extended-regex"
#define EXTENDEDREGEX_GREP "-E"
#define BASICREGEX_FLAG 'G'
#define BASICREGEX_LFLAG "basic-regex"
#define BASICREGEX_GREP "-G"
extern int extendedregex; // default false

// Print output colored like grep
#define COLOR_FLAG // NONE
#define COLOR_LFLAG //
#define COLOR_LFLAG1 "color"
#define COLOR_LFLAG2 "colour"
#define COLOR_GREP "--color=auto"
#define NOCOLOR_FLAG // NONE
#define NOCOLOR_LFLAG //
#define NOCOLOR_LFLAG1 "no-color"
#define NOCOLOR_LFLAG2 "no-colour"
#define NOCOLOR_GREP "--color=never"
extern int usecolor; // default true

// Use an exec call to the GNU grep function instead of our own grep
#define EXECGREP_FLAG // NONE
#define EXECGREP_LFLAG "execgrep"
extern int execgrep; // default true

// Spawn a process for each subdirectory traversed
#define MULTIPROCESS_FLAG 'P'
#define MULTIPROCESS_LFLAG //
#define MULTIPROCESS_LFLAG1 "fork"
#define MULTIPROCESS_LFLAG2 "multiprocess"
extern int multiprocess; // default false

// Launch a thread for each file grepped
#define MULTITHREADING_FLAG 'T'
#define MULTITHREADING_LFLAG //
#define MULTITHREADING_LFLAG1 "threads"
#define MULTITHREADING_LFLAG2 "multithreads"
extern int multithreading; // default true

// Choose traversal method
#define DIRTRAVERSAL_FLAG // NONE
#define DIRTRAVERSAL_LFLAG "dir"
#define FTSTRAVERSAL_FLAG // NONE
#define FTSTRAVERSAL_LFLAG "fts"
extern int dirtraversal; // default false
// ----> END OF OPTIONS OF SIMGREP



// <!--- SIMGREP POSITIONAL ARGUMENTS
// (POS #1) [REQUIRED] The regex pattern
extern char* pattern;

// (POS ...) [OPTIONAL] The array of filenames or directory names given
extern char** files;

// The number of filenames or directory names given.
extern size_t number_of_files;
// ----> END OF SIMGREP POSITIONAL ARGUMENTS



int parse_args(int argc, char** argv);

void print_usage();

void print_version();

void print_nopattern();

#endif // __OPTIONS_H__
