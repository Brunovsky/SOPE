CC = gcc
INCLUDES = -I ./
# Let writes fail silenty (Wno-unused-result)
CFLAGS = -pthread -O2 -Wall -Wno-unused-result
OBJECTS = test.o wthreads.o
ONAME = test

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(ONAME) $(OBJECTS)
	rm -f $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $*.c $(INCLUDES)

clean:
	rm -f $(OBJECTS) $(ONAME)