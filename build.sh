#!/bin/bash

# ensure a clean build
make clean

# gcc
printf "gcc make\n\n"
make -j8
printf "\n\ngcc test\n\n"
make test -j8
