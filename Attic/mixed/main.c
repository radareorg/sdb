/* radare - LGPL - Copyright 2009-2018 pancake */

#include <r_util.h>
#include "./r_mixed.h"

typedef struct {
	char *name;
	ut32 hashname;
	int length;
	ut64 offset;
} TestStruct;

TestStruct *test_struct_new(const char *name, int length, ut64 offset) {
	TestStruct *ts = R_NEW (TestStruct);
	ts->name = strdup (name);
	ts->hashname = r_str_hash (name);
	ts->length = length;
	ts->offset = offset;
	return ts;
}

void test_struct_free(TestStruct *ts) {
	free (ts->name);
	free (ts);
}

int main () {
	RList *list;
	RListIter *iter;
	TestStruct *ts, *ts2;
	RMixed *mx = r_mixed_new ();
	R_MIXED_KEY (mx, TestStruct, ts, hashname);
	R_MIXED_KEY (mx, TestStruct, ts, length);
	R_MIXED_KEY (mx, TestStruct, ts, offset);

	ts2 = test_struct_new ("FOOD", 444, 0x123456789);
	r_mixed_add (mx, ts2);

	r_mixed_add (mx, test_struct_new ("food", 12, 0x839481222000));
	r_mixed_add (mx, test_struct_new ("food", 12, 0x839481222000));
	r_mixed_add (mx, test_struct_new ("baar", 12, 0x441242910));
	r_mixed_add (mx, test_struct_new ("cocktel", 12, 0x224944));
	r_mixed_add (mx, test_struct_new ("cocktel2", 16, 0x224944));
	r_mixed_add (mx, test_struct_new ("cocktel3", 17, 0x224944));

	ts = r_mixed_get0 (mx, r_offsetof (TestStruct, hashname), (ut64)r_str_hash ("food"));
	if (ts) {
		printf ("NAM: %s\n", ts->name);
		printf ("LEN: %d\n", ts->length);
		printf ("OFF: %llx\n", ts->offset);
	} else eprintf ("oops. cannot find 'food'\n");

	eprintf ("--\n");
	list = r_mixed_get (mx, r_offsetof (TestStruct, offset), 0x224944);
	r_list_foreach (list, iter, ts) {
		printf ("NAM: %s\n", ts->name);
		printf ("LEN: %d\n", ts->length);
		printf ("OFF: %llx\n", ts->offset);
	}

	r_mixed_change_begin (mx, ts2);
	ts2->length = 666;
	if (!r_mixed_change_end (mx, ts2)) {
		eprintf ("MixedChange failed\n");
	}
	r_mixed_free (mx);
}
