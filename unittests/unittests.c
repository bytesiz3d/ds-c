#define DSALLOCATOR_IMPLEMENTATION
#include <dsAllocator.h>

#define DSARRAY_IMPLEMENTATION
#include <dsArray.h>

#define DSSET_IMPLEMENTATION
#include <dsSet.h>

#include "utest.h"
UTEST_MAIN()

UTEST(dsArray, Overview)
{
	dsArray array = array_new(int);

	int numbers[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	for (int i = 0; i < _countof(numbers); ++i)
		array_push(&array, &numbers[i]);

	int insert = 42;
	array_insert_at(&array, 2, &insert);

	array_remove_at(&array, 3);
	array_remove_at(&array, 0);
	array_remove_at(&array, -1);

	int expected[] = { 1, 42, 3, 4, 5, 6 };
	for (int i = 0; i < _countof(expected); ++i)
		EXPECT_EQ(expected[i], array_get(int, &array, i));

	array_free(&array);
}

typedef struct KV
{
	char key[16];
	int value;
} KV;

uint32_t
hash_KV(void* base, int SIZE)
{
	(void)SIZE;
	KV kv = *(KV*)base;
	return murmur3_32(kv.key, (int)strlen(kv.key));
}

UTEST(dsSet, Overview)
{
	dsSet set = set_new_hash(KV, hash_KV);
	KV map[] = {
		(KV){"my", 0},
		(KV){"name", 1},
		(KV){"is", 2},
		(KV){"slim", 3},
		(KV){"shady", 4},
		(KV){"Marshall", 5},
		(KV){"Mathers", 6},
		(KV){"m'm", 7},
		(KV){"Eminem", 8},
	};
	for (int i = 0; i < _countof(map); ++i)
		set_insert(&set, &map[i]);

	for (KV* it = set_begin(&set); it != set_end(&set); ++it)
		EXPECT_STREQ(it->key, map[it->value].key);

	EXPECT_TRUE(set_lookup(&set, &(KV){"Eminem"}));

	set_remove(&set, &map[5]);
	EXPECT_FALSE(set_lookup(&set, &(KV){"Marshall"}));

	set_free(&set);
}