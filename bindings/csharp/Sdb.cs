using System.Runtime.InteropServices;

// requires 64bit libsdb
namespace SDB {
	public unsafe class Sdb {
		private void *p;
		[DllImport ("sdb", EntryPoint="sdb_new")]
		private static extern void* sdb_new(string a, string b, bool c);
		[DllImport ("sdb", EntryPoint="sdb_set")]
		private static extern void* sdb_set(void *d, string a, string b, int c);
		[DllImport ("sdb", EntryPoint="sdb_get")]
		private static extern string sdb_get(void *d, string a, int c);

		public unsafe Sdb(string dir=null, string fil=null, bool lck =false) {
			p = sdb_new (dir, fil, lck);
		}

		public unsafe string Get(string k, int cas=0) {
			return sdb_get (p, k, cas);
		}

		public unsafe void Set(string k, string v, int cas=0) {
			sdb_set (p, k, v, cas);
		}
	}
}
