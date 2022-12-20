echo ^

test^

run>test_string.txt
tcc -run flown.c cat.fln <test_string.txt
tcc -run flown.c rev.fln <test_string.txt
tcc -run flown.c tac.fln <test_string.txt
tcc -run flown.c fg.fln <test_string.txt
tcc -run flown.c evenodd.fln <test_string.txt
echo 1 | tcc -run flown.c 124.fln
del test_string.txt
