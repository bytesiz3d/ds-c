#include "utest.h"
#define COUNTOF(x) ((int)(sizeof(x) / sizeof(x[0])))

#include <ds_array.h>

UTEST(ds_array, Overview)
{
	ds_array array = ds_array_new(int);

	int numbers[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	for (int i = 0; i < COUNTOF(numbers); ++i)
		ds_array_push(&array, &numbers[i]);

	ds_array_insert_at(&array, 2, &(int){42});

	ds_array_remove_at(&array, 3);
	ds_array_remove_at(&array, 0);
	ds_array_remove_at(&array, -1);

	int expected[] = { 1, 42, 3, 4, 5, 6 };
	for (int i = 0; i < COUNTOF(expected); ++i)
		EXPECT_EQ(expected[i], ds_array_get(int, &array, i));

	ds_array_free(&array);
	ds_array_free(&array); // double free
}

UTEST(ds_array, Insert_At)
{
	ds_array array = ds_array_new(int);

	ds_array_insert_at(&array, 0, &(int){0});           // start
	ds_array_insert_at(&array, array.count, &(int){3}); // end
	ds_array_insert_at(&array, 1, &(int){1});           // middle
	ds_array_insert_at(&array, 2, &(int){2});           // middle

	int expected[] = { 0, 1, 2, 3 };
	for (int i = 0; i < COUNTOF(expected); ++i)
		EXPECT_EQ(expected[i], ds_array_get(int, &array, i));

	ds_array_free(&array);
}

UTEST(ds_array, Remove_At)
{
	ds_array array = ds_array_new(int);
	int numbers[] = { 0, 1, 2, 3, 4, 5, 6 };
	for (int i = 0; i < COUNTOF(numbers); ++i)
		ds_array_push(&array, &numbers[i]);

	ds_array_remove_at(&array, 6); // end
	ds_array_remove_at(&array, 0); // start
	ds_array_remove_at(&array, 3); // middle

	int expected[] = { 1, 2, 3, 5 };
	for (int i = 0; i < COUNTOF(expected); ++i)
		EXPECT_EQ(expected[i], ds_array_get(int, &array, i));

	ds_array_free(&array);
}

UTEST(ds_array, Grow)
{
	ds_array array = ds_array_new_count(int, 2);
	ds_array_set(&array, 0, &(int){0});
	ds_array_set(&array, 1, &(int){1});
	ds_array_grow(&array, 4);
	ds_array_set(&array, 2, &(int){2});
	ds_array_set(&array, 3, &(int){3});
	int expected[] = { 0, 1, 2, 3 };
	for (int i = 0; i < COUNTOF(expected); ++i)
		EXPECT_EQ(expected[i], ds_array_get(int, &array, i));

	ds_array_free(&array);
}