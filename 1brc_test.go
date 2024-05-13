package main

import (
	"bytes"
	"testing"
)

func TestProcessFile(t *testing.T) {
	data := bytes.NewReader([]byte(`aaaa;12.3
bbbb;23.4
bbbb;33.4
`))

	t.Log(processFile(data, 12))
}
