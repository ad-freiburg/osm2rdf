package main
// Copyright 2020, University of Freiburg
// Authors: Axel Lehmann <lehmann@cs.uni-freiburg.de>.

// This file is part of osm2ttl.
//
// osm2ttl is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// osm2ttl is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with osm2ttl.  If not, see <https://www.gnu.org/licenses/>.

import (
	"bufio"
	"compress/bzip2"
	"container/heap"
	"encoding/json"
	"fmt"
	"os"
	"sort"
	"strconv"

	"golang.org/x/text/language"
	"golang.org/x/text/message"
)

type OsmObject struct {
	t string
	i int
}

func NewOsmObject(i int, t string) *OsmObject {
	o := &OsmObject{
		i: i,
		t: t,
	}
	if t == "area" {
		if o.i%2 == 0 {
			o.t = "way"
			o.i = o.i / 2
		} else {
			o.t = "relation"
			o.i = (o.i - 1) / 2
		}
	}
	return o
}

func (oo OsmObject) String() string {
	return fmt.Sprintf("https://www.openstreetmap.org/%s/%d", oo.t, oo.i)
}

type lineJSON struct {
	Function  string `json:"function"`
	Part      string `json:"part"`
	Check     string `json:"check"`
	OuterId   int    `json:"outer_id"`
	OuterType string `json:"outer_type"`
	InnerId   int    `json:"inner_id"`
	InnerType string `json:"inner_type"`
	Duration  int    `json:"duration_ns"`
	Result    bool   `json:"result"`
}

type Line struct {
	function string
	part     string
	check    string
	Outer    *OsmObject
	Inner    *OsmObject
	Duration int
	Result   bool
}

func NewLine(data lineJSON) *Line {
	l := &Line{
		function: data.Function,
		part:     data.Part,
		check:    data.Check,
		Inner:    NewOsmObject(data.InnerId, data.InnerType),
		Outer:    NewOsmObject(data.OuterId, data.OuterType),
		Duration: data.Duration,
		Result:   data.Result,
	}
	return l
}

type LineHeap []*Line

func (h LineHeap) Len() int {
	return len(h)
}
func (h LineHeap) Less(i, j int) bool {
	return h[i].Duration < h[j].Duration
}
func (h LineHeap) Swap(i, j int) {
	h[i], h[j] = h[j], h[i]
}
func (h *LineHeap) Push(x interface{}) {
	*h = append(*h, x.(*Line))
}
func (h *LineHeap) Pop() interface{} {
	tmp := *h
	n := len(tmp)
	x := tmp[n-1]
	*h = tmp[0 : n-1]
	return x
}

type Counter struct {
	Time  int
	Count int
}

type InfoCounter struct {
	Positive Counter
	Negative Counter
}

func (i InfoCounter) TotalTime() int {
	return i.Positive.Time + i.Negative.Time
}

func (i InfoCounter) TotalCount() int {
	return i.Positive.Count + i.Negative.Count
}

type Info struct {
	Inner InfoCounter
	Outer InfoCounter
}

func (i Info) TotalTime() int {
	return i.Inner.TotalTime() + i.Outer.TotalTime()
}

func (i Info) TotalCount() int {
	return i.Inner.TotalCount() + i.Outer.TotalCount()
}

func (i Info) TotalPositiveResults() int {
	return i.Inner.Positive.Count + i.Outer.Positive.Count
}

func (i Info) TotalNegativeResults() int {
	return i.Inner.Negative.Count + i.Outer.Negative.Count
}

type Pair struct {
	First  Info
	Second string
}

