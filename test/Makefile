
CC = gcc
CFLAGS = -I$(IDIR) -Wall -fopenmp -lm

driver: driver.c
	$(CC) -o $@ $^ $(CFLAGS) 

all: driver

.PHONY: clean

clean:
	rm -f driver