using GLib;

namespace SdbTypes {
	public interface Iterator {
		public abstract bool next();
		public abstract string @get();
	}
}
