#!/bin/bash

./test/bin/model || { exit 1; }

./test/bin/select || { exit 1; }
