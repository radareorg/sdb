namespace SimpleDB {
	[Compact]
	[CCode (cheader_filename="sdb.h", cname="struct sdb_t", cprefix="sdb_", free_function="sdb_free")]
	public class Sdb {
		/* lifecycle */
		public Sdb (string? path=null, string? file=null, bool locked=false);
		public bool sync ();
		/* query */
		public bool query(string cmd);
		public string querys(string? buf=null, int len=0, string cmd="");
		/* string */
		public string @get (string key, out int? cas=null);
		public bool @add (string key, string val, uint32 cas=0);
		public bool @set (string key, string val, uint32 cas=0);
		/* arrays */
		public int alength (string key);
		public string aget (string key, int idx, uint32 *cas=null);
		public string aset (string key, int idx, string val, uint32 *cas=null);
		public bool adel (string key, int idx, uint32 cas=0);
		/* numeric */
		public bool setn (string key, uint64 num, uint32 cas=0);
		public uint64 getn (string key, uint32 *cas = null);
		public uint64 inc (string key, uint64 n, uint32 cas=0);
		public uint64 dec (string key, uint64 n, uint32 cas=0);
		/* json */
		public string json_get (string key, string path, uint32 *cas = null);
		public string json_set (string key, string path, string val, uint32 cas);
		public int json_inc (string key, string path, int n, uint32 cas=0);
		public int json_dec (string key, string path, int n, uint32 cas=0);
		public static string json_indent (string json);
		public static string json_unindent (string json);
		/* remove */
		public bool exists (string key);
		public bool nexists (string key);
		public bool remove (string key, int cas=0);
		public void reset ();
		public void drop ();
		/* time */
		public uint64 get_expire (string key);
		public bool expire (string key, uint64 time);
		public static uint64 now ();
	}
}
