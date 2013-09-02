CC=gcc
LD=$(CC)

CFLAGS=-g -Wall -O2 -DVERSION_STR=\"1.02\" -DPROGRESS_SPINNER -I.
LDFLAGS=

PROG=apuplay.exe
OBJS = $(patsubst %.c,%.o,$(wildcard ../*.c))

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG)

%.o: ../%.c ../%.h
	$(CC) $(CFLAGS) -c $<

clean:
	del ..\*.o 
	del apuplay.exe
