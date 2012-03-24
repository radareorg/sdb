var FFI = require ("node-ffi");

var SDB = new FFI.Library ("libsdb", {
        "sdb_new": [ "pointer", ["string", "int32"]]
,	"sdb_get": [ "string", ["pointer", "string"]]
,	"sdb_set": [ "int32", ["pointer", "string", "string"]]
,	"sdb_add": [ "int32", ["pointer", "string", "string"]]
,	"sdb_sync": [ "int32", ["pointer"]]
});

exports.open = function (file) {
	this.o = SDB.sdb_new (file, 0);
	this.get = function (x) {
		return SDB.sdb_get (this.o, x);
	}
	this.set = function (x, y) {
		return SDB.sdb_set (this.o, x, y);
	}
	this.add = function (x, y) {
		return SDB.sdb_add (this.o, x, y);
	}
	this.sync = function () {
		SDB.sdb_sync (this.o);
	}
	return this;
}
