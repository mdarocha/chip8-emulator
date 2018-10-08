CC = gcc
CCFLAGS += -g -Wall
LDFLAGS += -lGL -lglut

all: chip

chip: chip.c main.c
	$(CC) $(CCFLAGS) $^ -o chip $(LDFLAGS)

clean:
	rm -f chip
