package sdb

// #cgo LDFLAGS: ../../src/libsdb.a
// #include <sdb/sdb.h>
import "C"

/*
import (
	"io"
	"unsafe"
)
*/

type Sdb struct {
	ptr *C.Sdb;
}

func New() *Sdb {
	d := new(Sdb)
	d.ptr = C.sdb_new(nil, nil, 0)
	return d;
}

func (d *Sdb) Close() {
	C.sdb_free (d.ptr)
	d.ptr = nil
}

func (d *Sdb) Get (k string) string {
	v := C.sdb_get(d.ptr, C.CString(k), nil)
	return C.GoString (v)
}

func (d *Sdb) Set (k, v string) {
	C.sdb_set(d.ptr, C.CString(k), C.CString(v), 0)
}
