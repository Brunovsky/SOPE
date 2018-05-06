#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

#define ERROR_NUMBER_LENGTH 2
#define ERROR_MESSAGE_LENGTH 3

#define ERROR_MESSAGE_TIMEOUT "OUT"
#define ERROR_MESSAGE_MAX "MAX"
#define ERROR_MESSAGE_NST "NST"
#define ERROR_MESSAGE_IID "IID"
#define ERROR_MESSAGE_ERR "ERR"
#define ERROR_MESSAGE_NAV "NAV"
#define ERROR_MESSAGE_FUL "FUL"

#define LOG_FILE_NAME "clog.txt"

//falta testar erros com ficheiros
int write_to_log_file(char data[80], unsigned char exceded_time) {
//	printf("\nsizeof -1-> %d", strlen(data));
//	printf("\ndata[1]-> %d", data[1]);

	printf("\n1");
	int fp_log_file = open(LOG_FILE_NAME, O_WRONLY | O_APPEND);
	int pid = getpid();
	char pid_temp[8];
	sprintf(pid_temp, "%05d", pid);
	char line[18];

	if (strlen(data) == ERROR_NUMBER_LENGTH) {
		char error[5];
		strcat(line, pid_temp);
		strcat(line, " ");

		if (exceded_time == 1) {
			strncpy(error, ERROR_MESSAGE_TIMEOUT, ERROR_MESSAGE_LENGTH);
		} else {
			switch (data[1]) {
			case 1:
				strncpy(error, ERROR_MESSAGE_MAX, ERROR_MESSAGE_LENGTH);
				break;
			case 2:
				strncpy(error, ERROR_MESSAGE_NST, ERROR_MESSAGE_LENGTH);
				break;
			case 3:
				strncpy(error, ERROR_MESSAGE_IID, ERROR_MESSAGE_LENGTH);
				break;
			case 4:
				strncpy(error, ERROR_MESSAGE_ERR, ERROR_MESSAGE_LENGTH);
				break;
			case 5:
				strncpy(error, ERROR_MESSAGE_NAV, ERROR_MESSAGE_LENGTH);
				break;
			case 6:
				strncpy(error, ERROR_MESSAGE_FUL, ERROR_MESSAGE_LENGTH);
				break;
			}
		}

		strcat(line, error);
		strcat(line, "\n");

//		printf("\n%s", line);
		write(fp_log_file, line, strlen(line));

	} else {
		char str_temp[80];
		strcpy(str_temp, data);

		const char s[2] = " ";
		char *token;

		token = strtok(data, s);

//		ssize_t num_char_written;
		int size = 0;
		while (token != NULL) {
			size++;
			token = strtok(NULL, s);
		}
		size--;

		token = strtok(str_temp, s);

		char number_temp[5], token_temp[10], i = 0;
//		printf("\n\n");
		while (token != NULL) {
			if (i != 0) {
				memset(line, 0, sizeof line);
				strcat(line, pid_temp);
				strcat(line, " ");

				snprintf(number_temp, sizeof(number_temp), "%02d", i);
				strcat(line, number_temp);
				strcat(line, ".");
				snprintf(number_temp, sizeof(number_temp), "%02d", size);
				strcat(line, number_temp);
				strcat(line, " ");
				sprintf(token_temp, "%05d", atoi(token));
				strcat(line, token_temp);
				strcat(line, "\n");

//				printf("\n%s", line);
				write(fp_log_file, line, strlen(line));
			}

			token = strtok(NULL, s);
			i++;
		}

//		printf("\n");
	}

	close(fp_log_file);
	return 0;

}
