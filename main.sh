#!/bin/bash

if [ "$#" -ne 1 ]
then
    echo "Error: Plesse specify number of processes as the only argument."
    echo "\$ $0 4"
    exit 1
elif [ $1 -le 3 2>/dev/null ] ;
then
    echo "Error: The number of processes must be equal to or greater than 4."
    echo "\$ $0 4"
    exit 1
fi

echo "The program will be repeated at least three times to estimate the average launch number and other metrics."
for index in {1..3}
do
    output="output$index.txt"
    echo "Started locating vessels ($index) > $output"
    mpirun --oversubscribe -np $1 main > $output
done

echo "Done!"
exit 0
