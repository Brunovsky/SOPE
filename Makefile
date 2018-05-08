.PHONY: all clean server client

CC = gcc
INCLUDES = -I ./

CFLAGS = -pthread -Og -g -march=native

LDFLAGS = -lrt

WARNS = -Wall -Wextra \
	-Wno-unused-result -Wno-unused-parameter

ROOT_PATH = ~/cinema

export CC INCLUDES LDFLAGS CFLAGS WARNS ROOT_PATH

all: server client

server:
	@$(MAKE) -C src/server

client:
	@$(MAKE) -C src/client

clean:
	@$(MAKE) clean -C src/server
	@$(MAKE) clean -C src/client
