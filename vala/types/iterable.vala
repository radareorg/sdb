using GLib;

public interface SdbTypes.Iterable<G> : Object {
        public abstract Type element_type { get; }
        public abstract Iterator<G> iterator ();
}
