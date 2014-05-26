package main

import (
	".."
	"fmt"
)

func main() {
	d := sdb.New()
	defer d.Close()

	d.Set("hello", "World")
	out := d.Get("hello")
	fmt.Println ("Hello", out)
}
