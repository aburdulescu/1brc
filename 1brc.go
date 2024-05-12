package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"
	"runtime/pprof"
	"sort"
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

	stats := StatsMapNew()

	for s.Scan() {
		parseLine(stats, s.Text())
	}
	if err := s.Err(); err != nil {
		panic(err)
	}

	stats.print()
}

const (
	fnv1aOffset32 = uint32(2166136261)
	fnv1aPrime32  = uint32(16777619)
)

func parseLine(stats *StatsMap, line string) {
	sep := -1
	cityHash := fnv1aOffset32

	for i := range line {
		if line[i] == ';' {
			sep = i
			break
		}
		cityHash ^= uint32(line[i])
		cityHash *= fnv1aPrime32
	}

	city := line[:sep]

	temp := parseTemp(line[sep+1:])

	stats.add(city, cityHash, temp)
}

var tempTable = [256]struct {
	mul int16
	val int16
}{
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{10, 0}, {10, 1}, {10, 2}, {10, 3}, {10, 4}, {10, 5}, {10, 6}, {10, 7},
	{10, 8}, {10, 9}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
	{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0},
}

func parseTemp(temp string) int16 {
	result := int16(0)
	for i := range temp {
		result *= tempTable[temp[i]].mul
		result += tempTable[temp[i]].val
	}
	if temp[0] == '-' {
		result *= -1
	}
	return result
}

type Stats struct {
	city string
	sum  int64
	cnt  int64
	min  int16
	max  int16
}

// max is 10000 but we need a power of 2
const MAX_CITIES = (1 << 14)

type StatsMap struct {
	entries [MAX_CITIES]Stats
	list    []*Stats
}

func StatsMapNew() *StatsMap {
	return &StatsMap{
		list: make([]*Stats, 0, MAX_CITIES),
	}
}

func (sm *StatsMap) add(city string, cityHash uint32, temp int16) {
	slot := cityHash & (MAX_CITIES - 1)
	for ; slot < MAX_CITIES; slot++ {
		e := &sm.entries[slot]
		if e.city == "" {
			break
		}
		if e.city == city {
			break
		}
	}
	if slot >= MAX_CITIES {
		panic("slot >= MAX_CITIES")
	}

	e := &sm.entries[slot]
	if e.city == "" {
		// if slot is empty, add city and update list
		e.city = city
		sm.list = append(sm.list, e)
	}
	e.sum += int64(temp)
	e.cnt++
	if temp > e.max {
		e.max = temp
	}
	if temp < e.min {
		e.min = temp
	}
}

func (sm StatsMap) print() {
	sort.Slice(sm.list[:], func(i, j int) bool {
		return sm.list[i].city < sm.list[j].city
	})

	for _, s := range sm.list {
		fmt.Printf(
			"%s=%.1f/%.1f/%.1f\n",
			s.city,
			float32(s.min)/10,
			float32(s.sum)/float32(s.cnt)/10,
			float32(s.max)/10,
		)
	}
}
