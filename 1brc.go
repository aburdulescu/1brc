package main

import (
	"bytes"
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
	"runtime"
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

	numWorkers := runtime.NumCPU()

	q := make(chan []byte, numWorkers)
	defer close(q)

	for i := 0; i < numWorkers; i++ {
		go doWork(q)
	}

	const chunkSize = 32 * 1024 * 1024

	if err := processFile(in, chunkSize); err != nil {
		panic(err)
	}
}

func processFile(in io.Reader, chunkSize int) error {
	stats := NewStatsMap()

	buf := make([]byte, chunkSize)
	off := 0
	for {
		n, err := in.Read(buf[off:])
		if errors.Is(err, io.EOF) {
			break
		}
		if err != nil {
			panic(err)
		}

		nn := n + off

		nl := bytes.LastIndexByte(buf[:nn], '\n')

		for chunk := buf[:nl+1]; len(chunk) != 0; {
			chunk = parseLine(stats, chunk)
		}

		copy(buf, buf[nl+1:nn])
		off = nn - nl - 1
	}

	stats.print()

	return nil
}

func doWork(q chan []byte) {
	for {
		chunk, ok := <-q
		if !ok {
			return
		}
	}
}

const (
	fnv1aOffset32 = uint32(2166136261)
	fnv1aPrime32  = uint32(16777619)
)

func parseLine(stats *StatsMap, line []byte) []byte {
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
	line = line[sep+1:]

	nl := bytes.IndexByte(line, '\n')

	temp := parseTemp(line[:nl])
	line = line[nl+1:]

	stats.add(city, cityHash, temp)

	return line
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

func parseTemp(temp []byte) int16 {
	result := int16(0)
	for _, b := range temp {
		result *= tempTable[b].mul
		result += tempTable[b].val
	}
	if temp[0] == '-' {
		result *= -1
	}
	return result
}

type Stats struct {
	city []byte
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

func NewStatsMap() *StatsMap {
	return &StatsMap{
		list: make([]*Stats, 0, MAX_CITIES),
	}
}

func (sm *StatsMap) add(city []byte, cityHash uint32, temp int16) {
	slot := cityHash & (MAX_CITIES - 1)
	for ; slot < MAX_CITIES; slot++ {
		e := &sm.entries[slot]
		if len(e.city) == 0 {
			break
		}
		if bytes.Equal(e.city, city) {
			break
		}
	}
	if slot >= MAX_CITIES {
		panic("slot >= MAX_CITIES")
	}

	e := &sm.entries[slot]
	if len(e.city) == 0 {
		// if slot is empty, add city and update list
		e.city = make([]byte, len(city))
		copy(e.city, city)
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
		return bytes.Compare(sm.list[i].city, sm.list[j].city) < 0
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
