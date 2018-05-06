#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

const char* opt[] = {
    "%d 4 3 6 1 2 7 1 3\n",
    "%d 2 1 2 3 4\n",
    "%d 3 8 5 2 9 2\n",
    "%d 1 10 11\n",
    "%d 1 3\n",
    "%d 4 2 3\n", // error
    "%d 2 7 13 2\n",
    "%d 3 13 14 5 17  18 \n",
    "%d 6 9 4 15 19 20 1 23 21\n"
};

int fail(const char* message) {
    int err = errno;
    printf("%s: %s\n", message, strerror(errno));
    return err;
}

int main(int argc, char** argv) {
    const char* p = opt[atoi(argv[1])];
    char str[1024];
    int pid = getpid();
    sprintf(str, p, pid);

    int requests_no = open("requests", O_WRONLY);
    if (requests_no == -1)
        return fail("Failed to open requests");
    
    char ans_name[32];
    sprintf(ans_name, "ans%d", pid);

    int s = mkfifo(ans_name, 0660);
    if (s != 0)
        return fail("Failed to open fifo");

    write(requests_no, str, strlen(str));
    // Let it block
    int ans_no = open(ans_name, O_RDONLY);

    printf("Open returned: %d\n", ans_no);
    char buf[4096];
    int z = read(ans_no, &buf, 4096);
    close(ans_no);
    return z;
}