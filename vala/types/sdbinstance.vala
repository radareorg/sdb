using SimpleDB;

namespace SdbTypes {
	public class SdbInstance {
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
			}
		}

		public void @set(string key, string val) {
			s.set (key, val);
		}

		public string @get(string key) {
			return s.get (key);
		}

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
