namespace SimpleDB {
	[Compact]
	[CCode (cheader_filename="sdb.h", cname="struct sdb_t", cprefix="sdb_", free_function="sdb_free")]
	public class Sdb {
		/* lifecycle */
		public Sdb (string? name=null, bool locked=false);
		public void sync ();
		/* string */
		public string @get (string key, out int? cas=null);
		public bool @add (string key, string @value);
		public bool @set (string key, string @value, uint32 cas=0);
		/* numeric */
		public uint64 inc (string key, uint64 n, uint32 cas=0);
		public uint64 dec (string key, uint64 n, uint32 cas=0);
		public uint64 getn (string key);
		public void setn (string key, uint64 num);
		/* json */
		public string json_get (string key, string path, uint32 *cas = null);
		public string json_set (string key, string path, string val, uint32 cas);
		public static string json_indent (string json);
		public static string json_unindent (string json);
		/* delete */
		public bool exists (string key);
		public bool nexists (string key);
		public bool @delete (string key, int cas=0);
		public void flush ();
		/* time */
		public uint64 get_expire (string key);
		public bool expire (string key, uint64 time);
		public static uint64 now ();
	}
}
