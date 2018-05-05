#ifndef QUEUE_H___
#define QUEUE_H___

void setup_queue();

int read_message(const char** message_p);

int write_message(const char* message);

#endif // QUEUE_H___