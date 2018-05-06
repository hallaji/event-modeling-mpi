#include "mpi.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

double getElapsed(struct timespec);
double getRate(int, int);
int getRandom();

void execute(void (*f)(double *), double);
void sampling();
void locating();

const double TIME_OVERALL_RUNTIME = 60;
const double TIME_SAMPLING_INTERVAL = 0.01;
const int MAXIMUM_LOCATIONS = 1000;
const int MASTER = 0;

int rank, size, strikes = 0, samples = 0;

/**
 * Main function controls operation of the program.
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

    if (rank == MASTER)
    {
        printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
        printf("%d strikes detected from %d samples.\nRate of strikes over samples is %f.\n",
               strikes, samples, getRate(strikes, samples));
        printf("%d locations in %.0fsec overall runtime and %.3fsec sampling interval.\n",
               MAXIMUM_LOCATIONS, TIME_OVERALL_RUNTIME, TIME_SAMPLING_INTERVAL);
    }

    MPI_Finalize();

    return 0;
}

/**
 * Get overall elapsed seconds from a time to now
 * @param time A timespec
 * @return Number of seconds
 */
double getElapsed(struct timespec time)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    return (double)(now.tv_sec + now.tv_nsec / 1.0e9) - (double)(time.tv_sec + time.tv_nsec / 1.0e9);
}

/**
 * Get rate of strikes over samples
 * @param strikes The number of strikes
 * @param samples The number of samples
 * @return The rate of strikes over samples
 */
double getRate(int strikes, int samples)
{
    if (samples == 0)
    {
        return 0;
    }

    return strikes / (double)samples;
}

/**
 * Random a number between 0 and maximum number of locations.
 * @return A random integer
 */
int getRandom()
{
    srand((rank + 1) * clock());

    return rand() % (MAXIMUM_LOCATIONS - 1) + 1;
}

/**
 * Execute a function in a period of time (seconds)
 * @param f The function to execute
 * @param duration The number of seconds
 */
void execute(void (*f)(double *), double duration)
{
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    double stopwatch = 0.0;
    do
    {
        (*f)(&stopwatch);
    } while ((stopwatch = getElapsed(start)) < duration);
}

/**
 * Do sampling at a particular time
 * @param instant
 */
void sampling(double *instant)
{
    if (rank == MASTER)
    {
        samples++;
    }
    execute(locating, TIME_SAMPLING_INTERVAL);
}

/**
 * Do locating at a particular time
 * @param instant
 */
void locating(double *instant)
{
    int location;

    if (rank != MASTER)
    {
        location = getRandom();
        MPI_Send(&location, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
    }
    else
    {
        typedef struct
        {
            int oddVessels;
            int evenVessels;
            int strikes;
        } Location;
        Location locations[MAXIMUM_LOCATIONS] = {{0, 0, 0}};

        MPI_Status status;
        for (int process = 1; process < size; process++)
        {
            MPI_Recv(&location, 1, MPI_INT, process, 0, MPI_COMM_WORLD, &status);
            if (process % 2 == 0)
            {
                locations[location].evenVessels++;
            }
            else
            {
                locations[location].oddVessels++;
            }
        }

        for (int index = 0; index < MAXIMUM_LOCATIONS; index++)
        {
            if (locations[index].oddVessels >= 1 && locations[index].evenVessels >= 2)
            {
                locations[index].strikes++;
                strikes += locations[index].strikes;
                printf("Strike! Location #%03d: %d odd & %d even vessels at %f instant of #%03d sampling interval.\n",
                       index,
                       locations[index].oddVessels,
                       locations[index].evenVessels,
                       *instant,
                       samples);
            }
        }
    }
}
