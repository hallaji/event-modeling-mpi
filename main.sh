#!/bin/bash

for index in {1..3}
do
    echo Running "$index"
    mpirun --oversubscribe -np $1 main > main"$index".txt
done
