namespace SimpleDB {
	[Compact]
	[CCode (cheader_filename="sdb.h", cname="struct sdb_t", cprefix="sdb_")]
	public class Sdb {
		/* lifecycle */
		public Sdb (string name, bool locked=false);
		public void sync ();
		/* string */
		public string @get (string key);
		public bool @set (string key, string @value);
		/* numeric */
		public uint64 inc (string key);
		public uint64 dec (string key);
		public uint64 getn (string key);
		public void setn (string key, uint64 num);
		/* delete */
		public bool exists (string key);
		public bool @delete (string key);
	}
}
