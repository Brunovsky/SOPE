#ifndef QUEUE_H___
#define QUEUE_H___

typedef pid_t client_t;

void setup_workers();

void add_request(client_t client, char* req);

int get_request_result(client_t client);

#endif // QUEUE_H___