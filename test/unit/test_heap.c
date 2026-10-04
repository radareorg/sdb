#include "minunit.h"
#include <string.h>
#include <sdb/sdb.h>
#include <sdb/heap.h>

#if USE_MMAN
extern const SdbGlobalHeap sdb_gh_custom;

static void use_custom_heap(void) {
	sdb_gh_use (&sdb_gh_custom);
}

static bool test_heap_realloc_zero_returns_null(void) {
	use_custom_heap ();
	char *ptr = sdb_gh_malloc (64);
	mu_assert_notnull (ptr, "custom heap malloc failed");
	memset (ptr, 0x41, 64);
	mu_assert_null (sdb_gh_realloc (ptr, 0), "realloc(size=0) must return NULL");
	mu_end;
}

static bool test_heap_coalesces_middle_blocks(void) {
	use_custom_heap ();
	char *prefix = sdb_gh_malloc (64);
	char *left = sdb_gh_malloc (64);
	char *right = sdb_gh_malloc (64);
	mu_assert_notnull (prefix, "prefix allocation failed");
	mu_assert_notnull (left, "left allocation failed");
	mu_assert_notnull (right, "right allocation failed");

	sdb_gh_free (left);
	sdb_gh_free (right);

	char *merged = sdb_gh_malloc (96);
	mu_assert_notnull (merged, "merged allocation failed");
	mu_assert_ptreq (merged, left, "coalesced block should be reused in place");

	sdb_gh_free (merged);
	sdb_gh_free (prefix);
	mu_end;
}

static bool test_heap_realloc_grows_in_place(void) {
	use_custom_heap ();
	char *prefix = sdb_gh_malloc (64);
	char *ptr = sdb_gh_malloc (32);
	char *next = sdb_gh_malloc (128);
	char pattern[32];
	memset (pattern, 0x5a, sizeof (pattern));
	mu_assert_notnull (prefix, "prefix allocation failed");
	mu_assert_notnull (ptr, "pointer allocation failed");
	mu_assert_notnull (next, "next allocation failed");
	memcpy (ptr, pattern, sizeof (pattern));

	sdb_gh_free (next);
	char *grown = sdb_gh_realloc (ptr, 96);
	mu_assert_notnull (grown, "realloc grow failed");
	mu_assert_ptreq (grown, ptr, "realloc should grow in place when next block is free");
	mu_assert_memeq (grown, pattern, sizeof (pattern), "realloc must preserve payload data");

	sdb_gh_free (grown);
	sdb_gh_free (prefix);
	mu_end;
}

static bool test_heap_page_free_smoke(void) {
	enum { EXACT_PAGE_PAYLOAD = 131056 };
	use_custom_heap ();
	for (int i = 0; i < 64; i++) {
		char *ptr = sdb_gh_malloc (EXACT_PAGE_PAYLOAD);
		mu_assert_notnull (ptr, "page-sized allocation failed");
		memset (ptr, 0x23, EXACT_PAGE_PAYLOAD);
		sdb_gh_free (ptr);

		// Repeatedly remap after a full-page free to catch stale tail hints.
		ptr = sdb_gh_malloc (32);
		mu_assert_notnull (ptr, "allocation after page free failed");
		memset (ptr, 0x42, 32);
		sdb_gh_free (ptr);
	}
	mu_end;
}

#endif

static int all_tests(void) {
#if USE_MMAN
	mu_run_test (test_heap_realloc_zero_returns_null);
	mu_run_test (test_heap_coalesces_middle_blocks);
	mu_run_test (test_heap_realloc_grows_in_place);
	mu_run_test (test_heap_page_free_smoke);
#endif
	return tests_passed != tests_run;
}

int main(int argc, char **argv) {
	int rc = all_tests ();
	sdb_gh_use (NULL);
	return rc;
}
