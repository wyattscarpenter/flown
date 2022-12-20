cl -O2 flown.c
echo ^

test^

run>test_string.txt
flown cat.fln <test_string.txt
flown rev.fln <test_string.txt
flown tac.fln <test_string.txt
flown fg.fln <test_string.txt
flown evenodd.fln <test_string.txt
echo 1 | flown 124.fln
del test_string.txt
del flown.exe
del flown.obj
