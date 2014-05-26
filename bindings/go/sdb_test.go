package sdb_test

import (
	"."
	"fmt"
)

func ExampleSdb() {
	db := sdb.New()
	defer db.Close()

	db.Set("jiji","jojo")
	fmt.Println ("Hello SDB %s", db.Get("jiji"))
}
