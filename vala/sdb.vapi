namespace SimpleDB {
	[Compact]
	[CCode (cheader_filename="sdb.h", cname="struct sdb_t", cprefix="sdb_")]
	public class Sdb {
		/* lifecycle */
		public Sdb (string name);
		public void sync ();
		/* string */
		public string @get (string key);
		public bool @set (string key, string @value);
		/* numeric */
		public void inc (string key);
		public void dec (string key);
		public uint64 getn (string key);
		public void setn (string key, uint64 num);
	}
}
