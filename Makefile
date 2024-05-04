CFLAGS = -Wall -Wextra -Werror -std=c11

release:
	gcc $(CFLAGS) -g0 -O3 -march=native 1brc.c
	/usr/bin/time -f "%e seconds, %M kbytes" ./a.out

debug: build-debug
	./a.out measurements_small.txt

test: build-debug
	./a.out -t

build-debug:
	gcc $(CFLAGS) -g -fsanitize=address,undefined 1brc.c

build-prof:
	gcc $(CFLAGS) -g -O3 -march=native 1brc.c

build-prof-debug:
	gcc $(CFLAGS) -g -O0 -march=native 1brc.c
