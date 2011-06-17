using SdbTypes;

void main () {
	print ("Hello types!\n");
	var si = new SdbInstance ("food.sdb");

// TODO	var s = si.get_stack ("s");
	var s = new SdbTypes.Stack (si, "s");
	s.push ("World");
	s.push ("Hello");
	print ("stack = %s %s\n", s.pop (), s.pop ());
	if (s.pop () == null)
		print ("Cant pop moar.. which is ok\n");


	var h = new SdbTypes.Hash (si, "h");
	h.set ("foo", "hello");
	h.set ("bar", "world");
	print ("hash = %s %s\n", h.get ("foo"), h.get ("bar"));


	si.sync ();
/*
	var l = new SdbTypes.List<string> (si, "l");
	l.append ("one");
	l.append ("two");
	l.append ("tri");
	foreach (var item in l) {
		print (" -> %s\n", item);
	}
*/
}
