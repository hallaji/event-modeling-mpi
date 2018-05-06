all: main
main: main.c
	mpicc -o main main.c
clean:
	rm main *.o
