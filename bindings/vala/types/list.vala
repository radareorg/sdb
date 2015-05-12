namespace SdbTypes {
	public class List : Iterable {
		SdbInstance si;
		string name;

		public List(SdbInstance si, string name) {
			this.si = si;
			this.name = name;
		}

		~List() {
			clear ();
		}

		public string get_head() {
			return si.get ("list."+name+".head") ?? "";
		}

 		public string get_uuid () {
			string c = si.get ("list."+name+".count") ?? "0";
			var cn = (uint64.parse (c)+1).to_string ();
			si.set ("list."+name+".count", cn);
			return cn;
		}

		public string get_tail() {
			return si.get ("list."+name+".tail") ?? "";
		}

		public string get_next(string node) {
			return si.get ("list."+name+"."+node+".next");
		}

		public string get_prev(string node) {
			return si.get ("list."+name+"."+node+".prev");
		}

		public override Iterator iterator () {
			return (Iterator) new ListIterator (si, name, get_head ());
		}

		public void append(string val) {
			string tail = get_tail ();
			string node = get_uuid ();
			if (tail != "")
				si.set ("list."+name+"."+tail+".next", node);
			si.set ("list."+name+"."+node+".value", val);
			si.set ("list."+name+"."+node+".prev", tail);
			si.set ("list."+name+".tail", node);
			if (get_head () == "")
				si.set ("list."+name+".head", node);
			inc ();
		}

		public void prepend(string val) {
			string head = get_head ();
			string node = get_uuid ();
			if (head != "")
				si.set ("list."+name+"."+head+".prev", node);
			si.set ("list."+name+"."+node+".value", val);
			si.set ("list."+name+"."+node+".next", head);
			si.set ("list."+name+".head", node);
			inc ();
		}

		public bool contains (string val) {
			return (index_of (val) != -1);
		}

		public void clear () {
			// TODO
		}

		public int index_of (string val) {
			int i = 0;
			foreach (var s in this) {
				if (s == val)
					return i;
				i++;
			}
			return -1;
		}

		public void insert (int index, string val) {
			string prev = get_head ();
			string node = prev;
			for (int i = 0; node != ""; i++) {
				if (i == index) {
					var newnode = get_uuid ();
					si.set ("list."+name+"."+newnode+".value", val);
					si.set ("list."+name+"."+newnode+".prev", prev);
					si.set ("list."+name+"."+newnode+".next", node);
					si.set ("list."+name+"."+node+".prev", newnode);
					si.set ("list."+name+"."+prev+".next", newnode);
					inc ();
				}
				prev = node;
				node = si.get ("list."+name+"."+prev+".next") ?? "";
			}
		}

		public string remove_at(int index) {
			string prev = get_head ();
			if (prev == "")
				return "";
			string node = prev;
			for (int i = 0; node != ""; i++) {
				if (i == index) {
					var nodenext = get_next (node);
					var nodevalue = si.get ("list."+name+"."+node+".value");
					if (get_tail () == node)
						si.set ("list."+name+".tail", prev);
					if (get_head () == node)
						si.set ("list."+name+".head", nodenext);
					si.unset ("list."+name+"."+node+".value");
					si.unset ("list."+name+"."+node+".next");
					si.unset ("list."+name+"."+node+".prev");
					si.set ("list."+name+"."+prev+".next", nodenext);
					si.set ("list."+name+"."+nodenext+".prev", prev);
					dec ();
					return nodevalue;
				}
				prev = node;
				node = si.get ("list."+name+"."+prev+".next") ?? "";
			}
			return "";
		}

		private void inc () {
			var l = size ();
			si.set ("list."+name+".length", (l+1).to_string ());
		}

		private void dec () {
			var l = size ();
			if (l != 0)
				si.set ("list."+name+".length", (l-1).to_string ());
		}

		public int size () {
			var l = si.get ("list."+name+".length") ?? "0";
			return int.parse (l);
		}
	}
}
