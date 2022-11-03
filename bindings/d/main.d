import std.stdio;
import sdb;

int main() {
	auto s = new sdb.Sdb ("jaja.sdb");

	s.set ("Hello", "hello");
	writeln ("output: ", s.get ("Hello"));

	s.set ("Hello", "world");
	writeln ("output: ", s.get ("Hello"));
	s.unset ("Hello");
	if (s.get ("Hello") == "") {
		s.set ("Hello", "works");
		writeln ("output: ", s.get ("Hello"));
	} else {
		writeln ("failed to unset");
	}
	s.sync();

	return 0;
}
