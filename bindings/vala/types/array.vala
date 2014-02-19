public class SdbTypes.Array : Iterable {
	SdbInstance si;
	string name;

	public Array (SdbInstance si, string name) {
		this.si = si;
		this.name = name;
	}

	~Array () {
		clear ();
	}

	public new string @get (int idx) {
		if (idx>=0)
			return si.get ("array."+name+"."+idx.to_string ());
		return "";
	}

	public new void @set (int idx, string val) {
		if (idx<0)
			return;
		si.set ("array."+name+"."+idx.to_string (), val);
		update_limits (idx);
	}

	public bool remove (string val) {
		int idx = index_of (val);
		if (idx == -1)
			return false;
		return remove_at (idx) != null;
	}

	public int index_of (string val) {
		int lower = get_lower ();
		int upper = get_upper ();
		for (int i = lower; i<upper; i++) {
			var item = si.get ("list."+name+"."+i.to_string ());
			if (item != null && item == val)
				return i;
		}
		return -1;
	}

	public bool contains (string val) {
		return (index_of (val) != -1);
	}

	public void clear () {
		int lower = get_lower ();
		int upper = get_upper ();
		for (int idx = lower; idx<upper; idx++) {
			var idxs = idx.to_string ();
			var item = si.get ("list."+name+"."+idxs);
			if (item != null)
				si.unset ("list."+name+"."+idxs);
		}
		si.set ("list."+name+".lower", "0");
		si.set ("list."+name+".upper", "0");
	}

	public string? remove_at (int idx) {
		if (idx<0)
			return null;
		var old = si.get ("array."+name+"."+idx.to_string ());
		si.unset ("array."+name+"."+idx.to_string ());
		update_limits (idx);
		return old;
	}

	public int get_lower () {
		var s = si.get ("array."+name+".lower");
		if (s == null || s == "")
			return 0;
		return int.parse (s);
	}

	public int get_upper () {
		var s = si.get ("array."+name+".upper");
		if (s == null || s == "")
			return 0;
		return int.parse (s);
	}

	public void update_limits (int idx) {
		int lower = get_lower ();
		int upper = get_upper ();
		if (idx<lower)
			si.set ("array."+name+".lower", idx.to_string ());
		if (idx>upper)
			si.set ("array."+name+".upper", idx.to_string ());
	}

	//public new ArrayIterator iterator () {
	public override Iterator iterator () {
		return (Iterator)new ArrayIterator (si, name, get_lower (), get_upper ());
	}
}
