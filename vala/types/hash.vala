namespace SdbTypes {
	public class Hash {
		SdbInstance si;
		string name;

		public Hash(SdbInstance si, string name) {
			this.si = si;
			this.name = name;
		}

		public void @set(string key, string val) {
			si.set (key, val);
		}

		public string @get(string key) {
			return si.get (key);
		}
	}
}
