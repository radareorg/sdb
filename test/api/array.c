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
#define TEST(x) static int test_array_##x (const char **name) {\
	*name = __FUNCTION__;
#define EXPECT(x) return iseq (sdb_const_get (s, "key", 0), x); }

TEST(get)
	sdb_set (s, "key", "foo,bar", 0);
	char *k = sdb_array_get (s, "key", 1, 0);
	sdb_set (s, "key", k, 0);
	free (k);
	EXPECT("bar")

TEST(get2)
	sdb_set (s, "key", "foo,bar", 0);
	char *k = sdb_array_get (s, "key", 2, 0);
	sdb_set (s, "key", k, 0);
	free (k);
	EXPECT(NULL)

TEST(getneg)
	sdb_set (s, "key", "foo,bar", 0);
	char *k = sdb_array_get (s, "key", -1, 0);
	sdb_set (s, "key", k, 0);
	free (k);
	EXPECT("bar")

TEST(ins)
	sdb_set (s, "key", "foo,bar", 0);
	sdb_array_insert (s, "key", 1, "lol", 0);
	EXPECT("foo,lol,bar")

TEST(ins2)
	sdb_set (s, "key", "foo,bar", 0);
	sdb_array_insert (s, "key", 2, "lol", 0);
	EXPECT("foo,bar,lol")

TEST(ins3)
	sdb_set (s, "key", "foo,bar", 0);
	sdb_array_insert (s, "key", 0, "lol", 0);
	EXPECT("lol,foo,bar")


TEST(ins4)
	sdb_set (s, "key", "foo", 0);
	sdb_array_insert (s, "key", 4, "bar", 0);
	EXPECT("foo,,,,bar")

TEST(insneg)
	sdb_set (s, "key", "foo,bar", 0);
	sdb_array_insert (s, "key", -1, "lol", 0);
	EXPECT("foo,bar,lol")

TEST(set)
	sdb_set (s, "key", "foo,bar", 0);
	sdb_array_set (s, "key", 1, "lol", 0);
	EXPECT("foo,lol")

TEST(set2)
	sdb_unset (s, "key", 0);
	sdb_array_set (s, "key", 0, "foo", 0);
	sdb_array_set (s, "key", 1, "lol", 0);
	sdb_array_set (s, "key", 2, "wow", 0);
	EXPECT("foo,lol,wow");

TEST(set3)
	sdb_unset (s, "key", 0);
	sdb_array_set (s, "key", 0, "foo", 0);
	sdb_array_set (s, "key", 2, "lol", 0);
	sdb_array_set (s, "key", 1, "wow", 0);
	EXPECT("foo,wow,lol")

TEST(set4)
	sdb_unset (s, "key", 0);
	sdb_array_set (s, "key", 0, "foo", 0);
	sdb_array_set (s, "key", 3, "lol", 0);
	sdb_array_set (s, "key", 1, "wow", 0);
	sdb_array_set (s, "key", 2, "two", 0);
	EXPECT("foo,wow,two,lol");

TEST(set5)
	sdb_set (s, "key", "foo,bar", 0);
	sdb_array_set (s, "key", 0, "lol", 0);
	EXPECT("lol,bar")

TEST(setneg)
	sdb_set (s, "key", "foo,bar", 0);
	sdb_array_set (s, "key", -1, "lol", 0);
	EXPECT("foo,bar,lol")

TEST(unset)
	sdb_set (s, "key", "foo,bar", 0);
	sdb_array_unset (s, "key", 1, 0);
	EXPECT("foo,")

TEST(unset2)
	sdb_set (s, "key", "foo,bar,cow", 0);
	sdb_array_unset (s, "key", 1, 0);
	EXPECT("foo,,cow")

TEST(unset3)
	sdb_set (s, "key", "foo,bar,cow", 0);
	sdb_array_unset (s, "key", 1, 0);
	sdb_array_unset (s, "key", 2, 0);
	EXPECT("foo,,")

TEST(delete)
	sdb_set (s, "key", "foo,bar,cow", 0);
	sdb_array_delete (s, "key", 1, 0);
	EXPECT("foo,cow")

TEST(delete2)
	sdb_set (s, "key", "foo,bar,cow", 0);
	sdb_array_delete (s, "key", 0, 0);
	EXPECT("bar,cow")

// push prepends
TEST(push)
	sdb_set (s, "key", "foo,bar", 0);
	sdb_array_push (s, "key", "cow", 0);
	EXPECT("cow,foo,bar")

TEST(push2)
	sdb_set (s, "key", ",foo,bar,", 0);
	sdb_array_push (s, "key", "cow", 0);
	EXPECT("cow,,foo,bar,")

TEST(pop)
	sdb_set (s, "key", "foo,bar", 0);
	free (sdb_array_pop (s, "key", 0));
	EXPECT("bar")

TEST(pop2)
	sdb_set (s, "key", ",foo,bar", 0);
	free (sdb_array_pop (s, "key", 0));
	EXPECT("foo,bar")

TEST(num)
	sdb_set (s, "key", "1,2", 0);
	sdb_array_set_num (s, "key", 1, 123, 0);
	EXPECT("1,0x7b")

TEST(num2)
	sdb_set (s, "key", "1,2", 0);
	sdb_array_set_num (s, "key", 3, 123, 0);
	EXPECT("1,2,,0x7b")

TEST(num3)
	sdb_set (s, "key", "1,2,", 0);
	sdb_array_set_num (s, "key", 1, 123, 0);
	sdb_array_set_num (s, "key", 1, 123, 0);
	EXPECT("1,0x7b,")

TEST(add)
	sdb_set (s, "key", "foo,bar",0);
	sdb_array_add (s, "key", "foo", 0);
	EXPECT("foo,bar");

TEST(add2)
	sdb_set (s, "key", "foo,bar",0);
	sdb_array_add (s, "key", "cow", 0);
	EXPECT("foo,bar,cow");

TEST(add_num)
	sdb_set (s, "key", "1,2",0);
	sdb_array_add_num (s, "key", 1, 0);
	EXPECT("1,2");

TEST(add_num2)
	sdb_set (s, "key", "0x1,0x2",0);
	sdb_array_add_num (s, "key", 1, 0);
	EXPECT("0x1,0x2");

TEST(add_num3)
	sdb_set (s, "key", "0x1,0x2",0);
	sdb_array_add_num (s, "key", 1, 0);
	EXPECT("0x1,0x2");

TEST(add_num4)
	sdb_set (s, "key", "0x1,0x2",0);
	sdb_array_add_num (s, "key", 3, 0);
	EXPECT("0x1,0x2,3");

TEST(add_num5)
	sdb_set (s, "key", "1,2",0);
	sdb_array_add_num (s, "key", 3, 0);
	EXPECT("0x1,0x2,0x3");

static TestFcn *tests[] = {
	test_array_get,
	test_array_get2,
	test_array_getneg,
	test_array_ins,
	test_array_ins2,
	test_array_ins3,
	test_array_ins4,
	test_array_insneg,
	test_array_set,
	test_array_set2,
	test_array_set3,
	test_array_set4,
	test_array_set5,
	test_array_setneg,
	test_array_unset,
	test_array_unset2,
	test_array_unset3,
	test_array_delete,
	test_array_delete2,
	test_array_push,
	test_array_push2,
	test_array_pop,
	test_array_pop2,
	test_array_num,
	test_array_num2,
	test_array_num3,
	test_array_add,
	test_array_add2,
	test_array_add_num,
	test_array_add_num2,
	test_array_add_num3,
	test_array_add_num4,
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
