CFLAGS = -Wall -Wextra -Werror -std=c11 -fno-omit-frame-pointer -g

release:
	gcc $(CFLAGS) -O3 -march=native 1brc.c
	/usr/bin/time -f "%e seconds, %M kbytes" ./a.out

debug: build-debug
	./a.out measurements_small.txt

debug-tsan: build-debug-tsan
	./a.out measurements_small.txt

test: build-debug
	./a.out -t

build-debug:
	gcc $(CFLAGS) -fsanitize=address,undefined 1brc.c

build-debug-tsan:
	gcc $(CFLAGS) -fsanitize=thread,undefined 1brc.c

build-prof:
	gcc $(CFLAGS) -O3 -march=native 1brc.c

build-prof-debug:
	gcc $(CFLAGS) -O0 -march=native 1brc.c
