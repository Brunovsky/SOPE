#ifndef FIFOS_H___
#define FIFOS_H___

int open_fifo_ans();

void close_fifo_ans();

int read_fifo_ans(const char** message_p);

int write_fifo_requests(const char* message);

#endif // FIFOS_H___
