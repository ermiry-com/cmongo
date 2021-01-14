#!/bin/bash

LD_LIBRARY_PATH=bin ./test/bin/select || { exit 1; }
