.PHONY: all clean server client

CC = gcc
INCLUDES = -I ./

CFLAGS = -pthread -Og -march=native

LDFLAGS = -lrt

# Let writes fail silently, for now
WARNS = -Wall -Wno-unused-result

export CC INCLUDES LDFLAGS CFLAGS WARNS

all:
	@$(MAKE) -C server
	@$(MAKE) -C client

server:
	@$(MAKE) -C server

client:
	@$(MAKE) -C client

clean:
	@$(MAKE) clean -C server
	@$(MAKE) clean -C client