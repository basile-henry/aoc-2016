CFLAGS = -g -O3 -std=c99 \
	-Wno-gnu-statement-expression \
	-Wno-gnu-auto-type \
	-Wimplicit-fallthrough \
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

day13: src/day13.c
	$(CC) $(CFLAGS) src/day13.c -o day13

day15: src/day15.c
	$(CC) $(CFLAGS) src/day15.c -o day15

day16: src/day16.c
	$(CC) $(CFLAGS) src/day16.c -o day16

test: src/test.c
	$(CC) $(CFLAGS) src/test.c -o test

.PHONY: all
all: test day04 day06 day07 day08 day09 day10 day11 day12 day13 day15 day16

.PHONY: clean
clean:
	rm -f ./test ./day04 ./day06 ./day07 ./day08 ./day09 ./day10 ./day11 ./day12 ./day13 ./day15 ./day16
