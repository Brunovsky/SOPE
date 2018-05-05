.PHONY: all clean

CC = gcc
INCLUDES = -I ./
# Compiler flags
CFLAGS = -pthread -O2
# Linker flags
LDFLAGS = -lrt
# Warnings
WARNS = -Wall -Werror -Wno-unused-result

export CC INCLUDES LDFLAGS CFLAGS WARNS

all:
	@$(MAKE) -C server
	@$(MAKE) -C client

clean:
	@$(MAKE) clean -C server
	@$(MAKE) clean -C client