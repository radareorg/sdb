namespace SdbTypes {
	public class Stack : Iterable {
		SdbInstance si;
		string name;

		public Stack (SdbInstance si, string name) {
			this.si = si;
			this.name = name;
		}

		public void push (string item) {
			uint64 idx = si.incr (get_size ());
			var key = get_idx (idx);
			si.set (key, item);
		}

		public string get_idx(uint64 idx) {
			return "stack."+name+"."+idx.to_string ();
		}

		public string get_size() {
			return "stack."+name+".size";
		}

		public string? pop () {
			uint64 idx = si.decr (get_size ());
			if (idx == 0)
				return null;
			string key = get_idx (idx-1);
			string ret = si.get (key);
			si.unset (key);
			return ret;
		}

		public override Iterator iterator () {
			return (Iterator) new StackIterator (si, name, si.get (get_size ()));
		}
	}
}