func main() {
	if len(os.Args) < 2 {
		panic("Missing Filename")
	}
	n := 10
	if len(os.Args) > 2 {
		var err error
		n, err = strconv.Atoi(os.Args[2])
		if err != nil {
			n = 10
		}
	}
	p := message.NewPrinter(language.Make("en"))
	if len(os.Args) > 3 {
		p = message.NewPrinter(language.Make(os.Args[3]))
	}
	f, err := os.OpenFile(os.Args[1], 0, 0)
	if err != nil {
		panic(err)
	}
	defer f.Close()
	br := bufio.NewReader(f)
	cr := bzip2.NewReader(br)

	d := bufio.NewReader(cr)
	s := bufio.NewScanner(d)

	h := &LineHeap{}
	heap.Init(h)

	info := make(map[string]Info)

	var i int = 0
	for s.Scan() {
		line := s.Bytes()
		if len(line) < 4 {
			continue
		}
		data := lineJSON{}
		if err := json.Unmarshal(line[:len(line)-1], &data); err != nil {
			p.Printf("ERROR: %#v\n", err)
			p.Printf("       %#v\n", line)
		}
		lineObj := NewLine(data)
		iKey := lineObj.Inner.String()
		oKey := lineObj.Outer.String()
		if _, ok := info[iKey]; !ok {
			info[iKey] = Info{}
		}
		if _, ok := info[oKey]; !ok {
			info[oKey] = Info{}
		}
		iObj, _ := info[iKey]
		oObj, _ := info[oKey]
		if lineObj.Result {
			iObj.Inner.Positive.Count++
			iObj.Inner.Positive.Time += lineObj.Duration
			oObj.Outer.Positive.Count++
			oObj.Outer.Positive.Time += lineObj.Duration
		} else {
			iObj.Inner.Negative.Count++
			iObj.Inner.Negative.Time += lineObj.Duration
			oObj.Outer.Negative.Count++
			oObj.Outer.Negative.Time += lineObj.Duration
		}
		info[iKey] = iObj
		info[oKey] = oObj
		if len(*h) < n {
			heap.Push(h, lineObj)
		} else {
			if lineObj.Duration > (*h)[0].Duration {
				heap.Pop(h)
				heap.Push(h, lineObj)
			}
		}
		i++
		if i%25000 == 0 {
			p.Printf(".")
		}
		if i%1000000 == 0 {
			p.Printf(" read %3dM lines\n", (i / 1000000))
		}
	}
	p.Printf(" read %12d lines\n", i)

	// Slowest checks
	var lines []*Line
	for len(*h) > 0 {
		lines = append(lines, heap.Pop(h).(*Line))
	}
	sort.Slice(lines, func(i, j int) bool {
		return lines[i].Duration > lines[j].Duration
	})
	p.Printf("\n%d slowest checks:\n", n)
	for i = 0; i < len(lines); i++ {
		l := lines[i]
		p.Printf("[%s:%s:%s] %s -> %s : %dns\n", l.function, l.part, l.check, l.Inner, l.Outer, l.Duration)
	}

	var pairs []Pair
	for k, v := range info {
		pairs = append(pairs, Pair{v, k})
	}

	// Most checked as outer
	sort.Slice(pairs, func(i, j int) bool {
		return pairs[i].First.Outer.TotalCount() > pairs[j].First.Outer.TotalCount()
	})
	p.Printf("\n%d most checks against:\n", n)
	for i = 0; i < len(pairs); i++ {
		if i >= n {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.Outer.TotalCount())
	}

	// Most time as outer
	sort.Slice(pairs, func(i, j int) bool {
		return pairs[i].First.Outer.TotalTime() > pairs[j].First.Outer.TotalTime()
	})
	p.Printf("\n%d most time against:\n", n)
	for i = 0; i < len(pairs); i++ {
		if i >= n {
			break
		}
		p.Printf("%-50s : %20dns\n", pairs[i].Second, pairs[i].First.Outer.TotalTime())
	}

	// Most positive result as outer
	sort.Slice(pairs, func(i, j int) bool {
		return pairs[i].First.Outer.Positive.Count > pairs[j].First.Outer.Positive.Count
	})
	p.Printf("\n%d most positive results against:\n", n)
	for i = 0; i < len(pairs); i++ {
		if i >= n {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.Outer.Positive.Count)
	}
	p.Printf("\n%d fewest positive results against:\n", n)
	for i = len(pairs) - 1; i >= 0; i-- {
		if i <= len(pairs)-n-1 {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.Outer.Positive.Count)
	}

	// Most negative result as outer
	sort.Slice(pairs, func(i, j int) bool {
		return pairs[i].First.Outer.Negative.Count > pairs[j].First.Outer.Negative.Count
	})
	p.Printf("\n%d most negative results against:\n", n)
	for i = 0; i < len(pairs); i++ {
		if i >= n {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.Outer.Negative.Count)
	}
	p.Printf("\n%d fewest negative results against:\n", n)
	for i = len(pairs) - 1; i >= 0; i-- {
		if i <= len(pairs)-n-1 {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.Outer.Negative.Count)
	}

	// Most checked as inner
	sort.Slice(pairs, func(i, j int) bool {
		return pairs[i].First.Inner.TotalCount() > pairs[j].First.Inner.TotalCount()
	})
	p.Printf("\n%d most checks triggered:\n", n)
	for i = 0; i < len(pairs); i++ {
		if i >= n {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.Inner.TotalCount())
	}

	// Most time as inner
	sort.Slice(pairs, func(i, j int) bool {
		return pairs[i].First.Inner.TotalTime() > pairs[j].First.Inner.TotalTime()
	})
	p.Printf("\n%d most time triggered:\n", n)
	for i = 0; i < len(pairs); i++ {
		if i >= n {
			break
		}
		p.Printf("%-50s : %20dns\n", pairs[i].Second, pairs[i].First.Inner.TotalTime())
	}

	// Most positive result as inner
	sort.Slice(pairs, func(i, j int) bool {
		return pairs[i].First.Inner.Positive.Count > pairs[j].First.Inner.Positive.Count
	})
	p.Printf("\n%d most positive results triggered:\n", n)
	for i = 0; i < len(pairs); i++ {
		if i >= n {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.Inner.Positive.Count)
	}
	p.Printf("\n%d fewest postive results triggered:\n", n)
	for i = len(pairs) - 1; i >= 0; i-- {
		if i <= len(pairs)-n-1 {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.Inner.Positive.Count)
	}

	// Most positive result as inner
	sort.Slice(pairs, func(i, j int) bool {
		return pairs[i].First.Inner.Negative.Count > pairs[j].First.Inner.Negative.Count
	})
	p.Printf("\n%d most negative results triggered:\n", n)
	for i = 0; i < len(pairs); i++ {
		if i >= n {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.Inner.Negative.Count)
	}
	p.Printf("\n%d fewest negative results triggered:\n", n)
	for i = len(pairs) - 1; i >= 0; i-- {
		if i <= len(pairs)-n-1 {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.Inner.Negative.Count)
	}

	// Most checked total
	sort.Slice(pairs, func(i, j int) bool {
		return pairs[i].First.TotalCount() > pairs[j].First.TotalCount()
	})
	p.Printf("\n%d most checks total:\n", n)
	for i = 0; i < len(pairs); i++ {
		if i >= n {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.TotalCount())
	}

	// Most time total
	sort.Slice(pairs, func(i, j int) bool {
		return pairs[i].First.TotalTime() > pairs[j].First.TotalTime()
	})
	p.Printf("\n%d most time total:\n", n)
	for i = 0; i < len(pairs); i++ {
		if i >= n {
			break
		}
		p.Printf("%-50s : %20dns\n", pairs[i].Second, pairs[i].First.TotalTime())
	}

	// Most positive result
	sort.Slice(pairs, func(i, j int) bool {
		return pairs[i].First.TotalPositiveResults() > pairs[j].First.TotalPositiveResults()
	})
	p.Printf("\n%d most positive results total:\n", n)
	for i = 0; i < len(pairs); i++ {
		if i >= n {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.TotalPositiveResults())
	}
	p.Printf("\n%d fewest positive results total:\n", n)
	for i = len(pairs) - 1; i >= 0; i-- {
		if i <= len(pairs)-1-n {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.TotalPositiveResults())
	}

	// Most negative result
	sort.Slice(pairs, func(i, j int) bool {
		return pairs[i].First.TotalNegativeResults() > pairs[j].First.TotalNegativeResults()
	})
	p.Printf("\n%d most negative results total:\n", n)
	for i = 0; i < len(pairs); i++ {
		if i >= n {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.TotalNegativeResults())
	}
	p.Printf("\n%d fewest negative results total:\n", n)
	for i = len(pairs) - 1; i >= 0; i-- {
		if i <= len(pairs)-1-n {
			break
		}
		p.Printf("%-50s : %20d\n", pairs[i].Second, pairs[i].First.TotalNegativeResults())
	}
}
