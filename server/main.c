#include "options.h"
#include "queue.h"
#include "seats.h"
#include "workers.h"
#include "signals.h"
#include "fifos.h"
#include "log.h"

#include <stdio.h>

int fifo_read_loop() {
	while (1) {
		const char* message;
		int s = read_fifo_requests(&message);

		if (s == 0) {
			printf("message. %s\n", message);
			write_message(message);
		} else {
			printf("error: %d\n", s);
			if (alarm_timeout()) {
				return 0;
			}
		}
	}
}

int main(int argc, char** argv) {
	int s = 0;

	s = parse_args(argc, argv);
	if (s != 0) return s;

	s = open_slog();
	if (s != 0) return s;

	s = open_fifo_requests();
	if (s != 0) return s;

	s = set_signal_handlers();
	if (s != 0) return s;

	setup_queue();
	setup_seats();
	launch_workers();
	set_alarm();

	printf("Server started...\n");

	fifo_read_loop();

	printf("Server exiting...\n");

	terminate_workers();
	return 0;
}