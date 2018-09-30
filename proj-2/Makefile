.PHONY: all clean server client

# gcc compiler
CC = gcc

CFLAGS = -pthread -O3 -march=native -lm

LDFLAGS = -lm -pthread -lrt

# include only headers in the same directory
INCLUDES = -I ./

# -Wno-unused-result: let writes fail silently.
# -Wno-unused-parameter: the signal handlers don't use their parameter.
WARNS = -Wall -Wextra -Wno-unused-result -Wno-unused-parameter

export CC INCLUDES CFLAGS LDFLAGS WARNS

all: server client

server:
	@$(MAKE) -C src/server
	mv src/server/server .

client:
	@$(MAKE) -C src/client
	mv src/client/client .

clean:
	@$(MAKE) clean -C src/server
	@$(MAKE) clean -C src/client
	rm -f server client
