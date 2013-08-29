CC=gcc
LD=$(CC)

CFLAGS=-g -lwiringPi -Wall -O2 -DVERSION_STR=\"1.0\"
LDFLAGS= -lwiringPi 

PROG=apuplay
OBJS = $(patsubst %.c,%.o,$(wildcard *.c))

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(PROG)
