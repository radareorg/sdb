using SimpleDB;

void main () {
	var s = new Sdb ("test.sdb");
	s.set ("user.name", "pancake");
	s.set ("user.password", "food");
	print ("Your name is: "+s.get ("user.name")+"\n");
	s.sync ();
	s.set ("user.name", "owned");
	print ("Your name is: "+s.get ("user.name")+"\n");
	s.sync ();

	var s = new Sdb.with_lock ("uuid.sdb");
	s.inc ("uuid");
	s.close ()
}
