#!/bin/bash

# ensure a clean build
make clean

# gcc
printf "gcc make\n\n"
make TYPE=test -j8
printf "\n\ngcc test\n\n"
make TYPE=test test -j8

# run
bash test/run.sh

make coverage
