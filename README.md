# Distributed Event Modelling with MPI
This program provides a simple solution for the naval fleet problem. The assumption is that a naval fleet patrols an area comprising `1000` distinct locations and each vessel can occupy any one of these locations randomly at the end of sampling interval. However, for a vessel to be able to launch a strike, other vessels must accompany it.

## Dependencies
* open-mpi
* gcc

## Compilation and Run
```
$ make fleet

# sh fleet.sh < # of processes >
# e.g.
$ sh fleet.sh 16
```
The `make` command compiles fleet MPI program and causes linking with the output executable `fleet` using `mpicc` command.

The shell script `fleet.sh` is used to repeat the program three times in order to estimate the average strikes number and other metrics. This will run `X` copies of fleet program in your current run-time environment. As a result, three output files will be generated in which includes the metrics.

`mpicc` is a convenience wrapper for the underlying C compiler. It compiles and links MPI programs written in C. `mpirun` is a shell script that attempts to hide the differences in starting jobs for various devices from the user. Mpirun attempts to determine what kind of machine it is running on and start the required number of jobs on that machine.

## Compile and Run Manually
`--oversubscribe` option may need if the number of slots is limited in your machine.
```
$ mpicc -o fleet fleet.c
$ mpirun --oversubscribe -np 16 fleet
```

## Ruls and Assumptions

1. At least one odd numbered and two even numbered vessels share the same location, at a given point in time, for a strike to be counted. MPI rank `1` to `n` may be used to number each vessel in the fleet.
2. The fleet may generate more than one strike at an instant of time (it will however depend on the number of locations meeting the strike criterion, item 1 above, at that instant of time).
3. There is no limit on the number of vessels in the fleet. The objective is to achieve the highest possible strike rate. It may however be pointed out that increases in fleet size will increase the probability of satisfying Rule no. 1 (above), but doing so will also slow the program owing to higher inter-process communication overheads.
4. The vessel movement is instantaneous and can be to a non-contagious location. In other words there is no constraint as to how a vessel moves from one location to another (except for the location being randomly assigned)
5. Sampling interval is a small fraction of 60 seconds i.e. the overall runtime period stipulated for calculating the total number of strikes.
