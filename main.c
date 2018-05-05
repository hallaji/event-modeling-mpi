#include "mpi.h"
#include <time.h>
#include <stdio.h>
#include <unistd.h>

double getElapsed(struct timespec);
void execute(void (*f)(), double);
void sampling();
void positioning();

const double TIME_OVERALL_RUNTIME = 4;
const double TIME_SAMPLING_INTERVAL = 0.00001;
const int MAXIMUM_LOCATIONS = 1000;
const int ROOT_RANK = 0;

int rank, size, strikes = 0, samples = 0;

/**
 * Main function controls operation of the program.
 * 
 * @param rgc The number of arguments passed to the program
 * @param argv A one-dimensional array of arguments
 * @return zero if it is successful, non-zero indicating error otherwise.
 */
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    execute(sampling, TIME_OVERALL_RUNTIME);

    MPI_Finalize();

    return 0;
}

/**
 * Get overall elapsed seconds from a time to now 
 * 
 * @param time A timespec
 * @return number of seconds
 */
double getElapsed(struct timespec time)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    double timeInSeconds = (double)(time.tv_sec + time.tv_nsec / 1.0e9);
    double nowInSeconds = (double)(now.tv_sec + now.tv_nsec / 1.0e9);

    return nowInSeconds - timeInSeconds;
}

/**
 * Execute a function in a period of time (seconds)
 * 
 * @param f The function to execute
 * @param duration The number of seconds
 */
void execute(void (*f)(double), double duration)
{
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    double stopwatch = 0.0;
    do
    {
        (*f)(stopwatch);

        stopwatch = getElapsed(start);
    } while (stopwatch < duration);
}

/**
 * Do sampling at a particular time
 * @param moment 
 */
void sampling(double moment)
{
    MPI_Bcast(&moment, 1, MPI_DOUBLE, ROOT_RANK, MPI_COMM_WORLD);
    sleep(1); // Prevent repeating prints
    printf("Sampling, Rank: %d\n", rank);

    execute(positioning, TIME_SAMPLING_INTERVAL);
}

/**
 * Do positioning at a particular time
 * @param moment
 */
void positioning(double moment)
{
    printf("Positioning, Rank: %d\n", rank);
}
