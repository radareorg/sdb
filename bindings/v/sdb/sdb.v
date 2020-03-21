module sdb

#flag -I../../src
#flag ../../src/libsdb.a
#include "sdb.h"

pub struct Sdb {}

fn C.sdb_new0() &Sdb
fn C.sdb_new(path, name string, lock bool) &Sdb
fn C.sdb_stats(sdb &Sdb, disk &int, mem &int) bool
fn C.sdb_unset(sdb &Sdb, value string, cas &int) bool
fn C.sdb_set(sdb &Sdb, key string, value string, cas &int) bool
fn C.sdb_reset(sdb &Sdb)
fn C.sdb_exists(sdb &Sdb, key string) bool
fn C.sdb_add(sdb &Sdb, key string, value string, cas &int) bool
fn C.sdb_const_get(sdb &Sdb, key string, cas int) byteptr
fn C.sdb_free(&Sdb)
// num
fn C.sdb_num_set(sdb &Sdb, key string, value u64, cas int) bool
fn C.sdb_num_add(sdb &Sdb, key string, value u64, cas int) bool
fn C.sdb_num_get(sdb &Sdb, key string, cas &int) u64
fn C.sdb_num_inc(sdb &Sdb, key string, cas &int) u64
fn C.sdb_num_dec(sdb &Sdb, key string, cas &int) u64

// lifecycle

pub fn new0() &Sdb {
	return C.sdb_new0()
}

pub fn new(path, name string, lock bool) &Sdb {
	return C.sdb_new(path.str, name.str, lock)
}

pub fn (sdb &Sdb)free() {
	C.sdb_free(sdb)
}

pub fn (sdb &Sdb)reset() {
	C.sdb_reset(sdb)
}

pub fn(sdb &Sdb)str() string {
	mut disk := 0
	mut mem := 0
	C.sdb_stats(sdb, &disk, &mem)
	return '(${disk} ${mem})'
}

pub fn (sdb &Sdb)exists(k string) bool {
	return C.sdb_exists(sdb, k.str)
}

pub fn (sdb &Sdb)set(k, v string) bool {
	return C.sdb_set(sdb, k.str, v.str, 0)
}

pub fn (sdb &Sdb)add(k, v string) bool {
	return C.sdb_add(sdb, k.str, v.str, 0)
}

pub fn (sdb &Sdb)unset(k string) bool {
	return C.sdb_unset(sdb, k.str, 0)
}

pub fn (sdb &Sdb)get(k string) string {
	return tos_clone(C.sdb_const_get(sdb, k.str, 0))
}

// numbers

pub fn (sdb &Sdb)num_get(k string) u64 {
	return C.sdb_num_get(sdb, k.str, 0)
}

pub fn (sdb &Sdb)num_set(k string, v u64) bool {
	return C.sdb_num_set(sdb, k.str, v, 0)
}

pub fn (sdb &Sdb)num_add(k string, v u64) bool {
	return C.sdb_num_add(sdb, k.str, v, 0)
}

pub fn (sdb &Sdb)num_inc(k string) u64 {
	return C.sdb_num_inc(sdb, k.str, 1, 0)
}

pub fn (sdb &Sdb)num_dec(k string) u64 {
	return C.sdb_num_dec(sdb, k.str, 1, 0)
}
