release:
	gcc -Wall -Wextra -Werror -g0 -O3 -march=native 1brc.c
	/usr/bin/time -f "%e sec, %M kbytes" ./a.out

debug: build
	./a.out

debug10: build
	./a.out measurements10.txt

test: build
	./a.out -t

build:
	gcc -Wall -Wextra -Werror -g -fsanitize=address,undefined 1brc.c
