#include "minunit.h"
#include <sdb/sdb.h>
#include <sdb/ht_uu.h>
#include <sdb/ht_up.h>
#include <sdb/ht_pp.h>

static bool test_hash64_vectors(void) {
	// Generated with upstream rapidhashMicro V3, independently of this port.
	static const struct {
		size_t len;
		ut64 hash[2];
	} vectors[] = {
		{ 0, { 0x0338dc4be2cecdaeULL, 0x565ef32cd8efb3ddULL } },
		{ 1, { 0xc1efc21089b01264ULL, 0xf067a466be7b848aULL } },
		{ 2, { 0x9fbbe7bc8f19c3b9ULL, 0x3b06d39d1fe719daULL } },
		{ 3, { 0xb6f7e44c9fadfc9aULL, 0xdf6efeb9fdbe15c6ULL } },
		{ 4, { 0x0d1baf46a62cf90eULL, 0xfb51257d2af9e430ULL } },
		{ 5, { 0x90231a1034acd884ULL, 0xeeff467d3c4f776dULL } },
		{ 6, { 0x9a2ebe83b7705be2ULL, 0x2c7642f4f9d1a688ULL } },
		{ 7, { 0xe7e47b5c33e809c5ULL, 0xbf24b2879efaf832ULL } },
		{ 8, { 0x2764b1897f0a2bbfULL, 0x4600d87336ea7096ULL } },
		{ 9, { 0x9997f8bc07ba84bcULL, 0x368c77eba2bc02acULL } },
		{ 10, { 0xe25849204219a22dULL, 0x1fb5d3bc5625ab5aULL } },
		{ 11, { 0xb1b449a092040d50ULL, 0xf2518b8d6988457eULL } },
		{ 12, { 0x5a68b62992dd53fcULL, 0x49c97e574a9bde43ULL } },
		{ 13, { 0x538d9c3fbc60fc9fULL, 0x5f17152fd16445abULL } },
		{ 14, { 0xf4d32a6ffa4e70aaULL, 0x350632959104df9bULL } },
		{ 15, { 0x058515efa8cd9ec1ULL, 0x6cf5911b77496fa9ULL } },
		{ 16, { 0x5ad285bf7f0f7905ULL, 0x9dbf010d4acdd91aULL } },
		{ 17, { 0xaa31e2c991f6f4f1ULL, 0xd4c00015bb1f84f5ULL } },
		{ 31, { 0x83ae4336d933bf5fULL, 0x85d94fe0b7d9f151ULL } },
		{ 32, { 0x9774b9294c5502a5ULL, 0xd52853796a85d697ULL } },
		{ 33, { 0x3973ba1f89b4033eULL, 0x62914f47458ac041ULL } },
		{ 47, { 0x45977a6b74720f65ULL, 0xf2b1f7ecd639ba39ULL } },
		{ 48, { 0x515d285ee1237d2fULL, 0xcee75151039dfcebULL } },
		{ 49, { 0xa243884ecedbee1eULL, 0xb358c9d8d93c7a40ULL } },
		{ 63, { 0x71cfe2c805711ff6ULL, 0x2e08b1869dbe6d71ULL } },
		{ 64, { 0xe8139e9946962d36ULL, 0x25e00a183f6afc8dULL } },
		{ 65, { 0x158f1304b70ed6d0ULL, 0xbe728d20558dd217ULL } },
		{ 79, { 0x2b5745c59a8d7646ULL, 0xf53091decadb6639ULL } },
		{ 80, { 0xa8e6ddc9379b380fULL, 0x97275c4201b79fa4ULL } },
		{ 81, { 0x7c8833bad51f1231ULL, 0x003972f2b86b8955ULL } },
		{ 159, { 0x3385182f5c2544edULL, 0x925d1b955fd3e6d4ULL } },
		{ 160, { 0x72bcc8cc28eccae7ULL, 0xf1d4ba1d905d0733ULL } },
		{ 161, { 0x8253ba8f406d560cULL, 0x76351cffe8b9903eULL } },
		{ 254, { 0x46ecc891a2aa8ed9ULL, 0x4669ce164e3d12d6ULL } },
		{ 255, { 0xa158da35bcafea40ULL, 0x598f5aedef0ad30dULL } },
		{ 256, { 0x6cb9afd470e82629ULL, 0x32a09be5eb274a45ULL } },
		{ 1024, { 0x0dd699e47478cfa6ULL, 0x52d421433ec278c6ULL } },
	};
	char buffer[1024 + 7];
	for (size_t offset = 0; offset < 8; offset++) {
		for (size_t i = 0; i < 1024; i++) {
			buffer[offset + i] = (char)(ut8)(i * 37 + 128);
		}
		for (size_t i = 0; i < sizeof (vectors) / sizeof (*vectors); i++) {
			mu_assert_eq (sdb_hash64_seed (buffer + offset, vectors[i].len, 0),
				vectors[i].hash[0], "default hash must match upstream on every host");
			mu_assert_eq (sdb_hash64_seed (buffer + offset, vectors[i].len, 0x0123456789abcdefULL),
				vectors[i].hash[1], "seeded hash must match upstream on every host");
		}
	}
	mu_assert_eq (sdb_hash64_seed (NULL, 0, 0), vectors[0].hash[0], "empty buffer hash");
	mu_end;
}

