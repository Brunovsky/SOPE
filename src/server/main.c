#include "options.h"
#include "queue.h"
#include "seats.h"
#include "workers.h"
#include "signals.h"
#include "fifos.h"
#include "log.h"

#include <stdlib.h>

int main(int argc, char** argv) {
	parse_args(argc, argv);
	set_signal_handlers();

	clear_client_files();
	open_slog();

	setup_seats();
	setup_queue();
	launch_workers();
	set_alarm();

	open_fifo_requests();
	fifo_read_loop();

	exit(EXIT_SUCCESS);
}
