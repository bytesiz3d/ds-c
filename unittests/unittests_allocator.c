#include "utest.h"
#define COUNTOF(x) ((int)(sizeof(x) / sizeof(x[0])))

#include <ds_allocator.h>

UTEST(ds_allocator, clib)
{
	ds_allocator allocator = ds_allocator_clib();

	int* numbers = allocator.alloc(allocator.self, sizeof(int) * 8);
	for (int i = 0; i < 8; ++i)
		numbers[i] = i;

	for (int i = 0; i < 8; ++i)
		EXPECT_EQ(i, numbers[i]);

	allocator.free(allocator.self, numbers, sizeof(int) * 8);
}

UTEST(ds_allocator, arena)
{
	ds_arena arena = ds_arena_new(ds_allocator_clib(), 1024);
	ds_allocator allocator = ds_arena_allocator(&arena);

	// [numbers...]
	int numbers_count = 8;
	int* numbers = allocator.alloc(allocator.self, sizeof(int) * numbers_count);
	for (int i = 0; i < numbers_count; ++i) numbers[i] = i;
	for (int i = 0; i < numbers_count; ++i) EXPECT_EQ(i, numbers[i]);

	{
		ds_arena subarena = ds_arena_subarena(&arena);
		ds_allocator suballocator = ds_arena_allocator(&subarena);

		// [numbers..., subnumbers...]
		int* subnumbers = suballocator.alloc(suballocator.self, sizeof(int) * numbers_count);
		for (int i = 0; i < numbers_count; ++i) subnumbers[i] = i;
		for (int i = 0; i < numbers_count; ++i) EXPECT_EQ(i, subnumbers[i]);
	}

	// [numbers..., last_number]
	int* last_number = allocator.alloc(allocator.self, sizeof(int));
	*last_number = numbers_count;
	for (int i = 0; i < numbers_count+1; ++i) EXPECT_EQ(i, numbers[i]);

	ds_arena_free(&arena);
}