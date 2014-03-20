#include <sdb.h>
#include <stdio.h>
#include <stdlib.h>

static Sdb *s = NULL;
typedef int TestFcn(const char **name);

static int iseq(const char *a, const char *b) {
	int ret = !a || !b;
	if (ret) return 0;
	ret = !!! strcmp (a, b);
	if (!ret) eprintf (".-- (%s) vs (%s)\n", a, b);
	return ret;
}

static int test(TestFcn tf) {
	const char *name = NULL;
	int ret = tf (&name);
	printf ("%s  %s\n", ret?"OK":"ER", name);
	return ret;
}

static int test_array_set(const char **name) {
	*name = __FUNCTION__;
	sdb_set (s, "key", "foo,bar", 0);
	sdb_array_set (s, "key", 1, "lol", 0);
	return iseq (sdb_const_get (s, "key", 0), "foo,lol");
}

static int test_array_set2(const char **name) {
	*name = __FUNCTION__;
	sdb_unset (s, "key", 0);
	sdb_array_set (s, "key", 0, "foo", 0);
	sdb_array_set (s, "key", 1, "lol", 0);
	sdb_array_set (s, "key", 2, "wow", 0);
	return iseq (sdb_const_get (s, "key", 0), "foo,lol,wow");
}

static int test_array_set3(const char **name) {
	*name = __FUNCTION__;
	sdb_unset (s, "key", 0);
	sdb_array_set (s, "key", 0, "foo", 0);
	sdb_array_set (s, "key", 2, "lol", 0);
	sdb_array_set (s, "key", 1, "wow", 0);
	return iseq (sdb_const_get (s, "key", 0), "foo,lol,wow");
}

static int test_array_set4(const char **name) {
	*name = __FUNCTION__;
	sdb_unset (s, "key", 0);
	sdb_array_set (s, "key", 0, "foo", 0);
	sdb_array_set (s, "key", 3, "lol", 0);
	sdb_array_set (s, "key", 1, "wow", 0);
	sdb_array_set (s, "key", 2, "two", 0);
	return iseq (sdb_const_get (s, "key", 0), "foo,wow,two,lol");
}

static int test_array_unset(const char **name) {
	*name = __FUNCTION__;
	sdb_set (s, "key", "foo,bar", 0);
	sdb_array_unset (s, "key", 1, 0);
	return iseq (sdb_const_get (s, "key", 0), "foo");
}

static int test_array_unset2(const char **name) {
	*name = __FUNCTION__;
	sdb_set (s, "key", "foo,bar,cow", 0);
	sdb_array_unset (s, "key", 1, 0);
	return iseq (sdb_const_get (s, "key", 0), "foo,,cow");
}

static int test_array_delete(const char **name) {
	*name = __FUNCTION__;
	sdb_set (s, "key", "foo,bar,cow", 0);
	sdb_array_delete (s, "key", 1, 0);
	return iseq (sdb_const_get (s, "key", 0), "foo,cow");
}

static int test_array_push (const char **name) {
	*name = __FUNCTION__;
	sdb_set (s, "key", "foo,bar", 0);
	sdb_array_push (s, "key", "cow", 0);
	return iseq (sdb_const_get (s, "key", 0), "foo,bar,cow");
}

static int test_array_push2 (const char **name) {
	*name = __FUNCTION__;
	sdb_set (s, "key", "foo,bar,", 0);
	sdb_array_push (s, "key", "cow", 0);
	return iseq (sdb_const_get (s, "key", 0), "foo,bar,cow");
}

static TestFcn *tests[] = {
	test_array_set,
	test_array_set2,
	test_array_set3,
	test_array_set4,
	test_array_unset,
	test_array_unset2,
	test_array_delete,
	test_array_push,
	test_array_push2,
	NULL
};

int main() {
	int i, res = 0;
	s = sdb_new (NULL, NULL, 0);
	for (i = 0; tests[i]; i++)
		res |= !! test (tests[i]);
	sdb_free (s);
	return res;
}
