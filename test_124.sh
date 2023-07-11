#echo adds a newline to the end, implicitly
cc flown.c -o flown && echo 1010101001 | ./flown 124.fln 2>&1 | less
rm flown
