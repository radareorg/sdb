using GLib;

namespace SdbTypes {
	public class StackIterator : SdbTypes.Iterator {
		SdbInstance si;
		string name;
		string nxt;
		string last;

		public StackIterator (SdbInstance si, string name, string last) {
			this.si = si;
			this.name = name;
			this.nxt = get_next (last);
		}
		
		/* DUPPED */
		private string get_next(string node) {
			uint64 foo = uint64.parse (node);
			if (foo == 0) return "";
			return (foo-1).to_string ();
		}

		public override bool next() {
			last = nxt;
			nxt = get_next (last);
			return last != "";
		}

		public override string @get() {
			return si.get ("stack."+name+"."+last);
		}
	}
}
