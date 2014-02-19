import std.stdio;
import std.stdio;
import std.conv;
import std.string;
import core.stdc.string;

extern (C) {
	void* sdb_new (const char *dir, int lock);
	char* sdb_get (void*, const char *key, uint* cas);
	const (char*) sdb_const_get (void*, const char *key, uint* cas);
	int sdb_unset (void*, const char *key, uint cas);
	int sdb_set (void*, const char *key, const char *data, uint cas);
	void sdb_free (void* s);
}

class Sdb {
	private void *db;

	~this () {
		sdb_free (db);
	}

	this () {
		db = sdb_new (null, 0);
		writeln ("Hello World");
	}

	this (const char *file) {
		db = sdb_new (file, 0);
		writeln ("Hello World on file ", file);
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

int main() {
	auto s = new Sdb ("jaja");

	s.set ("Hello", "hello");
	writeln ("output: ", s.get ("Hello"));
	s.set ("Hello", "world");
	writeln ("output: ", s.get ("Hello"));
	s.set ("Hello", "world");
	writeln ("output: ", s.get ("Hello"));
	s.unset ("Hello");
	writeln ("output: ", s.get ("Hello"));
	return 0;
}
