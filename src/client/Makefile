OBJECTS = main.o options.o requests.o signals.o log.o fifos.o
ONAME = client

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(WARNS) $(INCLUDES) -o $(ONAME) $(LDFLAGS) $(OBJECTS)
	@rm -f $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) $(WARNS) -c $*.c $(INCLUDES)

clean:
	@rm -f $(OBJECTS) $(ONAME)
