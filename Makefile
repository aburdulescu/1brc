release:
	gcc -Wall -Wextra -Werror -g0 -O3 -march=native 1brc.c
	/usr/bin/time -f "%e seconds, %M kbytes" ./a.out

debug: build-debug
	./a.out

debug10: build-debug
	./a.out measurements10.txt

test: build-debug
	./a.out -t

build-debug:
	gcc -Wall -Wextra -Werror -g -fsanitize=address,undefined 1brc.c

build-prof:
	gcc -Wall -Wextra -Werror -g -O3 -march=native -fno-omit-frame-pointer 1brc.c