static bool test_hash_strings(void) {
	static const struct {
		const char *key;
		ut32 hash;
	} vectors[] = {
		{ "", 0xe2cecdae },
		{ "a", 0x33a2e1eb },
		{ "sdb", 0xa395943d },
		{ "abcdefgh", 0x2a29f41f },
		{ "abcdefghi", 0xca96c1c6 },
		{ "\x80\xff", 0xa7548475 }
	};
	for (size_t i = 0; i < sizeof (vectors) / sizeof (*vectors); i++) {
		ut32 len = UT32_MAX;
		mu_assert_eq (sdb_hash (vectors[i].key), vectors[i].hash, "string hash");
		mu_assert_eq (sdb_hash_len (vectors[i].key, &len), vectors[i].hash, "hash with length");
		mu_assert_eq (len, strlen (vectors[i].key), "string length");
		ut32 h = vectors[i].hash;
		mu_assert_eq (sdb_hash_byte (vectors[i].key),
			(h ^ (h >> 8) ^ (h >> 16) ^ (h >> 24)) & 0xff, "folded hash byte");
	}
	ut32 len = UT32_MAX;
	mu_assert_eq (sdb_hash (NULL), 0, "null hash");
	mu_assert_eq (sdb_hash_len (NULL, &len), 0, "null hash with length");
	mu_assert_eq (len, 0, "null length");
	mu_end;
}

typedef struct _test_struct {
	char *name;
	int age;
} Person;

bool test_ht_insert_lookup(void) {
	HtPP *ht = sdb_ht_new ();
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
	HtPP *ht = sdb_ht_new ();
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
	HtPP *ht = sdb_ht_new ();
	mu_assert ("nothing should be deleted", !sdb_ht_delete (ht, "non existing"));

	sdb_ht_insert (ht, "AAAA", "vAAAA");
	mu_assert ("AAAA should be deleted", sdb_ht_delete (ht, "AAAA"));
	mu_assert ("AAAA still there", !sdb_ht_find (ht, "AAAA", NULL));

	sdb_ht_free (ht);
	mu_end;
}

bool test_ht_insert_kvp(void) {
	HtPP *ht = sdb_ht_new ();
	SdbKv *kv = sdbkv_new ("AAAA", "vAAAA");
	mu_assert ("AAAA shouldn't exist", !sdb_ht_find_kvp (ht, "AAAA", NULL));
	sdb_ht_insert_kvp (ht, kv, false);
	free (kv);

	mu_assert ("AAAA should exist", sdb_ht_find_kvp (ht, "AAAA", NULL));
	SdbKv *kv2 = sdbkv_new ("AAAA", "vNEWAAAA");
	mu_assert ("AAAA shouldn't be replaced", !sdb_ht_insert_kvp (ht, kv2, false));
	mu_assert ("AAAA should be replaced", sdb_ht_insert_kvp (ht, kv2, true));
	free (kv2);

	SdbKv *foundkv = sdb_ht_find_kvp (ht, "AAAA", NULL);
	mu_assert_streq (foundkv->base.value, "vNEWAAAA", "vNEWAAAA should be there");

	sdb_ht_free (ht);
	mu_end;
}

