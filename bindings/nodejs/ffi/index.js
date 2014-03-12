/* nodejs bindings for sdb */

var FFI = require ("node-ffi");

// TODO: this is not CAS-safe yet
var SDB = new FFI.Library ("libsdb", {
        "sdb_new": [ "pointer", ["string", "string", "int32"]]
,	"sdb_disk_create": [ "int", ["pointer"]]
//,	"malloc": [ "pointer", ["int"]]
,	"sdb_disk_insert": [ "int", ["pointer", "string", "string"]]
,	"sdb_disk_finish": [ "int", ["pointer"]]
,	"sdb_unset": [ "int", ["pointer", "string", "int"]]
,	"sdb_query": [ "int", ["pointer", "string"]]
,	"sdb_querys": [ "string", ["pointer", "pointer", "int", "string"]]
,	"sdb_get": [ "string", ["pointer", "string", "pointer"]]
,	"sdb_set": [ "int", ["pointer", "string", "string", "uint32"]]
,	"sdb_add": [ "int", ["pointer", "string", "string", "uint32"]]
,	"sdb_decode": [ "string", ["string", "pointer"]]
,	"sdb_encode": [ "string", ["string"]]
,	"sdb_num_inc": [ "int", ["pointer", "string", "int64", "uint32"]]
,	"sdb_num_dec": [ "int", ["pointer", "string", "int64"]]
,	"sdb_num_get": [ "int64", ["pointer", "string", "pointer"]]
,	"sdb_num_set": [ "int", ["pointer", "string", "int64"]]
,	"sdb_hash": [ "int32", []]
,	"sdb_now": [ "int64", []]
//,	"sdb_lock": [ "int", ["string"]]
//,	"sdb_unlock": [ "void", ["string"]]
,	"sdb_exists": ["int", ["pointer", "string"]]
,	"sdb_expire_set": [ "int32", ["pointer", "string", "int64"]]
,	"sdb_expire_get": [ "int64", ["pointer", "string"]]
,	"sdb_json_get": [ "string", ["pointer", "string", "string", "pointer"]]
,	"sdb_json_set": [ "int", ["pointer", "string", "string", "string"]]
,	"sdb_json_num_get": [ "int", ["pointer", "string", "string"]]
,	"sdb_json_num_set": [ "int", ["pointer", "string", "string", "int"]]
,	"sdb_json_indent": [ "string", ["string"]]
,	"sdb_json_unindent": [ "string", ["string"]]
,	"sdb_sync": [ "int32", ["pointer"]]
});

exports.open = function (file) {
	this.o = SDB.sdb_new (null, file, 0);
	this.json_get = function (key, path) { return SDB.sdb_json_get (this.o, key, path, null); }
	this.json_set = function (key, path, val) { return SDB.sdb_json_set (this.o, key, path, val, 0); }
	this.json_indent = function (str) { return SDB.sdb_json_indent (str); }
	this.json_unindent = function (str) { return SDB.sdb_json_unindent (str); }
	this.get = function (x) { return SDB.sdb_get (this.o, x, null); }
	this.set = function (x, y) { return SDB.sdb_set (this.o, x, y, 0); }
	this.add = function (x, y) { return SDB.sdb_add (this.o, x, y, 0); }
	this.unset = function (x, y) { return SDB.sdb_unset (this.o, x, y); }
	//this.malloc = function (x){ return SDB.malloc (x); }
	this.exists = function (key) { return SDB.sdb_exists (this.o, key); }
	this.encode = function (key) { return SDB.sdb_encode(key); }
	this.decode = function (key) { return SDB.sdb_decode(key, null); }
	this.num_inc = function (x, y) { return SDB.sdb_num_inc (this.o, x, y?y:1, 0); }
	this.num_dec = function (x, y) { return SDB.sdb_num_dec (this.o, x, y, 0); }
	this.num_set = function (x, y) { return SDB.sdb_num_set (this.o, x, y, 0); }
	this.num_get = function (x) { return SDB.sdb_num_get (this.o, x); }
	this.query = function (x) { SDB.sdb_query (this.o, x); }
	this.querys = function (x) { return SDB.sdb_querys (this.o, null, 0, x); }
	this.sync = function () { SDB.sdb_sync (this.o); }
	return this;
}
