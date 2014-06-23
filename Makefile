GCOV_OUTPUT = *.gcda *.gcno *.gcov 
GCOV_CCFLAGS = -fprofile-arcs -ftest-coverage
CC     = gcc
CCFLAGS = -g -I. -Wall -Werror -W -fno-omit-frame-pointer -fno-common -fsigned-char $(GCOV_CCFLAGS)

all: sparsefile.o
	$(CC) $(CCFLAGS) -o $@ $^

sparsefile.o: sparsefile.c
	$(CC) $(CCFLAGS) -c -o $@ $^

clean:
	rm -f sparsefile.o
