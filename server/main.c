#include "options.h"
#include "register.h"
#include "simgrep.h"
#include "processes.h"
#include "traversal.h"

int main(int argc, char** argv) {
	timestamp_epoch();
	
	int parse_s = parse_args(argc, argv);
	if (parse_s != 0) return parse_s;

	int logfile_s = open_logfile();
	if (logfile_s != 0) return logfile_s;

	int regex_s = validate_regex_pattern();
	if (regex_s != 0) return regex_s;

	int signals_s = set_main_signal_handlers();
	if (signals_s != 0) return signals_s;

	if (number_of_files > 0) {
		if (multiprocess) {
			if (dirtraversal) {
				init_traversal_dir();
			} else {
				init_traversal_fts();
			}
		} else {
			if (dirtraversal) {
				init_traversal_dir_singleprocess();
			} else {
				init_traversal_fts_singleprocess();
			}
		}
	} else {
		grep_stdin();
	}

	waitall_children();
	return 0;
}