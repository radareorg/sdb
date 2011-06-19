using GLib;

namespace SdbTypes {
	public abstract class Iterator {
		public abstract bool next();
		public abstract string @get();
	}
}
