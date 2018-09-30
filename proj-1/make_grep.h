#ifndef MAKE_GREP_H___
#define MAKE_GREP_H___

char* const* make_grep(const char* file);

void free_grep(char* const* grep);

char* make_grep_string(const char* file);

char* join_string_array(char* const* string_arr, const char* joiner);

void print_command(char* const* argv);

const char* flatten_command(char* const* argv);

#endif // MAKE_GREP_H___
