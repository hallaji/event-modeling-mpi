all: fleet
fleet: fleet.c
	mpicc -o fleet fleet.c
clean:
	rm -f fleet *.o *.txt
