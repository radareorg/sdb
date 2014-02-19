namespace SdbTypes {
	public class Hash {
		SdbInstance si;
		string name;
		const string FS = "\x1c";

		public string[] keys {
			owned get {
				var k = si.get ("hash."+name+".keys");
				return k.split (FS);
			}
		}

		public Hash(SdbInstance si, string name) {
			this.si = si;
			this.name = name;
		}

		public void @set(string key, string val) {
			if (si.get ("hash."+name+"."+key) == null) {
				var kstr = si.get ("hash."+name+".keys") ?? "";
				if (kstr != "")
					kstr += FS;
				si.set ("hash."+name+".keys", kstr+key);
			}
			si.set ("hash."+name+"."+key, val);
		}

		public string @get(string key) {
			return si.get ("hash."+name+"."+key);
		}

		public void remove (string key) {
			var kstr = "";
			foreach (var k in keys) {
				if (k != key) {
					if (kstr != "")
						kstr += FS;
					kstr += k;
				}
			}
			si.unset ("hash."+name+"."+key);
			si.set ("hash."+name+"keys", kstr);
		}

		public uint size () {
			return keys.length;
		}
	}
}
