#include "utest.h"
#define COUNTOF(x) ((int)(sizeof(x) / sizeof(x[0])))

#include <ds_set.h>

typedef struct KV
{
	char key[16];
	int value;
} KV;

uint32_t
hash_KV(const void* base, int SIZE)
{
	(void)SIZE;
	char* key = ((KV*)base)->key;
	return ds_hash_murmur3(key, (int)strlen(key));
}

UTEST(ds_set, Overview)
{
	ds_set set = ds_set_new_hash_fn(KV, hash_KV);
	KV map[] = {
		{"My",       0},
		{"name",     1},
		{"is",       2},
		{"Slim",     3},
		{"Shady",    4},
		{"Marshall", 5},
		{"Mathers",  6},
		{"M'M",      7},
		{"Eminem",   8},
	};
	for (int i = 0; i < COUNTOF(map); ++i)
		ds_set_insert(&set, &map[i]);

	for (KV* it = ds_set_begin(&set); it != ds_set_end(&set); ++it)
		EXPECT_STREQ(it->key, map[it->value].key);

	EXPECT_TRUE(ds_set_lookup(&set, &(KV){"Eminem"}));

	ds_set_remove(&set, &map[5]);
	EXPECT_FALSE(ds_set_lookup(&set, &(KV){"Marshall"}));

	ds_set_free(&set);
	ds_set_free(&set); // double free
}

UTEST(ds_set, Duplicate_Key)
{
	ds_set set = ds_set_new_hash_fn(KV, hash_KV);
	KV map[] = {
		{"My",       0},
		{"name",     1},
		{"is",       2},
		{"Slim",     3},
		{"Shady",    4},
		{"Marshall", 5},
		{"Mathers",  6},
		{"M'M",      7},
		{"Eminem",   8},
	};
	for (int i = 0; i < COUNTOF(map); ++i)
		ds_set_insert(&set, &map[i]);

	KV* eminem = (KV*)ds_set_lookup(&set, &(KV){"Eminem"});
	EXPECT_EQ(eminem->value, 8);

	eminem = ds_set_insert(&set, &(KV){"Eminem", 9});
	EXPECT_EQ(eminem->value, 9);

	ds_set_free(&set);
}