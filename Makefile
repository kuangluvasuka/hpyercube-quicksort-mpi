CC = mpic++
CFLAGS = -I.
OBJ = quicksort_mpi.o

TARGETS = quicksort
all: $(TARGETS)

%.o: %.cpp
	$(CC) -c -o $@ $^ $(CFLAGS)

quicksort: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -rf ./*.o *~ core $(TARGETS)
	
