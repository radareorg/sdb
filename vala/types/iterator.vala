using GLib;

namespace SdbTypes {
        public delegate A FoldFunc<A, G> (owned G g, owned A a);
        public delegate void ForallFunc<G> (owned G g);
}

public interface SdbTypes.Iterator<G> : Object {
	/**
	 * Advances to the next element in the iteration.
	 *
	 * @return ``true`` if the iterator has a next element
	 */
	public abstract bool next ();

	/**
	 * Checks whether there is a next element in the iteration.
	 *
	 * @return ``true`` if the iterator has a next element
	 */
	public abstract bool has_next ();

	/**
	 * Returns the current element in the iteration.
	 *
	 * @return the current element in the iteration
	 */
	public abstract G get ();

	/**
	 * Removes the current element in the iteration. The cursor is set in an
	 * in-between state. Both {@link get} and {@link remove} will fail until
	 * the next move of the cursor (calling {@link next}).
	 */
	public abstract void remove ();
	
	/**
	 * Determines wheather the call to {@link get} is legal. It is false at the
	 * beginning and after {@link remove} call and true otherwise.
	 */
	public abstract bool valid { get; }
	
	/**
	 * Determines wheather the call to {@link remove} is legal assuming the
	 * iterator is valid. The value must not change in runtime hence the user
	 * of iterator may cache it.
	 */
	public abstract bool read_only { get; }
	
	/**
	 * Standard aggragation function.
	 *
	 * It takes a function, seed and first element, returns the new seed and
	 * progress to next element when the operation repeats.
	 *
	 * Operation moves the iterator to last element in iteration. If iterator
	 * points at some element it will be included in iteration.
	 *
	 * Note: Default implementation uses {@link foreach}.
	 */
	public virtual A fold<A> (FoldFunc<A, G> f, owned A seed)
	{
		this.foreach ((item) => {seed = f ((owned) item, (owned) seed);});
		return (owned) seed;
	}
	
	/**
	 * Apply function to each element returned by iterator. 
	 *
	 * Operation moves the iterator to last element in iteration. If iterator
	 * points at some element it will be included in iteration.
	 */
	public new virtual void foreach (ForallFunc<G> f) {
		if (valid)
			f (get ());
		while (next ())
			f (get ());
	}
}

