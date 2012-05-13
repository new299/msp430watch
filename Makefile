CC=msp430-gcc
CFLAGS=-Os -Wall -mmcu=msp430x2013 -std=gnu99

OBJS=main_colourlcd.elf main_3310test.elf

all: $(OBJS)
	

%.elf: %.o
	$(CC) $(CFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o *.elf
