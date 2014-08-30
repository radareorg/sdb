/* sdb api testsuite - Copyright 2014 - pancake */

#include <sdb.h>
#include <stdio.h>
#include <stdlib.h>


#define OK "\x1b[32mOK\x1b[0m"
#define ER "\x1b[31mER\x1b[0m"
static int success = 0;
static Sdb *s = NULL;
typedef int TestFcn(const char **name);

static int iseq(const char *a, const char *b) {
	int ret = !a || !b;
	if (ret) return b==0;
	ret = !!! strcmp (a, b);
	if (!ret) eprintf (".-- (%s) expected (%s)\n", a, b);
	return ret;
}

static int test(TestFcn tf) {
	const char *name = NULL;
	int ret = tf (&name);
	printf ("%s  %s\n", ret? OK:ER, name);
	success += ret;
	return ret;
}

/* --- testsuite --- */
#define TEST(x) static int test_refs_##x (const char **name) {\
	*name = __FUNCTION__;
#define EXPECT(x) return iseq (sdb_const_get (s, "key", 0), x); }

TEST(refs)
	Sdb *n = sdb_new0 ();
	sdb_num_set (s, "key", n->refs, 0);
	sdb_free (n);
	EXPECT("0x1");
TEST(refs2)
	Sdb *n = sdb_new0 ();
	Sdb *n2 = sdb_new0 ();
	sdb_ns_set (n2, "foo", n);
	sdb_num_set (s, "key", n->refs, 0);
	EXPECT("0x2");

static TestFcn *tests[] = {
	test_refs_refs,
	test_refs_refs2,
	NULL
};

int main() {
	int i, res = 0;
	s = sdb_new (NULL, NULL, 0);
	for (i = 0; tests[i]; i++) {
		res |= !!! test (tests[i]);
	}
	sdb_free (s);
	printf ("TOTAL   %d\n", i);
	printf ("SUCCESS %d\n", success);
	printf ("FAILED  %d\n", i-success);
	printf ("RATIO   %d%%\n", (100*success)/i);
eprintf ("RES = %d\n", res);
	return !! res;
}
