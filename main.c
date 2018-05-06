#include "mpi.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

double getElapsed(struct timespec);
double getRate(int, int);
int getRandomLocation();

void execute(void (*f)(double *), double);
void sampling();
void locating();

const double TIME_OVERALL_RUNTIME = 10;
const double TIME_SAMPLING_INTERVAL = 0.006;
const int MAXIMUM_LOCATIONS = 1000;
const int MASTER = 0;
const int LOG = 1;

int rank, size;
int strikes = 0, samples = 0;

MPI_Status status;

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

    // execute(sampling, TIME_OVERALL_RUNTIME);

    // MPI_Barrier(MPI_COMM_WORLD);
    if (rank == MASTER)
    {
        printf("\n\n");
        printf("Number of Strikes: %d\n", strikes);
        printf("Number of Samples: %d\n", samples);
        printf("Rate of strikes over samples: %f\n", getRate(strikes, samples));
        fflush(stdout);
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

int getRandomLocation()
{
    srand((rank + 1) * clock());
    return rand() % MAXIMUM_LOCATIONS + 1;
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
    // printf("sampling Rank%d instant: %f\n", rank, *instant);
    MPI_Bcast(instant, 1, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
    execute(locating, TIME_SAMPLING_INTERVAL);
}

/**
 * Do locating at a particular time
 * @param instant
 */
void locating(double *instant)
{
    // printf("sampling Rank%d instant: %f\n", rank, *instant);
    MPI_Bcast(instant, 1, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);

    // char message[BUFSIZ];
    // int location;
    // int oddVessels;
    // int evenVessels;

    if (rank != MASTER)
    {
        int location = getRandomLocation();
        // MPI_Pack(&oddVessels, 1, MPI_FLOAT, message, BUFSIZ, &position, MPI_COMM_WORLD);

        // if (rank % 2 == 0) {
        //     MPI_Reduce(1, &evenVessels, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        // }
        // else {
        //     MPI_Reduce(1, &oddVessels, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        // }

        MPI_Send(&location, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
        // printf("Rank %d sent location %d to master.", rank, location);
    }
    else
    {
        samples++;

        typedef struct
        {
            int number;
            int oddVessels;
            int evenVessels;
            int strikes;
        } Location;

        Location locations[MAXIMUM_LOCATIONS];
        for (int i = 0; i < MAXIMUM_LOCATIONS; i++)
        {
            locations[i].number = 0;
            locations[i].oddVessels = 0;
            locations[i].evenVessels = 0;
            locations[i].strikes = 0;
        }

        int location2;
        for (int process = 1; process < size; process++)
        {
            MPI_Recv(&location2, 1, MPI_INT, process, 0, MPI_COMM_WORLD, &status);
            // printf("received %d from rank: %d\n", location2, process);
            // locations[location].number = location;
            // if (process % 2 == 0)
            // {
            //     locations[location].evenVessels++;
            // }
            // else
            // {
            //     locations[location].oddVessels++;
            // }
        }

        
        for (int index = 0; index < MAXIMUM_LOCATIONS; index++)
        {
            int locationStrikes = 0;
            // if (locations[index].oddVessels >= 1) {
            //    locationStrikes++; 
            // }
            if (locations[index].oddVessels >= 1 && locations[index].evenVessels >= 2) {
               locationStrikes++; 
            } 

            // if (locations[index].oddVessels >= 1 ) // && (locations[index].oddVessels * 2) <= locations[index].evenVessels
            // {
            //     locationStrikes = locations[index].oddVessels;
            // }
            // else if (locations[index].evenVessels >= 2 ) // && (locations[index].evenVessels / 2) <= locations[index].oddVessels
            // {
            //     locationStrikes = locations[index].evenVessels / 2;
            // }

            if (locationStrikes > 0) {
               strikes += locationStrikes; 
               locations[index].strikes = locationStrikes;
            //    printf("Location %d has %d strikes.\n", index, locationStrikes);
            //    fflush(stdout);
            }

            if (LOG)
            {
                
                // fflush(stdout);
            }

        }
        
    }

    // if (LOG)
    // {
    //     printf("Vessel %d is located in %d\n", rank, location);
    //     fflush(stdout);
    // }
}
