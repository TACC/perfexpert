
OBJECTS = instrument.o
BINARIES = instrument

all:    $(OBJECTS)
	$(CC) -o instrument instrument.o -lpthread

inst:
	macpo.sh --macpo:instrument=thread_func -c instrument.c
	macpo.sh --macpo:instrument=thread_func -o instrument instrument.o -lpthread

clean:
	$(RM) -f $(OBJECTS) $(BINARIES) rose_*
