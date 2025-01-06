#!/bin/bash

# clean up
make clean

# compile testque
make testque

clear

# ask if they want to run testque and put in variable
read -p "Do you want to run testque? (y/N): " run_testque

# make run_testque variable lowercase
run_testque=$(echo $run_testque | tr '[:upper:]' '[:lower:]')

# run testque
if [ "$run_testque" == "y" ]; then
    ./testque
fi


# ask if they want to run testatom and put in variable
read -p "Do you want to run testatom? (y/N): " run_testatom

# make run_testatom variable lowercase
run_testatom=$(echo $run_testatom | tr '[:upper:]' '[:lower:]')


# compile testatom
make testatom

# clear screen
clear

# run testatom
if [ "$run_testatom" == "y" ]; then
    ./testatom
fi

# ask if they want to run tspcc and put in variable
read -p "Do you want to run tspcc (V1)? (y/N): " run_tspcc

# to lowecase
run_tspcc=$(echo $run_tspcc | tr '[:upper:]' '[:lower:]')

# compile tspcc
make tspcc

# run tspcc
if [ "$run_tspcc" == "y" ]; then
    read -p "Number of threads? (10): " number_threads
    number_threads=$(echo "$number_threads" | tr '[:upper:]' '[:lower:]')
    number_threads=${number_threads:-10}
    clear
    echo "Running tspcc with $number_threads threads..."
    ./tspcc dj38.tsp $number_threads
fi

# ask if they want to run tspcc and put in variable
read -p "Do you want to run tspcc (V2)? (y/N): " run_tspcc2

# to lowecase
run_tspcc2=$(echo $run_tspcc2 | tr '[:upper:]' '[:lower:]')

# compile tspcc
make clean
make tspcc CPPFLAGS=-DADVANCE_END_CONDITION

# run tspcc
if [ "$run_tspcc2" == "y" ]; then
    read -p "Number of threads? (10): " number_threads
    number_threads=$(echo "$number_threads" | tr '[:upper:]' '[:lower:]')
    number_threads=${number_threads:-10}
    clear
    echo "Running tspcc with $number_threads threads..."
    ./tspcc dj38.tsp $number_threads
fi