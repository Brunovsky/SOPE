#include "options.h"
#include "log.h"
#include "fifos.h"
#include "requests.h"
#include "signals.h"

#include <stdlib.h>

int main(int argc, char** argv) {
    parse_args(argc, argv);
    set_signal_handlers();

    make_request();
    open_fifo_ans();
    set_alarm();

    write_fifo_requests(request->message);
    read_fifo_ans(&request->answer);

    parse_answer();

    clog_log();
    cbook_log();
    
    exit(EXIT_SUCCESS);
}
