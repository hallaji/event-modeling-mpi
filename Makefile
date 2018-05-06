all: main
main: main.c
	mpicc -o main main.c
clean:
	rm -f main *.o *.txt
