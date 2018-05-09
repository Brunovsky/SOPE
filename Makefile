.PHONY: all clean server client

CC = gcc
INCLUDES = -I ./

CFLAGS = -pthread -Og -g -march=native -lm

LDFLAGS = -lm -pthread -lrt

WARNS = -Wall -Wextra \
	-Wno-unused-result -Wno-unused-parameter

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
	rm server client
