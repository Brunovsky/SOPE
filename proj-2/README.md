# cinema

## Unspecified
A request which could not be returned to the client,
as it was timed out or access to ansXXXXX failed:
Should it be logged in slog.txt / sbook.txt and kept, or should it be freed?

Chosen: It is kept and logged.

(Selfnote) If it should be freed:
    Go to worker() in workers.c and check answer_client(),
    if it fails call free_reserved_seats(request) instead of logging the request.

## Script confirm

Run as ./confirm after the server closes. It will compare only the seats successfully reserved.

#### slog.txt VS sbook.txt
The reserved seats should be the same.
That is: when slog.txt is filtered and sorted, its contents should match those of sbook.txt

#### clog.txt VS cbook.txt
The reserved seats should be the same.
Same deal: when clog.txt is filtered and sorted, and cbook.txt is also sorted,
their contents should match exactly.

#### sbook.txt VS cbook.txt
sbook.txt is naturally ordered, so the ordered version of cbook.txt should match sbook.txt exactly.

## Script multiclient
A quick tool to launch N clients from bash.

Syntax:
    
    ./multiclient [valgrind] n_clients timeout number total seats

n_clients --> number of clients to launch.
timeout --> each client's timeout.
number --> each client may request a number of seats in the range 1..number.
total --> each client may request a number of seats in the range (2*number)/3..total.
seats --> each client may choose seat numbers in the range 0..seats.
