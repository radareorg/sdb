#include "minunit.h"
#include <sdb.h>

typedef struct _test_struct {
	char *name;
	int age;
} Person;

bool test_ht_insert_lookup(void) {
	SdbHash *ht = sdb_ht_new ();
	sdb_ht_insert (ht, "AAAA", "vAAAA");
	sdb_ht_insert (ht, "BBBB", "vBBBB");
	sdb_ht_insert (ht, "CCCC", "vCCCC");

	mu_assert_streq (sdb_ht_find (ht, "BBBB", NULL), "vBBBB", "BBBB value wrong");
	mu_assert_streq (sdb_ht_find (ht, "AAAA", NULL), "vAAAA", "AAAA value wrong");
	mu_assert_streq (sdb_ht_find (ht, "CCCC", NULL), "vCCCC", "CCCC value wrong");

	sdb_ht_free (ht);
	mu_end;
}

bool test_ht_update_lookup(void) {
	SdbHash *ht = sdb_ht_new ();
	sdb_ht_insert (ht, "AAAA", "vAAAA");
	sdb_ht_insert (ht, "BBBB", "vBBBB");

	// test update to add a new element
	sdb_ht_update (ht, "CCCC", "vCCCC");
	mu_assert_streq (sdb_ht_find (ht, "CCCC", NULL), "vCCCC", "CCCC value wrong");

	// test update to replace an existing element
	sdb_ht_update (ht, "AAAA", "vDDDD");
	mu_assert_streq (sdb_ht_find (ht, "AAAA", NULL), "vDDDD", "DDDD value wrong");

	sdb_ht_free (ht);
	mu_end;
}

bool test_ht_delete(void) {
	SdbHash *ht = sdb_ht_new ();
	mu_assert ("nothing should be deleted", !sdb_ht_delete (ht, "non existing"));

	sdb_ht_insert (ht, "AAAA", "vAAAA");
	mu_assert ("AAAA should be deleted", sdb_ht_delete (ht, "AAAA"));
	mu_assert ("AAAA still there", !sdb_ht_find (ht, "AAAA", NULL));

	sdb_ht_free (ht);
	mu_end;
}

bool test_ht_insert_kvp(void) {
	SdbHash *ht = sdb_ht_new ();
	SdbKv *kv = sdb_kv_new ("AAAA", "vAAAA");
	mu_assert ("AAAA shouldn't exist", !sdb_ht_find_kvp (ht, "AAAA", NULL));
	sdb_ht_insert_kvp (ht, kv, false);
	mu_assert ("AAAA should exist", sdb_ht_find_kvp (ht, "AAAA", NULL));
	SdbKv *kv2 = sdb_kv_new ("AAAA", "vNEWAAAA");
	mu_assert ("AAAA shouldn't be replaced", !sdb_ht_insert_kvp (ht, kv2, false));
	mu_assert ("AAAA should be replaced", sdb_ht_insert_kvp (ht, kv2, true));

	SdbKv *foundkv = sdb_ht_find_kvp (ht, "AAAA", NULL);
	mu_assert_streq (foundkv->value, "vNEWAAAA", "vNEWAAAA should be there");

	ht_free (ht);
	mu_end;
}

ut32 create_collision(const char *key) {
	return 10;
}

bool test_ht_insert_collision(void) {
	SdbHash *ht = sdb_ht_new ();
	ht->hashfn = create_collision;
	ht_insert (ht, "AAAA", "vAAAA");
	mu_assert_streq (sdb_ht_find (ht, "AAAA", NULL), "vAAAA", "AAAA should be there");
	ht_insert (ht, "BBBB", "vBBBB");
	mu_assert_streq (sdb_ht_find (ht, "AAAA", NULL), "vAAAA", "AAAA should still be there");
	mu_assert_streq (sdb_ht_find (ht, "BBBB", NULL), "vBBBB", "BBBB should be there");
	ht_insert (ht, "CCCC", "vBBBB");
	mu_assert_streq (sdb_ht_find (ht, "CCCC", NULL), "vBBBB", "CCCC should be there");

	sdb_ht_free (ht);
	mu_end;
}

ut32 key2hash(const char *key) {
	return atoi(key);
}

