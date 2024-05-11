package main

import (
	"bufio"
	"fmt"
	"os"
	"sort"
	"strings"
)

func main() {
	in, err := os.Open(os.Args[1])
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
	parts := strings.Split(line, ";")

	city := parts[0]
	temp := parseTemp(parts[1])

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
	min int16
	max int16
	sum int64
	cnt int64
}
