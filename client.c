//client <time_out> <num wanted seats> <pref_seat_list>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

#include "output.c"

#define BUFFER_LENGTH 5
#define ERROR -1
#define FINE 0

char *fifo_path_client = "/tmp/ans";

char *time_out;
char *num_wanted_seats;
char *pref_seat_list;

char fifo_dir_path[20];
clock_t enlapsed_time;
int pid;
unsigned char exceded_time = 0;

//needs to validate inputtime_out
char configure_options(int argc, char* argv[]) {

	if (argc != 4) {
		return ERROR;
	}

	time_out = argv[1];
	num_wanted_seats = argv[2];
	pref_seat_list = argv[3];

//	printf("\ntime_out-> %s", time_out);
//	printf("\nnum_wanted_seats-> %s", num_wanted_seats);
//	printf("\npref_seat_list-> %s", pref_seat_list);
//	printf("\n");
	return FINE;

}

void create_fifo() {

	strcpy(fifo_dir_path, fifo_path_client);
	pid = getpid();

	char pidString[100];
	sprintf(pidString, "%d", pid);
	strcat(fifo_dir_path, pidString);
//	printf("\nfifo_dir_path-> %s", fifo_dir_path);
	mkfifo(fifo_dir_path, 0660);
//	printf("\n");
}

void initiate_clock() {
	enlapsed_time = clock();
}

void send_information() {
	int fifo_req;
	do {
		fifo_req = open("/tmp/requests", O_WRONLY);
		if (fifo_req == -1)
			sleep(1);
	} while (fifo_req == -1);

	char message[100];
	sprintf(message, "%d %s %s\n", getpid(), num_wanted_seats, pref_seat_list);
	write(fifo_req, message, strlen(message));

	initiate_clock();

}

unsigned char check_time_out() {
	enlapsed_time = clock() - enlapsed_time;
	double time_taken = ((double) enlapsed_time) / CLOCKS_PER_SEC; // in seconds
	printf("answer took %f seconds to arrive \n", time_taken);

	return (time_taken > atof(time_out)) ? 1 : 0;

}

void read_information() {

	int fifo_ans;
	char answer[256];

	do {
		fifo_ans = open(fifo_dir_path, O_RDONLY);
		if (fifo_ans == -1)
			sleep(1);
	} while (fifo_ans == -1);

	read(fifo_ans, answer, sizeof(answer));

	write_to_log_file(answer, check_time_out());
}

int main(int argc, char* argv[]) {
	if (configure_options(argc, argv) == ERROR) {
		printf("Usage: %s <time_out> <num_wanted_seats> <pref_seat_list>\n",
				argv[0]);
		exit(1);
	}
	create_fifo();
	send_information();
	read_information();
}
