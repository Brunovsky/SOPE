#include "options.h"
#include "log.h"
#include "fifos.h"
#include "requests.h"
#include "signals.h"

#include <stddef.h>

int main(int argc, char** argv) {
    int s = 0;

    s = parse_args(argc, argv);
    if (s != 0) return s;

    s = set_signal_handlers();
    if (s != 0) return s;

    request_t* request = make_request();
    if (request == NULL) return 1;

    s = open_fifo_ans();
    if (s != 0) return s;

    set_alarm();

    s = write_fifo_requests(request->message);
    if (s != 0) return s;

    s = read_fifo_ans(&request->answer);
    if (s != 0) return s;

    s = parse_answer(request);
    if (s != 0) return s;

    clog_log(request);
    cbook_log(request);
    free_request(request);
    return 0;
}