bool test_ht_grow(void) {
	SdbHash *ht = sdb_ht_new ();
	char str[15], vstr[15];
	char buf[100];
	int i;

	ht->hashfn = key2hash;
	for (i = 0; i < 20000; ++i) {
		snprintf (str, 15, "%d", i);
		snprintf (vstr, 15, "v%d", i);
		sdb_ht_insert (ht, str, vstr);
	}

	for (i = 0; i < 20000; ++i) {
		snprintf (str, 15, "%d", i);
		snprintf (vstr, 15, "v%d", i);
		char *v = sdb_ht_find (ht, str, NULL);
		snprintf (buf, 100, "%s/%s should be there", str, vstr);
		mu_assert (buf, v);
		snprintf (buf, 100, "%s/%s should be right", str, vstr);
		mu_assert_streq (v, vstr, buf);
	}

	sdb_ht_free (ht);
	mu_end;
}

bool test_ht_kvp(void) {
	SdbHash *ht = sdb_ht_new ();
	SdbKv *kvp = sdb_kv_new ("AAAA", "vAAAA");

	mu_assert_eq (kvp->key_len, 4, "key_len should be 4");
	mu_assert_eq (kvp->value_len, 5, "value_len should be 5");
	mu_assert ("kvp should be inserted", sdb_ht_insert_kvp (ht, kvp, false));
	kvp = sdb_ht_find_kvp (ht, "AAAA", NULL);
	mu_assert_eq (kvp->key_len, 4, "key_len should be 4 after kvp_insert");
	mu_assert_eq (kvp->value_len, 5, "value_len should be 5 after kvp_insert");

	sdb_ht_insert (ht, "BBBB", "vBBBB");
	kvp = sdb_ht_find_kvp (ht, "BBBB", NULL);
	mu_assert_eq (kvp->key_len, 4, "key_len should be 4 after insert");
	mu_assert_eq (kvp->value_len, 5, "value_len should be 5 after insert");

	sdb_ht_free (ht);
	mu_end;
}

Person* duplicate_person(Person *p) {
	Person* c = malloc (sizeof (Person));
	c->name = strdup (p->name);
	c->age = p->age;
	return c;
}

void free_kv(HtKv *kv) {
	free (kv->key);
	Person *p = kv->value;
	free (p->name);
	free (p);
}

size_t calcSizePerson(void *c) {
	Person *p = c;
	return sizeof (*p);
}
bool test_ht_general(void) {
	bool found = false;
	Person *p, *person1 = malloc (sizeof (Person));
	if (!person1) {
		mu_end;
	}
	person1->name = strdup ("radare");
	person1->age = 10;

	Person *person2 = malloc (sizeof (Person));
	if (!person2) {
		mu_end;
	}
	person2->name = strdup ("pancake");
	person2->age = 9000;

	SdbHash *ht = ht_new ((DupValue)duplicate_person, free_kv, 
	  			(CalcSize)calcSizePerson);
	if (!ht) {
		mu_end;
	}
	ht_insert (ht, "radare", (void *)person1);
	ht_insert (ht, "pancake", (void *)person2);
	p = ht_find (ht, "radare", &found);
	mu_assert ("radare not found", found);
	mu_assert_streq (p->name, "radare", "wrong person");
	mu_assert_eq (p->age, 10, "wrong radare age");

	p = ht_find (ht, "pancake", &found);
	mu_assert ("radare not found", found);
	mu_assert_streq (p->name, "pancake", "wrong person");
	mu_assert_eq (p->age, 9000, "wrong pancake age");

	(void)ht_find (ht, "not", &found);
	mu_assert ("found but it should not exists", !found);

	ht_delete (ht, "pancake");
	p = ht_find (ht, "pancake", &found);
	mu_assert ("pancake was deleted", !found);

	ht_insert (ht, "pancake", (void *)person2);
	ht_delete (ht, "radare");
	ht_update (ht, "pancake", (void *)person1);
	p = ht_find (ht, "pancake", &found);

	mu_assert ("pancake was updated", found);
	mu_assert_streq (p->name, "radare", "wrong person");
	mu_assert_eq (p->age, 10, "wrong age");

	ht_free (ht);
	mu_end;
}

int all_tests() {
	mu_run_test (test_ht_insert_lookup);
	mu_run_test (test_ht_update_lookup);
	mu_run_test (test_ht_delete);
	mu_run_test (test_ht_insert_kvp);
	mu_run_test (test_ht_insert_collision);
	mu_run_test (test_ht_grow);
	mu_run_test (test_ht_kvp);
	mu_run_test (test_ht_general);
	return tests_passed != tests_run;
}

int main(int argc, char **argv) {
	return all_tests ();
}
