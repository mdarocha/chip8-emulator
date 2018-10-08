CC = gcc
CCFLAGS += -g -Wall
LDFLAGS =

all: chip

chip: chip.c main.c
	$(CC) $(CCFLAGS) $^ -o chip $(LDFLAGS)

clean:
	rm -f chip
