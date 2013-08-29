# This make file creates a library for the gpio pins.
CC=gcc
CCFLAGS=-Wall -Werror -g -I../include -Iinc

AR=ar
ARFLAGS=-rcs

OUT_DIR=output
LIB_DIR=../library
LIB_NAME=librpigpio.a

VPATH= $(OUT_DIR) $(LIB_DIR)

all: dirs $(LIB_NAME)

$(LIB_NAME): gpio.o i2c.o
	$(AR) $(ARFLAGS) $(LIB_DIR)/$@ $(OUT_DIR)/gpio.o $(OUT_DIR)/i2c.o

%.o: %.c
	$(CC) $(CCFLAGS) -o $(OUT_DIR)/$@ -c $<

dirs:
	test -d $(OUT_DIR) || mkdir $(OUT_DIR);
	test -d $(LIB_DIR) || mkdir $(LIB_DIR);

clean:
	-rm $(OUT_DIR)/*.o;
	-rmdir $(OUT_DIR);
	-rm $(LIB_DIR)/*.a;
	-rmdir $(LIB_DIR);
