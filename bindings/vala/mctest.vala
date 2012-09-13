using McSdb;

int main() {
	var c = new McSdbClient ("localhost", "11211");
	if (c == null) {
		stderr.printf ("Cannot connect\n");
		return 1;
	}
	c.set ("count", "0");
	for(int i = 0 ; i<2000; i++) {
		c.add ("foo", "Hello");
		c.set ("bar", "World");
		print ("%s %s\n", c.get ("foo"), c.get ("bar"));
		c.remove ("foo");
		c.incr ("count", 1);
	}
	return 0;
}
