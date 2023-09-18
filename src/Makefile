IDIR = ../include
ODIR = ../temp
CC = gcc
CFLAGS = -I$(IDIR) -Wall -fopenmp -lm

_DEPS = fish.h util.h constants.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = fish.o util.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: seq_fss parallel_fss

seq_fss: $(OBJ) $(ODIR)/sequential_fss.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

parallel_fss: $(OBJ) $(ODIR)/parallel_fss.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o 
	rm -f ./seq_fss
	rm -f ./parallel_fss





