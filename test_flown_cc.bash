#!/bin/bash
#I think I made this script sh-compatible, because why not.
set -euo pipefail #bash strict mode
cc -O3 flown.c -o flown # most of the time is taken up by compiling anyway, so -O2 quadruples the test script time, but it's important not to be blindsided by possible compiler optimizations

test_string="
test
run"
echo Test string: "$test_string"
echo Testing flown commands:
echo "$test_string" | ./flown cat.fln
echo "$test_string" | ./flown rev.fln
echo "$test_string" | ./flown tac.fln
echo "$test_string" | ./flown fg.fln
echo "$test_string" | ./flown evenodd.fln
echo "1" | ./flown 124.fln

echo "In lieu of a big diff, visually compare that output to the expected:"
echo "
test
run

nur
tset
run
test

1"

rm flown

<<<HEREDOC #this got too complex

echo Testing flown commands.
test_string = `printf "\ntest\nrun\n"`
echo Test string: "$test_string"

echo Testing cat.fln.
expect_cat="
test
run
"
real_cat=`echo "$test_string" | ./flown cat.fln <test_string.txt`
if [  "$real_cat" == "$expect_cat" ] ; then echo OK. ; 

HEREDOC
