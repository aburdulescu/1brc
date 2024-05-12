package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"
	"runtime/pprof"
	"sort"
	"strings"
)

func main() {

	cpuProfile := flag.Bool("cpuprofile", false, "Write CPU profile")
	memProfile := flag.Bool("memprofile", false, "Write memory profile")

	flag.Parse()

	if *cpuProfile {
		f, err := os.Create("cpu.profile")
		if err != nil {
			panic(err)
		}
		pprof.StartCPUProfile(f)
		defer pprof.StopCPUProfile()
	}
	defer func() {
		if *memProfile {
			f, err := os.Create("mem.profile")
			if err != nil {
				fmt.Fprintln(os.Stderr, "error:", err)
			}
			pprof.WriteHeapProfile(f)
			f.Close()
		}
	}()

	in, err := os.Open(flag.Arg(0))
	if err != nil {
		panic(err)
	}
	defer in.Close()

	s := bufio.NewScanner(in)

	stats := map[string]*Stats{}

	for s.Scan() {
		if err := parseLine(stats, s.Text()); err != nil {
			panic(err)
		}
	}

	if err := s.Err(); err != nil {
		panic(err)
	}

	cities := make([]string, 0, len(stats))
	for city := range stats {
		cities = append(cities, city)
	}
	sort.Strings(cities)

	for _, city := range cities {
		s := stats[city]
		fmt.Printf(
			"%s=%.1f/%.1f/%.1f\n",
			city,
			float32(s.min)/10,
			float32(s.sum)/float32(s.cnt)/10,
			float32(s.max)/10,
		)
	}
}

func parseLine(stats map[string]*Stats, line string) error {
	sep := strings.IndexByte(line, ';')

	city := line[:sep]
	temp := parseTemp(line[sep+1:])

	s, ok := stats[city]
	if ok {
		s.sum += int64(temp)
		s.cnt++
		if temp > s.max {
			s.max = temp
		}
		if temp < s.min {
			s.min = temp
		}
	} else {
		stats[city] = &Stats{
			min: temp,
			max: temp,
			sum: int64(temp),
			cnt: 1,
		}
	}

	return nil
}

func parseTemp(temp string) int16 {
	i := 0
	isNeg := false
	if temp[i] == '-' {
		isNeg = true
		i++
	}

	var result int16

	result = int16(temp[i] - '0')
	i++

	if temp[i] != '.' {
		result = result*10 + int16(temp[i]-'0')
		i++
	}

	i++

	result = result*10 + int16(temp[i]-'0')

	if isNeg {
		return result * (-1)
	}

	return result
}

type Stats struct {
	sum int64
	cnt int64
	min int16
	max int16
}
