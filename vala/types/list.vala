namespace SdbTypes {
	public class List<G> {
		SdbInstance si;
		string name;

		public class List(SdbInstance si, string name) {
			this.si = si;
			this.name = name;
		}

		public void append(string val) {
		}

		public void prepend(string val) {
		}

		public ListIterator iterator() {
			return new ListIterator ();
		}

		public void remove() {
		}

		public int length () {
			return 0;
		}
	}
}
