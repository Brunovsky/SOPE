#!/bin/bash

waitenter() {
    echo
    echo "Done $1"
    echo
    read key
    clear
}

call-valgrind() {
    valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all "$@"
}

debug() {
    # Init server
    call-valgrind ./server 100 7 30 &
    SERVER_PID=$!
    waitenter "server"


    # Client 1: Reserves seats 10, 20, 30 successfully
    # + Unordered seats
    call-valgrind ./client 5 3 "10 30 20"
    waitenter "client 1"


    # Client 2: Reserves seats 5, 15, 25, 35, 45, 55 successfully
    # + More seats than requested
    call-valgrind ./client 3 6 "55 35 45 25 5 15 65 85 75"
    waitenter "client 2"


    # Client 3: Reserves seats 90, 95, 100 successfully
    # + Duplicates of available requested seats
    # + Duplicates of unavailable requested seats
    call-valgrind ./client 3 3 "30 35 95 25 55 30 100 10 100 20 5 90"
    waitenter "client 3"


    # Client 4: Fails with NST
    call-valgrind ./client 3 4 "12 14"
    waitenter "client 4"


    # Client 5: Fails with NST
    call-valgrind ./client 3 0 "85 66 73"
    waitenter "client 5"


    # Client 6: Fails with IID
    call-valgrind ./client 3 3 "14 5 17 109"
    waitenter "client 6"


    # Client 7: Fails with IID
    call-valgrind ./client 3 3 "13 0 89 14"
    waitenter "client 7"


    # Client 8: Fails with NAV (10 and 20 reserved)
    call-valgrind ./client 3 4 "60 80 20 40 10"
    waitenter "client 8"

    /bin/kill -TERM $SERVER_PID
    waitenter
}

debug "$@"

exit 0
