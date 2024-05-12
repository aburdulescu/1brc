package main

import (
	"bufio"
	"flag"
	"fmt"
	"hash/fnv"
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

	stats := StatsMapNew()

	for s.Scan() {
		parseLine(stats, s.Text())
	}
	if err := s.Err(); err != nil {
		panic(err)
	}

	stats.print()
}

func parseLine(stats *StatsMap, line string) {
	sep := strings.IndexByte(line, ';')

	city := line[:sep]

	h := fnv.New32a()
	h.Write([]byte(city))
	cityHash := h.Sum32()

	temp := parseTemp(line[sep+1:])

	stats.add(city, cityHash, temp)
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
