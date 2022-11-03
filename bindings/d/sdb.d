import std.stdio;
import std.conv;
import std.string;
import core.stdc.string;

extern (C) {
	void* sdb_new (const char *dir, const char *name, int lock);
	char* sdb_get (void*, const char *key, uint* cas);
	const (char*) sdb_const_get (void*, const char *key, uint* cas);
	int sdb_unset (void*, const char *key, uint cas);
	int sdb_set (void*, const char *key, const char *data, uint cas);
	void sdb_free (void* s);
	bool sdb_sync (void* s);
}

public class Sdb {
	private void *db;

	~this () {
		sdb_free (db);
	}

	public this () {
		db = sdb_new (null, null, 0);
		writeln ("Hello World");
	}

	this (const char *file) {
		db = sdb_new (".", file, 0);
	}

	public bool sync () {
		return sdb_sync (db);
	}

	public bool unset(string key, uint cas=0) {
		return sdb_unset (db, cast(char*)key, cas) != 0;
	}

	public bool set(string key, string value, uint cas=0) {
		return sdb_set (db, cast(char*)key, cast(char*)value, cas) != 0;
	}

	public string get(string key, uint* cas=null) {
		auto s = sdb_const_get (db, cast(char*)key, cas);
		return s? to!string (s): null;
	}
}
