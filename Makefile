CFLAGS = -g -O3 -std=c99 \
	-Wno-gnu-statement-expression \
	-Wno-gnu-auto-type \
	-pedantic -Wall -Wextra -Wconversion -Werror

day04: src/day04.c
	$(CC) $(CFLAGS) src/day04.c -o day04

day06: src/day06.c
	$(CC) $(CFLAGS) src/day06.c -o day06

day07: src/day07.c
	$(CC) $(CFLAGS) src/day07.c -o day07

day08: src/day08.c
	$(CC) $(CFLAGS) src/day08.c -o day08

day09: src/day09.c
	$(CC) $(CFLAGS) src/day09.c -o day09

day10: src/day10.c
	$(CC) $(CFLAGS) src/day10.c -o day10

day11: src/day11.c
	$(CC) $(CFLAGS) src/day11.c -o day11

day12: src/day12.c
	$(CC) $(CFLAGS) src/day12.c -o day12

test: src/test.c
	$(CC) $(CFLAGS) src/test.c -o test

.PHONY: all
all: test day04 day06 day07 day08 day09 day10 day11 day12

.PHONY: clean
clean:
	rm -f ./test ./day04 ./day06 ./day07 ./day08 ./day09 ./day10 ./day11 ./day12
