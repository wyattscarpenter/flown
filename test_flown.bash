#!/bin/bash
set -euo pipefail #bash strict mode
cc -O2 flown.c -o flown # most of the time is taken up by compiling anyway, so -O2 quadruples the test script time, but it's important not to be blindsided by possible compiler optimizations
printf "\ntest\nrun\n" >test_string.txt
./flown cat.fln <test_string.txt
./flown rev.fln <test_string.txt
./flown tac.fln <test_string.txt
./flown fg.fln <test_string.txt
./flown evenodd.fln <test_string.txt
./flown 124.fln <<<"1"
