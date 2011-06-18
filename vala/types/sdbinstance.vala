using SimpleDB;

// TODO: implement locking for files and for queries
namespace SdbTypes {
	public class SdbInstance : Object {
		Sdb? s = null;

		public SdbInstance (string? host=null, int port=-1) {
			if (host == null) {
				/* use in memory sdb */
				s = new Sdb ();
			} else if (port==-1) {
				/* use file sdb */
				s = new Sdb (host);
			} else {
				/* use host+port memcache */
				stderr.printf ("memcache not yet supported\n");
			}
		}

		~SdbInstance () {
			s = null;
		}

		public SdbTypes.Array get_array (string name) {
			return new SdbTypes.Array (this, name);
		}

		public SdbTypes.List get_list (string name) {
			return new SdbTypes.List (this, name);
		}

		public SdbTypes.Hash get_hash (string name) {
			return new SdbTypes.Hash (this, name);
		}

		public SdbTypes.Stack get_stack (string name) {
			return new SdbTypes.Stack (this, name);
		}

		public new void @set(string key, string val) {
			s.set (key, val);
		}

		public new string? @get(string key) {
			return s.get (key);
		}
		// TODO: add getn/setn?

		public void @delete(string key) {
			s.delete (key);
		}

		public uint64 incr (string key, uint64 delta=1) {
			return s.inc (key, delta);
		}

		public uint64 decr (string key, uint64 delta=1) {
			return s.dec (key, delta);
		}

		public void sync () {
			if (s != null)
				s.sync ();
		}
	}
}
