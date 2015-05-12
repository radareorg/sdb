using SDB;
using Posix;

void main () {
	var s = new Sdb (null, "test.sdb");
	s.set ("user.name", "pancake");
	s.set ("user.password", "food");
	print ("Your name is: "+s.get ("user.name")+"\n");
	s.sync ();

#if TESTFLUSH
	s.flush ();
	print ("USERNAME %s\n", s.get ("user.name"));
	return;
#endif
	s.set ("user.name", "owned");
	print ("Your name is: "+s.get ("user.name")+"\n");
	s.sync ();
	s = null;

	// open database with lock
	//var s2 = new Sdb (null, "uuid.sdb", true);
	var s2 = new Sdb (null, "uuid.sdb", false);
	if (s2 != null) {
		var uuid = s2.num_inc ("uuid", 1);
		print (@"UUID: $uuid\n");

		s2.expire_set ("uuid", 2);

		Posix.sleep (1);
		string? suuid = s2.get ("uuid");
		print (@"UUID = $uuid\n");

		Posix.sleep (2);
		//s2.expire ("uuid", 10);
		suuid = s2.get ("uuid");
		if (suuid != null) {
			print (@"!!! UUID = $uuid\n");
			print ("EXPIRE TEST FAILED\n");
		} else {
			print ("EXPIRE TEST SUCCESS\n");
		}
		s2.sync ();
		s2 = null;
	} else print ("Cannot open 'uuid.sdb'\n");
}
