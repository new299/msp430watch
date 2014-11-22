CC=msp430-gcc
CFLAGS=-Os -Wall -mmcu=msp430f2013 -std=gnu99

OBJS=msp430watch.elf

all: $(OBJS)
	

%.elf: %.o
	$(CC) $(CFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.program: %.elf
	mspdebug uif -d /dev/ttyUSB1 "prog $<"

clean:
	rm *.o *.elf
