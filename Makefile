CFLAGS = -Wall -Wextra -Werror -std=c11 -fno-omit-frame-pointer -g

release: build-release
	/usr/bin/time -f "%e seconds, %M kbytes" ./a.out

debug: build-asan
	./a.out 1brc/measurements_small.txt

test: build-asan
	./a.out test

build-asan:
	gcc $(CFLAGS) -fsanitize=address,undefined 1brc.c

build-release:
	gcc $(CFLAGS) -O3 -march=native 1brc.c

build-debug:
	gcc $(CFLAGS) -O0 -march=native 1brc.c

profile: build-debug
	LD_PRELOAD=$(shell pwd)/gperftool-dist/lib/libprofiler.so CPUPROFILE=cpu.prof ./a.out
	pprof -http=: cpu.prof
