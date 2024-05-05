CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -fno-omit-frame-pointer -g

release: build-release
	/usr/bin/time -f "%e seconds, %M kbytes" ./a.out

debug: build-asan
	ASAN_OPTIONS=detect_leaks=0 ./a.out 1brc/measurements_small.txt

test: build-asan
	ASAN_OPTIONS=detect_leaks=0 ./a.out test

build-asan:
	$(CC) $(CFLAGS) -fsanitize=address,undefined 1brc.c

build-release:
	$(CC) $(CFLAGS) -O3 -march=native 1brc.c -static

build-debug:
	$(CC) $(CFLAGS) -O0 -march=native 1brc.c -static

profile: build-debug
	LD_PRELOAD=$(shell pwd)/gperftool-dist/lib/libprofiler.so CPUPROFILE=cpu.prof ./a.out
	pprof -http=: cpu.prof
