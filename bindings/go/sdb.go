package sdb

// #cgo LDFLAGS: ../../src/libsdb.a
// #include <sdb/sdb.h>
import "C"

import (
	"unsafe"
)

type Sdb struct {
	ptr *C.Sdb
}

/* lifecycle */

func New() *Sdb {
	d := new(Sdb)
	d.ptr = C.sdb_new(nil, nil, 0)
	return d;
}

func (d *Sdb) Close() {
	C.sdb_free (d.ptr)
	d.ptr = nil
}

func (d *Sdb) Reset() {
	C.sdb_reset (d.ptr)
}

func (d *Sdb) Unlink() {
	C.sdb_unlink (d.ptr)
}

func (d *Sdb) Drain(f *Sdb) {
	C.sdb_drain (d.ptr, f.ptr)
}

/* string */

func (d *Sdb) Exists(k string) bool {
	return C.sdb_exists (d.ptr, C.CString(k)) != 0
}

func (d *Sdb) Get (k string) string {
	v := C.sdb_const_get(d.ptr, C.CString(k), nil)
	return C.GoString (v)
}

func (d *Sdb) Set (k, v string) {
	C.sdb_set(d.ptr, C.CString(k), C.CString(v), 0)
}

func (d *Sdb) Unset (k string) {
	C.sdb_unset (d.ptr, C.CString (k), 0)
}

func (d *Sdb) UnsetMatching (k string) {
	C.sdb_unset_matching (d.ptr, C.CString (k))
}

func (d *Sdb) Concat (k, v string) {
	C.sdb_concat (d.ptr, C.CString(k), C.CString(v), 0)
}

func (d *Sdb) Uncat (k, v string) {
	C.sdb_uncat (d.ptr, C.CString(k), C.CString(v), 0)
}


/* number api */

func (d *Sdb) NumGet (k string) uint64 {
	return uint64(C.sdb_num_get (d.ptr, C.CString (k), nil))
}

func (d *Sdb) NumExists(k string) bool {
	return C.sdb_num_exists (d.ptr, C.CString(k)) != 0
}

func (d *Sdb) NumSet (k string, v uint64) bool {
	return C.sdb_num_set (d.ptr, C.CString (k), C.ulonglong(v), 0) != 0
}

func (d *Sdb) NumInc (k string, n uint64) uint64 {
	return uint64(C.sdb_num_inc (d.ptr, C.CString (k), C.ulonglong(n), 0))
}

func (d *Sdb) NumDec (k string, n uint64) uint64 {
	return uint64(C.sdb_num_dec (d.ptr, C.CString (k), C.ulonglong(n), 0))
}

/* boolean */

func (d *Sdb) BoolGet (k string) bool {
	return C.sdb_bool_get (d.ptr, C.CString (k), nil) != 0
}

func (d *Sdb) BoolSet (k string, v bool) {
	if v {
		C.sdb_bool_set (d.ptr, C.CString (k), 1, 0)
	} else {
		C.sdb_bool_set (d.ptr, C.CString (k), 0, 0)
	}
}

/* query */

func (d *Sdb) QueryString(k string) string {
	v := C.sdb_querys (d.ptr, nil, 0, C.CString (k))
	defer C.free (unsafe.Pointer(v))
	return C.GoString(v)
}

func (d *Sdb) QueryLines(k string) bool {
	return C.sdb_query_lines (d.ptr, C.CString (k)) != 0
}

func (d *Sdb) Query(k string) bool {
	return C.sdb_query (d.ptr, C.CString (k)) != 0
}

/* lock */

func (d *Sdb) Lock(f string) bool {
	return C.sdb_lock (C.CString (f)) != 0
}

func (d *Sdb) LockWait(f string) bool {
	return C.sdb_lock_wait(C.CString (f)) != 0
}

func (d *Sdb) LockFile(f string) string {
	return C.GoString (C.sdb_lock_file (C.CString (f)))
}

/* array */
