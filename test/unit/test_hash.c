#include "minunit.h"
#include <sdb.h>

bool test_ht_insert_lookup(void) {
	SdbHash *ht = ht_new ();
	ht_insert (ht, "AAAA", "vAAAA");
	ht_insert (ht, "BBBB", "vBBBB");
	ht_insert (ht, "CCCC", "vCCCC");

	mu_assert_streq (ht_find (ht, "BBBB", NULL), "vBBBB", "BBBB value wrong");
	mu_assert_streq (ht_find (ht, "AAAA", NULL), "vAAAA", "AAAA value wrong");
	mu_assert_streq (ht_find (ht, "CCCC", NULL), "vCCCC", "CCCC value wrong");

	ht_free (ht);
	mu_end;
}

int all_tests() {
	mu_run_test (test_ht_insert_lookup);
	return tests_passed != tests_run;
}

int main(int argc, char **argv) {
	return all_tests ();
}
