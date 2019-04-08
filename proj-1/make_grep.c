#include "make_grep.h"
#include "options.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Constructs a string array argv which can be passed to execvp for a proper
 * call of GNU grep with the given options and the single file.
 *
 * If execvp is not called with the string array provided, the array should
 * be freed with free_grep.
 */
char* const* make_grep(const char* file) {
	int i = 0;
	char** grep = calloc(20, sizeof(char*));

	// *** grep
	grep[i++] = strdup("grep");

	// *** [option]...
	if (icase)          grep[i++] = strdup(ICASE_GREP);
	if (listfiles)      grep[i++] = strdup(LISTFILES_GREP);
	if (linenumbers)    grep[i++] = strdup(LINENUMBERS_GREP);
	if (countmatches)   grep[i++] = strdup(COUNTMATCHES_GREP);
	if (wordregex)      grep[i++] = strdup(WORDREGEX_GREP);
	if (withfilename)   grep[i++] = strdup(WITHFILENAME_GREP);
	if (!withfilename)  grep[i++] = strdup(NOFILENAME_GREP);
	if (extendedregex)  grep[i++] = strdup(EXTENDEDREGEX_GREP);
	if (usecolor)       grep[i++] = strdup(COLOR_GREP);
	if (!usecolor)      grep[i++] = strdup(NOCOLOR_GREP);

	// *** pattern
	grep[i++] = strdup(pattern);

	// *** file
	if (file != NULL) grep[i++] = strdup(file);

	// *** end
	return grep;
}

/**
 * Frees a previously allocated grep string array argv made by make_grep
 */
void free_grep(char* const* grep) {
	size_t i = 0;
	while (grep[i] != NULL) free((void*)grep[i++]);
	free((void*)grep);
}

/**
 * Constructs a string which can be passed to system() call for a proper
 * call of GNU grep with the given options and the single file.
 */
char* make_grep_string(const char* file) {
	char* grep = calloc(300 + strlen(file), sizeof(char));

	// *** grep command
	strcpy(grep, "grep");

	// *** simgrep options
	if (icase)         strcat(grep, " " ICASE_GREP);
	if (listfiles)     strcat(grep, " " LISTFILES_GREP);
	if (linenumbers)   strcat(grep, " " LINENUMBERS_GREP);
	if (countmatches)  strcat(grep, " " COUNTMATCHES_GREP);
	if (wordregex)     strcat(grep, " " WORDREGEX_GREP);
	if (withfilename)  strcat(grep, " " WITHFILENAME_GREP);
	if (!withfilename) strcat(grep, " " NOFILENAME_GREP);
	if (extendedregex) strcat(grep, " " EXTENDEDREGEX_GREP);
	if (usecolor)      strcat(grep, " " COLOR_GREP);
	if (!usecolor)     strcat(grep, " " NOCOLOR_GREP);

	// *** pattern
	strcat(strcat(strcat(grep, " \""), pattern), "\"");

	// *** file
	if (file != NULL) strcat(strcat(grep, " "), file);

	// *** end
	return grep;
}

/**
 * Joins a string array using a custom joiner string.
 * The string is allocated with one extra space for an optional newline
 * at the end.
 */
char* join_string_array(char* const* string_arr,
		const char* joiner) {
	if (string_arr == NULL || string_arr[0] == NULL) return NULL;

	size_t s = 0, i = 0;
	while (string_arr[i] != NULL) {
		s += strlen(string_arr[i++]);
	}

	char* str = calloc(s + i * strlen(joiner) + 3, sizeof(char));

	strcpy(str, string_arr[0]);

	i = 1;
	while (string_arr[i] != NULL) {
		strcat(strcat(str, joiner), string_arr[i++]);
	}

	return str;
}

/**
 * Prints a string array argv to standard out.
 */
void print_command(char* const* argv) {
	char* s = join_string_array(argv, " ");
	strcat(s, "\n");
	write(STDOUT_FILENO, s, strlen(s));
}

/**
 * Constructs a command string given a string array argv.
 */
const char* flatten_command(char* const* argv) {
	return join_string_array(argv, " ");
}
