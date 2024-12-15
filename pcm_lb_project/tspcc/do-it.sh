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
read -p "Do you want to run tspcc? (y/N): " run_tspcc

# to lowecase
run_tspcc=$(echo $run_tspcc | tr '[:upper:]' '[:lower:]')

# compile tspcc
make tspcc

# clear screen
clear

# run tspcc
if [ "$run_tspcc" == "y" ]; then
    ./tspcc dj38.tsp
fi