cc flown.c -o flown
echo 1001010001 | ./flown 124.fln 2>&1 >> output.txt & sleep 1
rm flown
