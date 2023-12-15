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
	for (int i = 0; i < (int)_countof(numbers); ++i)
		array_push(&array, &numbers[i]);

	array_insert_at(&array, 2, &(int){42});

	array_remove_at(&array, 3);
	array_remove_at(&array, 0);
	array_remove_at(&array, -1);

	int expected[] = { 1, 42, 3, 4, 5, 6 };
	for (int i = 0; i < (int)_countof(expected); ++i)
		EXPECT_EQ(expected[i], array_get(int, &array, i));

	array_free(&array);
}

UTEST(dsArray, Insert_At)
{
	dsArray array = array_new(int);

	array_insert_at(&array, 0, &(int){0});           // start
	array_insert_at(&array, array.count, &(int){3}); // end
	array_insert_at(&array, 1, &(int){1});           // middle
	array_insert_at(&array, 2, &(int){2});           // middle

	int expected[] = { 0, 1, 2, 3 };
	for (int i = 0; i < (int)_countof(expected); ++i)
		EXPECT_EQ(expected[i], array_get(int, &array, i));

	array_free(&array);
}

UTEST(dsArray, Remove_At)
{
	dsArray array = array_new(int);
	int numbers[] = { 0, 1, 2, 3, 4, 5, 6 };
	for (int i = 0; i < (int)_countof(numbers); ++i)
		array_push(&array, &numbers[i]);

	array_remove_at(&array, 6); // end
	array_remove_at(&array, 0); // start
	array_remove_at(&array, 3); // middle

	int expected[] = { 1, 2, 3, 5 };
	for (int i = 0; i < (int)_countof(expected); ++i)
		EXPECT_EQ(expected[i], array_get(int, &array, i));

	array_free(&array);
}

UTEST(dsArray, Grow)
{
	dsArray array = array_new_count(int, 2);
	array_set(&array, 0, &(int){0});
	array_set(&array, 1, &(int){1});
	array_grow(&array, 4);
	array_set(&array, 2, &(int){2});
	array_set(&array, 3, &(int){3});
	int expected[] = { 0, 1, 2, 3 };
	for (int i = 0; i < (int)_countof(expected); ++i)
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
	return hash_murmur3(kv.key, (int)strlen(kv.key));
}

UTEST(dsSet, Overview)
{
	dsSet set = set_new_hash(KV, hash_KV);
	KV map[] = {
		{"My", 0},
		{"name", 1},
		{"is", 2},
		{"Slim", 3},
		{"Shady", 4},
		{"Marshall", 5},
		{"Mathers", 6},
		{"M'M", 7},
		{"Eminem", 8},
	};
	for (int i = 0; i < (int)_countof(map); ++i)
		set_insert(&set, &map[i]);

	for (KV* it = set_begin(&set); it != set_end(&set); ++it)
		EXPECT_STREQ(it->key, map[it->value].key);

	EXPECT_TRUE(set_lookup(&set, &(KV){"Eminem"}));

	set_remove(&set, &map[5]);
	EXPECT_FALSE(set_lookup(&set, &(KV){"Marshall"}));

	set_free(&set);
}