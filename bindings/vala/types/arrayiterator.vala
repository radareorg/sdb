namespace SdbTypes {
	public class ArrayIterator : SdbTypes.Iterator {
		SdbInstance si;
		string name;
		int upper;
		int idx;
		int cur;

		public ArrayIterator (SdbInstance si, string name, int lower, int upper) {
			this.si = si;
			this.name = name;
			this.cur =
			this.idx = lower;
			this.upper = upper;
		}

		public override bool next() {
			cur = idx++;
			for (; idx<=upper; idx++) {
				var c = si.get ("array."+name+"."+idx.to_string ());
				if (c != null)
					return true;
			}
			return get () != null;
		}

		public override string @get() {
			return si.get ("array."+name+"."+cur.to_string ());
		}
	}
}
