OBJECTS = main.o options.o wthreads.o seats.o queue.o \
	log.o workers.o fifos.o signals.o
ONAME = server

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(WARNS) $(INCLUDES) -o $(ONAME) $(LDFLAGS) $(OBJECTS)
	@rm -f $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) $(WARNS) -c $*.c $(INCLUDES)

clean:
	rm -f $(OBJECTS) $(ONAME)