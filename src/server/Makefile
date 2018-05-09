OBJECTS = main.o options.o wthreads.o seats.o queue.o \
	log.o workers.o fifos.o signals.o requests.o
ONAME = server

all: $(OBJECTS)
	$(CC) $(WARNS) $(INCLUDES) -o $(ONAME) $(OBJECTS) $(LDFLAGS)
	@rm -f $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) $(WARNS) -c $*.c $(INCLUDES)

clean:
	@rm -f $(OBJECTS) $(ONAME)
