CC = gcc
CCFLAGS += -g -Wall -DDEBUG
LDFLAGS += -lGL -lglut

all: chip

chip: chip.c chip.h main.c text.h
	$(CC) $(CCFLAGS) $^ -o chip $(LDFLAGS)

clean:
	rm -f chip
