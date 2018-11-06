#include "./r_slist.h"

int main() {
	RSList *s = r_slist_new();
	r_slist_add (s, "hello", 0, 32);

	void *name = r_slist_get_at (s, 16);
	eprintf ("%s\n", name);

	r_slist_free (s);

	return 0;
}
