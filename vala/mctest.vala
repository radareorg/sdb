using McSdb;

int main() {
	var c = new McSdbClient ("localhost", "11211");
	if (c == null) {
		stderr.printf ("Cannot connect\n");
		return 1;
	}
	c.set ("foo", "Hello");
	c.set ("bar", "World");
	print ("%s %s\n", c.get ("foo"), c.get ("bar"));
	return 0;
}
