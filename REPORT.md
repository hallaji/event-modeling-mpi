# Report

## 1. Program Structure

### Main Function

The `main()` function controls the operation of the program. `MPI_Init` initializes the MPI execution environment. The `argc` is the pointer to the number of arguments and `argv` is the pointer to the argument vector. Furthermore, `MPI_Finalize` terminates MPI execution environment at the end of program.

In addition, `MPI_Comm_size` determines the size of the group associated with a communicator. The output is the number of processes in the group of communicator. On the other hand, `MPI_Comm_rank` determines the rank of the calling process in the communicator. `MPI_COMM_WORLD` is used as the communicator. The main function prints out the metrics if the current rank is 0 `MASTER`. The metrics are the number of strikes, the rate of strikes over samples, the total number of sampling intervals and the defualt overall runtime.

```
execute(sampling, TIME_OVERALL_RUNTIME);
```

The `execute()` accepts `sampling()` callback as the first argument and `TIME_OVERALL_RUNTIME` (60sec) as the second argument to do the sampling in this period of time.

### Execute Function

The `execute()` function runs a callback function consecutively in a period of time. The first argument is the callback function and the second argument is duration in seconds.

```
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
```

The `clock_gettime()` gets the current time of the clock specified by clock_id `CLOCK_MONOTONIC`, and puts it into the `start` buffer.

In addition, the `getElapsed()` is applied to get the number of passed seconds (double) from now. `CLOCK_MONOTONIC` clock represents monotonic time since some unspecified starting point. The `start` and `now` buffers points to a structure containing the number of seconds and nano seconds that helps to calculate the number of passed seconds (double) from now.

```
double getElapsed(struct timespec time)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    return (double)(now.tv_sec + now.tv_nsec / 1.0e9) - (double)(time.tv_sec + time.tv_nsec / 1.0e9);
}
```

### Sampling Function
The `sampling()` is a callback function that is executed within overall runtime. This function does sampling and counts the total number of samples if the current rank is 0 `MASTER`.

```
/**
 * Do sampling at a particular time
 * @param instant
 */
void sampling(double *instant)
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == MASTER)
    {
        samples++;
    }
    execute(locating, TIME_SAMPLING_INTERVAL);
}
```

Within sampling, the `execute()` function accepts locating as the first argument and `TIME_SAMPLING_INTERVAL` (0.01sec) as the second argument to do the locating in this period.

```
execute(locating, TIME_SAMPLING_INTERVAL);
```

### Locating Function
The `locating()` is a callback function that is executed within a sampling interval. This function does locate vessels in 1000 locations `MAXIMUM_LOCATIONS` and counts the total number of possible strikes considering strike rules.

`MPI_Send` performs a blocking send. All the non-master ranks generate random location number and send it to the master rank.

```
if (rank != MASTER)
{
    location = getRandom();
    MPI_Send(&location, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
}
```

On the other hand, `MPI_Recv` is blocking receive for a message in this case vessel location. The master rank receives the locations of vessels from other ranks and counts total strikes for that particular location. An struct is used to keep track of total strikes, number of odd and even vessels for each location.

```
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
```

The first argument is the data buffer. The second and third arguments describe the count and type of elements that reside in the buffer. `MPI_Send` sends the location, and `MPI_Recv` will receive the location. The fourth and fifth arguments specify the rank of the sending/receiving process and the tag of the message. The sixth argument specifies the communicator and the last argument (for `MPI_Recv` only) provides information about the received message. The program detects the possible strike at the end of a cycle and prints it out.

## 2. Inter-process Communication Scheme (IPC)
The solution utilises message passing approach and shares information among number of processes. The communication is synchronous. In other words, synchronization semantics is blocking send and receive. All non-master processes sends their own randomised location to the master process. On the other hand, the master process receives the location of each vessel and calculate the number of strikes considering the share of one odd numbered and two even numbered vessels at the same location.
```
    Sender      Receiver
 (Non-master)   (Master)
      |            |
      |            |
    1 |\           |
         \               1: All Non-master ranks sends the vessel location.
           \             2: Master rank receives all the locations from
             \              each rank and perform counting.
               \
                 \>| 2
      |            |
      |            |
```

## 3. Performance Metrics

This solution provides a shell script. The shell script `fleet.sh` is run to repeat the program three times in order to estimate the average strikes number and other metrics. This will run `16` copies of fleet program in current run-time environment. As a result, three output files will be generated in which includes the metrics.

The average strikes number is `573`, the rate of strikes over `5715` samples is roughly `0.1`.

### Attempt 1
```
650 strikes detected from 5714 samples.
Rate of strikes over samples is 0.113756.
1000 locations in 60sec overall runtime and 0.010sec sampling interval.
```

### Attempt 2
```
503 strikes detected from 5683 samples.
Rate of strikes over samples is 0.088510.
1000 locations in 60sec overall runtime and 0.010sec sampling interval.
```

### Attempt 3
```
568 strikes detected from 5750 samples.
Rate of strikes over samples is 0.098783.
1000 locations in 60sec overall runtime and 0.010sec sampling interval.
```
