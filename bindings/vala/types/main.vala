using SdbTypes;

void main () {
	print ("Hello types!\n");
	var si = new SdbInstance ("food.sdb");

	bool is_initialized = si.get_bool ("is_initialized");

	if (is_initialized)
		print ("** already initialized **\n");
	else si.set_bool ("is_initialized", true);

	var s = si.get_stack ("s");
	s.push ("World");
	s.push ("Hello");
	foreach (var p in s) {
		print ("STACK : "+p+"\n");
	}

	print ("stack = %s %s\n", s.pop (), s.pop ());
	if (s.pop () == null)
		print ("Cant pop moar.. which is ok\n");

	print ("HASH\n");
	var h = si.get_hash ("h");
	h.set ("foo", "hello");
	h.set ("bar", "world");
	print ("  api: %s %s\n", h.get ("foo"), h.get ("bar"));
	print ("  sugar: '"+h["foo"]+" "+h["bar"]+"'\n");

	print ("KEYS\n");
	foreach (var k in h.keys) {
		print (" key : "+k+"\n");
	}

	var l = si.get_list ("l");
	if (!is_initialized) {
		l.append ("one");
		l.append ("two");
		l.append ("pene");
		l.append ("tri");
		l.prepend ("zero");
	}

	l.remove_at (3);
	l.insert (3, "----");
	print ("SIZE: %d\n", l.size ());
	foreach (var item in l)
		print (" -> %s\n", item);

	print ("ARRAY\n");

	var a = si.get_array ("a");
	a.set (0, "zero");
	a.set (1, "wan");
	a.set (3, "tri");
	a.set (8, "ouch");

	foreach (var item in a)
		print (" -> %s\n", item);

	si.sync ();
}
