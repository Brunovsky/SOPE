.PHONY: all clean

CC = gcc

INCLUDES = -I ./

# Let writes fail silenty (Wno-unused-result)
CFLAGS = -pthread -O2 -Wall -Wno-unused-result -march=native

OBJECTS = main.o options.o register.o processes.o simgrep.o \
	make_grep.o traversal.o wthreads.o

ONAME = simgrep

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(ONAME) $(OBJECTS)
	rm -f $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $*.c $(INCLUDES)

clean:
	rm -f $(OBJECTS) $(ONAME)
