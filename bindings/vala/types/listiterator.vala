using GLib;

namespace SdbTypes {
	public class ListIterator : SdbTypes.Iterator {
		SdbInstance si;
		string head;
		string n;
		string name;

		public ListIterator (SdbInstance si, string name, string head) {
			this.si = si;
			this.name = name;
			this.n = this.head = head;
		}
		/* DUPPED */
		private string get_next(string node) {
			return si.get ("list."+name+"."+node+".next") ?? "";
		}

		public override bool next() {
			head = n;
			n = get_next (head);
			return head != "";
		}

		public override string @get() {
			return si.get ("list."+name+"."+head+".value");
		}
	}
}
