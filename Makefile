CC=~/msp430/bin/msp430-gcc
CFLAGS=-Os -Wall -mmcu=msp430x2013 -std=gnu99

OBJS=main.o 


all: $(OBJS)
	$(CC) $(CFLAGS) -o main.elf $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -fr main.elf $(OBJS)
