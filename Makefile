GCOV_OUTPUT = *.gcda *.gcno *.gcov 
GCOV_CCFLAGS = -fprofile-arcs -ftest-coverage
CC     = gcc
CCFLAGS = -g -I. -Itests -Wall -Werror -W -fno-omit-frame-pointer -fno-common -fsigned-char $(GCOV_CCFLAGS)


all: test

main.c:
	sh tests/make-tests.sh tests/test_*.c > main.c

test: main.c sparsefile_allocator.o tests/test_sparsefile_allocator.c CuTest.o main.c
	$(CC) $(CCFLAGS) -o $@ $^
	./test
	gcov main.c tests/test_sparsefile_allocator.c sparsefile_allocator.c

sparsefile_allocator.o: sparsefile_allocator.c
	$(CC) $(CCFLAGS) -c -o $@ $^

CuTest.o: tests/CuTest.c
	$(CC) $(CCFLAGS) -c -o $@ $^

clean:
	rm -f main.c sparsefile_allocator.o test $(GCOV_OUTPUT)
