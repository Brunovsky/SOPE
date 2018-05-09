#ifndef FIFOS_H___
#define FIFOS_H___

int open_fifo_requests();

int read_fifo_requests(const char** message_p);

int write_to_fifo(const char* fifoname, const char* message);

int fifo_read_loop();

#endif // FIFOS_H___
