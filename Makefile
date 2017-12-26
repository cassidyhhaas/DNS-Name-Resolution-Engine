CC= gcc
CFLAGS = -c -g -Wall -Wextra
LFLAGS = -Wall -Wextra -pthread

.PHONY: all clean

all: multilookup

multilookup: multilookup.o util.o
	$(CC) $(LFLAGS) $^ -o $@

multilookup.o: multilookup.c multilookup.h
	$(CC) $(CFLAGS) $<

util.o: util.c util.h
		$(CC) $(CFLAGS) $<

clean:
	rm -f multilookup
	rm -f *.o
	rm -f *~
	rm -f results.txt
	rm -f serviced.txt