ut32 create_collision(const void *key) {
	return 10;
}

bool test_ht_insert_collision(void) {
	HtPP *ht = sdb_ht_new ();
	ht->opt.hashfn = create_collision;
	ht_pp_insert (ht, "AAAA", "vAAAA");
	mu_assert_streq (sdb_ht_find (ht, "AAAA", NULL), "vAAAA", "AAAA should be there");
	ht_pp_insert (ht, "BBBB", "vBBBB");
	mu_assert_streq (sdb_ht_find (ht, "AAAA", NULL), "vAAAA", "AAAA should still be there");
	mu_assert_streq (sdb_ht_find (ht, "BBBB", NULL), "vBBBB", "BBBB should be there");
	ht_pp_insert (ht, "CCCC", "vBBBB");
	mu_assert_streq (sdb_ht_find (ht, "CCCC", NULL), "vBBBB", "CCCC should be there");

	sdb_ht_free (ht);
	mu_end;
}

ut32 key2hash(const void *key) {
	return atoi(key);
}

bool test_ht_grow(void) {
	HtPP *ht = sdb_ht_new ();
	char str[15], vstr[15];
	char buf[100];
	int i;

	ht->opt.hashfn = key2hash;
	for (i = 0; i < 20000; i++) {
		snprintf (str, 15, "%d", i);
		snprintf (vstr, 15, "v%d", i);
		sdb_ht_insert (ht, str, vstr);
	}

	for (i = 0; i < 20000; i++) {
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
	HtPP *ht = sdb_ht_new ();
	SdbKv *kvp = sdbkv_new ("AAAA", "vAAAA");

	mu_assert_eq (kvp->base.key_len, 4, "key_len should be 4");
	mu_assert_eq (kvp->base.value_len, 5, "value_len should be 5");
	mu_assert ("kvp should be inserted", sdb_ht_insert_kvp (ht, kvp, false));
	free (kvp);

	kvp = sdb_ht_find_kvp (ht, "AAAA", NULL);
	mu_assert_eq (kvp->base.key_len, 4, "key_len should be 4 after kvp_insert");
	mu_assert_eq (kvp->base.value_len, 5, "value_len should be 5 after kvp_insert");

	sdb_ht_insert (ht, "BBBB", "vBBBB");
	kvp = sdb_ht_find_kvp (ht, "BBBB", NULL);
	mu_assert_eq (kvp->base.key_len, 4, "key_len should be 4 after insert");
	mu_assert_eq (kvp->base.value_len, 5, "value_len should be 5 after insert");

	sdb_ht_free (ht);
	mu_end;
}

Person* duplicate_person(Person *p) {
	Person* c = malloc (sizeof (Person));
	c->name = strdup (p->name);
	c->age = p->age;
	return c;
}

void free_kv(HtPPKv *kv) {
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
	int retval = MU_PASSED;
	bool found = false;
	Person *p, *person1 = malloc (sizeof (Person));
	if (!person1) {
		mu_cleanup_fail(err_malloc, "person1 malloc");
	}
	person1->name = strdup ("radare");
	person1->age = 10;

	Person *person2 = malloc (sizeof (Person));
	if (!person2) {
		mu_cleanup_fail(err_free_person1, "person2 malloc");
	}
	person2->name = strdup ("pancake");
	person2->age = 9000;

	HtPP *ht = ht_pp_new ((HtPPDupValue)duplicate_person, free_kv, (HtPPCalcSizeV)calcSizePerson);
	if (!ht) {
		mu_cleanup_fail(err_free_persons, "ht alloc");
	}
	ht_pp_insert (ht, "radare", (void *)person1);
	ht_pp_insert (ht, "pancake", (void *)person2);
	p = ht_pp_find (ht, "radare", &found);
	mu_assert ("radare not found", found);
	mu_assert_streq (p->name, "radare", "wrong person");
	mu_assert_eq (p->age, 10, "wrong radare age");

	p = ht_pp_find (ht, "pancake", &found);
	mu_assert ("radare not found", found);
	mu_assert_streq (p->name, "pancake", "wrong person");
	mu_assert_eq (p->age, 9000, "wrong pancake age");

	(void)ht_pp_find (ht, "not", &found);
	mu_assert ("found but it should not exists", !found);

	ht_pp_delete (ht, "pancake");
	p = ht_pp_find (ht, "pancake", &found);
	mu_assert ("pancake was deleted", !found);

	ht_pp_insert (ht, "pancake", (void *)person2);
	ht_pp_delete (ht, "radare");
	ht_pp_update (ht, "pancake", (void *)person1);
	p = ht_pp_find (ht, "pancake", &found);

	mu_assert ("pancake was updated", found);
	mu_assert_streq (p->name, "radare", "wrong person");
	mu_assert_eq (p->age, 10, "wrong age");

	ht_pp_free (ht);
 err_free_persons:
	free (person2->name);
	free (person2);
 err_free_person1:
	free (person1->name);
	free (person1);
 err_malloc:
	mu_cleanup_end;
}

static void free_key_value(HtPPKv *kv) {
	free (kv->key);
	free (kv->value);
}

static HtPPKv *expected_freed_kv;
static bool received_embedded_kv;

static void free_key_value_track_kv(HtPPKv *kv) {
	if (kv == expected_freed_kv) {
		received_embedded_kv = true;
	}
	free (kv->key);
	free (kv->value);
}

bool test_freefn_receives_embedded_kv(void) {
	HtPP *ht = ht_pp_new ((HtPPDupValue)strdup, free_key_value_track_kv, NULL);
	mu_assert_notnull (ht, "ht alloc failed");
	mu_assert ("insert key1", ht_pp_insert (ht, "key1", "value1"));

	bool found = false;
	expected_freed_kv = ht_pp_find_kv (ht, "key1", &found);
	mu_assert ("key1 should exist", found);
	mu_assert_notnull (expected_freed_kv, "key1 kv missing");
	received_embedded_kv = false;

	mu_assert ("delete key1", ht_pp_delete (ht, "key1"));
	mu_assert ("freefn should receive embedded kv", received_embedded_kv);

	expected_freed_kv = NULL;
	ht_pp_free (ht);
	mu_end;
}

bool should_not_be_caled(void *user, const char *k, void *v) {
	mu_fail ("this function should not be called");
	return false;
}

bool test_empty_ht(void) {
	HtPP *ht = ht_pp_new0 ();
	ht_pp_foreach (ht, (HtPPForeachCallback) should_not_be_caled, NULL);
	void *r = ht_pp_find (ht, "key1", NULL);
	mu_assert_null (r, "key1 should not be present");
	ht_pp_free (ht);
	mu_end;
}

bool test_insert(void) {
	HtPP *ht = ht_pp_new0 ();
	void *r;
	bool res;
	bool found;

	res = ht_pp_insert (ht, "key1", "value1");
	mu_assert ("key1 should be a new element", res);
	r = ht_pp_find (ht, "key1", &found);
	mu_assert ("found should be true", found);
	mu_assert_streq (r, "value1", "value1 should be retrieved");

	res = ht_pp_insert (ht, "key1", "value2");
	mu_assert ("key1 should be an already existing element", !res);
	r = ht_pp_find (ht, "key1", &found);
	mu_assert_streq (r, "value1", "value1 should be retrieved");
	mu_assert ("found should be true", found);

	r = ht_pp_find (ht, "key2", &found);
	mu_assert_null (r, "key2 should not be present");
	mu_assert ("found for key2 should be false", !found);

	ht_pp_free (ht);
	mu_end;
}

bool test_update(void) {
	HtPP *ht = ht_pp_new0 ();
	bool found;

	ht_pp_insert (ht, "key1", "value1");
	ht_pp_update (ht, "key1", "value2");
	void *r = ht_pp_find (ht, "key1", &found);
	mu_assert_streq (r, "value2", "value2 should be retrieved");
	mu_assert ("found should be true", found);
	ht_pp_free (ht);
	mu_end;
}

bool test_delete(void) {
	HtPP *ht = ht_pp_new0 ();
	bool found;

	ht_pp_insert (ht, "key1", "value1");
	ht_pp_delete (ht, "key1");
	void *r = ht_pp_find (ht, "key1", &found);
	mu_assert_null (r, "key1 should not be found");
	mu_assert ("found should be false", !found);
	ht_pp_free (ht);
	mu_end;
}

static bool grow_1_found[3];
static bool grow_1_foreach(void *user, const char *k, int v) {
	grow_1_found[v] = true;
	return true;
}

bool test_grow_1(void) {
	HtPP *ht = ht_pp_new0 ();
	int i;

	for (i = 0; i < 3; i++) {
		grow_1_found[i] = false;
	}

	ht_pp_insert (ht, "key0", (void *)0);
	ht_pp_insert (ht, "key1", (void *)1);
	ht_pp_insert (ht, "key2", (void *)2);

	ht_pp_foreach (ht, (HtPPForeachCallback)grow_1_foreach, NULL);
	for (i = 0; i < 3; i++) {
		if (!grow_1_found[i]) {
			fprintf (stderr, "i = %d\n", i);
			mu_fail ("An element has not been traversed");
		}
	}

	ht_pp_free (ht);
	mu_end;
}

bool test_grow_2(void) {
	HtPP *ht = ht_pp_new ((HtPPDupValue)strdup, (HtPPKvFreeFunc)free_key_value, NULL);
	char *r;
	bool found;
	int i;

	for (i = 0; i < 3000; i++) {
		char buf[20], buf2[20];
		snprintf (buf, 20, "key%d", i);
		snprintf (buf2, 20, "value%d", i);
		ht_pp_insert (ht, buf, buf2);
	}

	r = ht_pp_find (ht, "key1", &found);
	mu_assert_streq (r, "value1", "value1 should be retrieved");
	mu_assert ("found should be true", found);

	r = ht_pp_find (ht, "key2000", &found);
	mu_assert_streq (r, "value2000", "value2000 should be retrieved");
	mu_assert ("found should be true", found);

	r = ht_pp_find (ht, "key4000", &found);
	mu_assert_null (r, "key4000 should not be there");
	mu_assert ("found should be false", !found);

	ht_pp_free (ht);
	mu_end;
}

bool test_grow_3(void) {
	HtPP *ht = ht_pp_new ((HtPPDupValue)strdup, (HtPPKvFreeFunc)free_key_value, NULL);
	char *r;
	bool found;
	int i;

	for (i = 0; i < 3000; i++) {
		char buf[20], buf2[20];
		snprintf (buf, 20, "key%d", i);
		snprintf (buf2, 20, "value%d", i);
		ht_pp_insert (ht, buf, buf2);
	}

	for (i = 0; i < 3000; i += 3) {
		char buf[20];
		snprintf (buf, 20, "key%d", i);
		ht_pp_delete (ht, buf);
	}

	r = ht_pp_find (ht, "key1", &found);
	mu_assert_streq (r, "value1", "value1 should be retrieved");
	mu_assert ("found should be true", found);

	r = ht_pp_find (ht, "key2000", &found);
	mu_assert_streq (r, "value2000", "value2000 should be retrieved");
	mu_assert ("found should be true", found);

	r = ht_pp_find (ht, "key4000", &found);
	mu_assert_null (r, "key4000 should not be there");
	mu_assert ("found should be false", !found);

	r = ht_pp_find (ht, "key0", &found);
	mu_assert_null (r, "key0 should not be there");
	mu_assert ("found should be false", !found);

	for (i = 1; i < 3000; i += 3) {
		char buf[20];
		snprintf (buf, 20, "key%d", i);
		ht_pp_delete (ht, buf);
	}

	r = ht_pp_find (ht, "key1", &found);
	mu_assert_null (r, "key1 should not be there");
	mu_assert ("found should be false", !found);

	ht_pp_free (ht);
	mu_end;
}

bool test_grow_4(void) {
	HtPP *ht = ht_pp_new (NULL, (HtPPKvFreeFunc)free_key_value, NULL);
	char *r;
	bool found;
	int i;

	for (i = 0; i < 3000; i++) {
		char buf[20], *buf2;
		snprintf (buf, 20, "key%d", i);
		buf2 = malloc (20);
		snprintf (buf2, 20, "value%d", i);
		ht_pp_insert (ht, buf, buf2);
	}

	r = ht_pp_find (ht, "key1", &found);
	mu_assert_streq (r, "value1", "value1 should be retrieved");
	mu_assert ("found should be true", found);

	r = ht_pp_find (ht, "key2000", &found);
	mu_assert_streq (r, "value2000", "value2000 should be retrieved");
	mu_assert ("found should be true", found);

	for (i = 0; i < 3000; i += 3) {
		char buf[20];
		snprintf (buf, 20, "key%d", i);
		ht_pp_delete (ht, buf);
	}

	r = ht_pp_find (ht, "key2000", &found);
	mu_assert_streq (r, "value2000", "value2000 should be retrieved");
	mu_assert ("found should be true", found);

	r = ht_pp_find (ht, "key0", &found);
	mu_assert_null (r, "key0 should not be there");
	mu_assert ("found should be false", !found);

	for (i = 1; i < 3000; i += 3) {
		char buf[20];
		snprintf (buf, 20, "key%d", i);
		ht_pp_delete (ht, buf);
	}

	r = ht_pp_find (ht, "key1", &found);
	mu_assert_null (r, "key1 should not be there");
	mu_assert ("found should be false", !found);

	ht_pp_free (ht);
	mu_end;
}

bool foreach_delete_cb(void *user, const ut64 key, const void *v) {
	HtUP *ht = (HtUP *)user;

	ht_up_delete (ht, key);
	return true;
}

static void free_up_value(HtUPKv *kv) {
	free (kv->value);
}

bool test_foreach_delete(void) {
	HtUP *ht = ht_up_new ((HtUPDupValue)strdup, free_up_value, NULL);

	// create a collision
	ht_up_insert (ht, 0, "value1");
	ht_up_insert (ht, ht->size, "value2");
	ht_up_insert (ht, ht->size * 2, "value3");
	ht_up_insert (ht, ht->size * 3, "value4");

	ht_up_foreach (ht, foreach_delete_cb, ht);
	ht_up_foreach (ht, (HtUPForeachCallback) should_not_be_caled, NULL);

	ht_up_free (ht);
	mu_end;
}

bool test_update_key(void) {
	bool res;
	HtUP *ht = ht_up_new ((HtUPDupValue)strdup, free_up_value, NULL);

	// create a collision
	ht_up_insert (ht, 0, "value1");
	ht_up_insert (ht, 0xdeadbeef, "value2");
	ht_up_insert (ht, 0xcafebabe, "value3");

	res = ht_up_update_key (ht, 0xcafebabe, 0x10000);
	mu_assert ("cafebabe should be updated", res);
	res = ht_up_update_key (ht, 0xdeadbeef, 0x10000);
	mu_assert ("deadbeef should NOT be updated, because there's already an element at 0x10000", !res);

	const char *v = ht_up_find (ht, 0x10000, NULL);
	mu_assert_streq (v, "value3", "value3 should be at 0x10000");
	v = ht_up_find (ht, 0xdeadbeef, NULL);
	mu_assert_streq (v, "value2", "value2 should remain at 0xdeadbeef");

	ht_up_free (ht);
	mu_end;
}

int all_tests() {
	mu_run_test (test_hash64_vectors);
	mu_run_test (test_hash_strings);
	mu_run_test (test_ht_insert_lookup);
	mu_run_test (test_ht_update_lookup);
	mu_run_test (test_ht_delete);
	mu_run_test (test_ht_insert_kvp);
	mu_run_test (test_ht_insert_collision);
	mu_run_test (test_ht_grow);
	mu_run_test (test_ht_kvp);
	mu_run_test (test_ht_general);
	mu_run_test (test_freefn_receives_embedded_kv);
	mu_run_test (test_empty_ht);
	mu_run_test (test_insert);
	mu_run_test (test_update);
	mu_run_test (test_delete);
	mu_run_test (test_grow_1);
	mu_run_test (test_grow_2);
	mu_run_test (test_grow_3);
	mu_run_test (test_grow_4);
	mu_run_test (test_foreach_delete);
	mu_run_test (test_update_key);
	return tests_passed != tests_run;
}

int main(int argc, char **argv) {
	return all_tests ();
}
