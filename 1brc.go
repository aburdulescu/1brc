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
	"sync"
)

var (
	cpuProfile = flag.Bool("cpuprofile", false, "Write CPU profile")
	memProfile = flag.Bool("memprofile", false, "Write memory profile")
	chunkSize  = flag.Int("s", 32*1024*1024, "Chunk size")
)

func main() {
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

	inChan := make(chan []byte, numWorkers)
	outChan := make(chan *StatsMap, numWorkers)

	for i := 0; i < numWorkers; i++ {
		go doWork(inChan, outChan)
	}

	processFile(in, inChan)

	close(inChan)

	stats := <-outChan

	for i := 1; i < numWorkers; i++ {
		stats.merge(<-outChan)
	}

	stats.print()
}

var chunkPool = sync.Pool{
	New: func() interface{} {
		return make([]byte, *chunkSize)
	},
}

func processFile(f *os.File, workerChan chan []byte) {
	for {
		buf := chunkPool.Get().([]byte)

		n, err := f.Read(buf)
		if errors.Is(err, io.EOF) {
			return
		}
		if err != nil {
			fmt.Println("file read error:", err)
			return
		}

		nl := bytes.LastIndexByte(buf[:n], '\n')

		workerChan <- buf[:nl+1]

		nleft := int64(nl + 1 - n)

		if _, err := f.Seek(nleft, 1); err != nil {
			fmt.Println("file seek error:", err)
			return
		}
	}
}

func doWork(in chan []byte, out chan *StatsMap) {
	stats := NewStatsMap()
	defer func() {
		out <- stats
	}()

	for {
		chunk, ok := <-in
		if !ok {
			return
		}

		for len(chunk) != 0 {
			chunk = parseLine(stats, chunk)
		}

		chunkPool.Put(chunk[:cap(chunk)])
	}
}

const (
	fnv1aOffset32 = uint32(2166136261)
	fnv1aPrime32  = uint32(16777619)
)

func fnv1a(b []byte) uint32 {
	result := fnv1aOffset32
	for _, v := range b {
		result ^= uint32(v)
		result *= fnv1aPrime32
	}
	return result
}

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

	if sep == -1 {
		panic("line has no separator")
	}

	city := line[:sep]
	line = line[sep+1:]

	nl := bytes.IndexByte(line, '\n')
	if sep == -1 {
		panic("line has no newline")
	}

	temp := parseTemp(line[:nl])
	line = line[nl+1:]

	stats.update(city, cityHash, temp)

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

func (sm *StatsMap) merge(other *StatsMap) {
	for _, e := range other.list {
		sm.add(e, fnv1a(e.city))
	}
}

func (sm *StatsMap) add(stats *Stats, cityHash uint32) {
	slot := cityHash & (MAX_CITIES - 1)
	for ; slot < MAX_CITIES; slot++ {
		e := &sm.entries[slot]
		if len(e.city) == 0 {
			break
		}
		if bytes.Equal(e.city, stats.city) {
			break
		}
	}
	if slot >= MAX_CITIES {
		panic("slot >= MAX_CITIES")
	}

	e := &sm.entries[slot]
	if len(e.city) == 0 {
		// if slot is empty, add city and update list
		e.city = make([]byte, len(stats.city))
		copy(e.city, stats.city)
		sm.list = append(sm.list, e)
	}
	e.sum += stats.sum
	e.cnt += stats.cnt
	if stats.max > e.max {
		e.max = stats.max
	}
	if stats.min < e.min {
		e.min = stats.min
	}
}

func (sm *StatsMap) update(city []byte, cityHash uint32, temp int16) {
	stats := &Stats{
		city: city,
		sum:  int64(temp),
		max:  temp,
		min:  temp,
		cnt:  1,
	}
	sm.add(stats, cityHash)
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
