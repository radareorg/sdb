using SDB;
using McSdb;

// TODO: use single interface for mcsdb and sdb
// TODO: implement locking for files and for queries
namespace SdbTypes {
	public class SdbInstance : Object {
		Sdb? s = null;
		McSdbClient? m = null;

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
				m = new McSdbClient (host, port.to_string ());
			}
		}

		~SdbInstance () {
			s = null;
			m = null;
		}

		public bool set_bool(string name, bool val) {
			if (m != null) {
				m.set (name, val.to_string ());
				return true;
			}
			return s.set (name, val.to_string ());
		}
		public bool get_bool(string name) {
			if (m != null)
				return m.get (name) == "true";
			return s.get (name) == "true";
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
			if (m != null)
				m.set (key, val);
			else s.set (key, val);
		}

		public new string? @get(string key) {
			if (m != null)
				return m.get (key);
			return s.get (key);
		}
		// TODO: add getn/setn?

		public void unset(string key) {
			if (m != null)
				m.remove (key);
			else s.unset (key);
		}

		public uint64 incr (string key, uint64 delta=1) {
			if (m != null)
				return uint64.parse (m.incr (key, delta));
			return s.num_inc (key, delta);
		}

		public uint64 decr (string key, uint64 delta=1) {
			if (m != null)
				return uint64.parse (m.decr (key, delta));
			return s.num_dec (key, delta);
		}

		public void sync () {
			if (s != null)
				s.sync ();
		}
	}
}
