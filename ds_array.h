#ifndef DS_ARRAY_H
#define DS_ARRAY_H

#include "ds_allocator.h"

typedef struct ds_array
{
	void* base;
	int count;
	int cap;
	ds_allocator allocator;
	int SIZE;
} ds_array;

#ifdef __cplusplus
extern "C"
{
#endif

ds_array
ds_array_new_ex(int SIZE, int count, ds_allocator allocator);

#define ds_array_new(T)                      ds_array_new_ex(sizeof(T), 0, ds_allocator_clib())
#define ds_array_new_count(T, count)         ds_array_new_ex(sizeof(T), count, ds_allocator_clib())
#define ds_array_new_allocator(T, allocator) ds_array_new_ex(sizeof(T), 0, allocator)

void
ds_array_free(ds_array* array);

ds_array
ds_array_clone(ds_array* array);

void
ds_array_grow(ds_array* array, int new_count);

void*
ds_array_begin(ds_array* array);

void*
ds_array_end(ds_array* array);

void*
ds_array_at(ds_array* array, int i);
#define ds_array_get(T, array, i) (*(T*)ds_array_at(array, i))

void
ds_array_set(ds_array* array, int i, void* value);

void
ds_array_insert_at(ds_array* array, int i, void* element);

void
ds_array_remove_at(ds_array* array, int i);

void
ds_array_push(ds_array* array, void* element);

#ifdef __cplusplus
}
#endif
#endif // DS_ARRAY_H

#ifdef DS_ARRAY_IMPLEMENTATION

#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

void*
ds__array_at(ds_array* array, int i)
{
	return (void*)((size_t)array->base + i * array->SIZE);
}

int
ds__array_bytes(ds_array* array)
{
	return array->SIZE * array->count;
}

ds_array
ds_array_new_ex(int SIZE, int count, ds_allocator allocator)
{
	assert(SIZE > 0);
	ds_array array = {0};
	array.SIZE = SIZE;
	array.count = count;
	array.allocator = allocator;

	if (count > 0)
	{
		array.base = allocator.alloc(allocator.self, ds__array_bytes(&array));
		array.cap = array.count;
	}

	return array;
}

void
ds_array_free(ds_array* array)
{
	if (array->base && array->cap)
		array->allocator.free(array->allocator.self, array->base, array->SIZE * array->cap);

	array->base = NULL;
	array->cap = 0;
	array->count = 0;
}

ds_array
ds_array_clone(ds_array* array)
{
	ds_array clone = ds_array_new_ex(array->SIZE, array->count, array->allocator);
	memcpy(clone.base, array->base, ds__array_bytes(array));
	return clone;
}

void
ds_array_grow(ds_array* array, int new_count)
{
	// TODO: Shrink
	if (new_count <= array->cap)
	{
		array->count = new_count;
		return;
	}

	int new_cap = new_count * 3 / 2;
	if (new_cap < 8)
		new_cap = 8;
	void* new_base = array->allocator.alloc(array->allocator.self, array->SIZE * new_cap);
	memcpy(new_base, array->base, ds__array_bytes(array));

	ds_array_free(array);
	array->base = new_base;
	array->count = new_count;
	array->cap = new_cap;
}

void*
ds_array_begin(ds_array* array)
{
	return array->base;
}

void*
ds_array_end(ds_array* array)
{
	return ds__array_at(array, array->count);
}

void*
ds_array_at(ds_array* array, int i)
{
	assert(i < array->count);
	if (i < 0)
		i += array->count;
	assert(i >= 0);
	return ds__array_at(array, i);
}

void
ds_array_set(ds_array* array, int i, void* value)
{
	memcpy(ds_array_at(array, i), value, array->SIZE);
}

void
ds_array_insert_at(ds_array* array, int i, void* element)
{
	assert(i <= array->count);
	if (i < 0)
		i += array->count;
	assert(i >= 0);

	int old_count = array->count;
	ds_array_grow(array, old_count + 1);
	if (i < old_count)
	{
		int right_bytes = array->SIZE * (old_count - i);
		memcpy(ds_array_at(array, i+1), ds_array_at(array, i), right_bytes);
	}
	memcpy(ds_array_at(array, i), element, array->SIZE);
}

void
ds_array_remove_at(ds_array* array, int i)
{
	assert(i < array->count);
	if (i < 0)
		i += array->count;
	assert(i >= 0);

	if (i+1 < array->count)
	{
		int right_bytes = array->SIZE * (array->count - (i+1));
		memcpy(ds_array_at(array, i), ds_array_at(array, i+1), right_bytes);
	}
	array->count--;
}

void
ds_array_push(ds_array* array, void* element)
{
	ds_array_insert_at(array, array->count, element);
}

#undef DS_ARRAY_IMPLEMENTATION
#endif // DS_ARRAY_IMPLEMENTATION