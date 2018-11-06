#include "./r_trie.h"

int main() {
	RTrie *t = r_trie_new ();
	r_trie_insert (t, "findus", "rules");
	r_trie_insert (t, "gamba", "kunde");
	eprintf ("%s %s\n", r_trie_find (t, "findus"), r_trie_find (t, "gamba"));
	r_trie_delete (t, "findus");
	eprintf ("%s %s\n", r_trie_find (t, "findus"), r_trie_find (t, "gamba"));
	r_trie_free (t);
}
