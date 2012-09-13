namespace McSdb {
	[Compact]
	[CCode (cheader_filename="mcsdb.h", cname="McSdbClient", cprefix="mcsdb_client_")]
	public class McSdbClient {
		/* lifecycle */
		public McSdbClient (string host, string port);
		/* string */
		public string @get (string key, out uint64? expire = null);
		public void @set (string key, string @value, uint64 exptime=0);
		public bool add (string key, string @value, uint64 exptime=0);
		public bool append (string key, string @value, uint64 exptime=0);
		public bool prepend (string key, string @value, uint64 exptime=0);
		public bool replace (string key, string @value, uint64 exptime=0);
		/* numeric */
		public string incr (string key, uint64 n);
		public string decr (string key, uint64 n);
		/* delete */
		public bool remove (string key, uint64 exptime=0);
		/* time 
		public bool exists (string key);
		public bool nexists (string key);
		public uint64 get_expire (string key);
		public bool expire (string key, uint64 time);
		public static uint64 now ();
		 */
	}
}
