#ifndef DSARRAY_H
#define DSARRAY_H

#include "dsAllocator.h"

typedef struct dsArray
{
	void* base;
	int count;
	int cap;
	dsAllocator allocator;
	int SIZE;
} dsArray;

#ifdef __cplusplus
extern "C"
{
#endif

#define array_new(T)                      array_new_ex(sizeof(T), 0, allocator_clib())
#define array_new_count(T, count)         array_new_ex(sizeof(T), count, allocator_clib())
#define array_new_allocator(T, allocator) array_new_ex(sizeof(T), 0, allocator)
dsArray
array_new_ex(int SIZE, int count, dsAllocator allocator);

void
array_free(dsArray* array);

dsArray
array_clone(dsArray* array);

void
array_grow(dsArray* array, int new_count);

void*
array_begin(dsArray* array);

void*
array_end(dsArray* array);

void*
array_at(dsArray* array, int i);
#define array_get(T, array, i) (*(T*)array_at(array, i))

void
array_set(dsArray* array, int i, void* value);

void
array_insert_at(dsArray* array, int i, void* element);

void
array_remove_at(dsArray* array, int i);

void
array_push(dsArray* array, void* element);

#ifdef __cplusplus
}
#endif
#endif // DSARRAY_H

#ifdef DSARRAY_IMPLEMENTATION

#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

void*
_array_at(dsArray* array, int i)
{
	return (void*)((size_t)array->base + i * array->SIZE);
}

int
_array_bytes(dsArray* array)
{
	return array->SIZE * array->count;
}

dsArray
array_new_ex(int SIZE, int count, dsAllocator allocator)
{
	assert(SIZE > 0);
	dsArray array = {0};
	array.SIZE = SIZE;
	array.count = count;
	array.allocator = allocator;

	if (count > 0)
	{
		array.base = allocator.alloc(allocator.self, _array_bytes(&array));
		array.cap = array.count;
	}

	return array;
}

void
array_free(dsArray* array)
{
	if (array->base && array->cap)
		array->allocator.free(array->allocator.self, array->base, array->SIZE * array->cap);
}

dsArray
array_clone(dsArray* array)
{
	dsArray clone = array_new_ex(array->SIZE, array->count, array->allocator);
	memcpy(clone.base, array->base, _array_bytes(array));
	return clone;
}

void
array_grow(dsArray* array, int new_count)
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
	memcpy(new_base, array->base, _array_bytes(array));

	array_free(array);
	array->base = new_base;
	array->count = new_count;
	array->cap = new_cap;
}

void*
array_begin(dsArray* array)
{
	return array->base;
}

void*
array_end(dsArray* array)
{
	return _array_at(array, array->count);
}

void*
array_at(dsArray* array, int i)
{
	assert(i < array->count);
	if (i < 0)
		i += array->count;
	assert(i >= 0);
	return _array_at(array, i);
}

void
array_set(dsArray* array, int i, void* value)
{
	memcpy(array_at(array, i), value, array->SIZE);
}

void
array_insert_at(dsArray* array, int i, void* element)
{
	assert(i <= array->count);
	if (i < 0)
		i += array->count;
	assert(i >= 0);

	int old_count = array->count;
	array_grow(array, old_count + 1);
	if (i < old_count)
	{
		int right_bytes = array->SIZE * (old_count - i);
		memcpy(array_at(array, i+1), array_at(array, i), right_bytes);
	}
	memcpy(array_at(array, i), element, array->SIZE);
}

void
array_remove_at(dsArray* array, int i)
{
	assert(i < array->count);
	if (i < 0)
		i += array->count;
	assert(i >= 0);

	if (i+1 < array->count)
	{
		int right_bytes = array->SIZE * (array->count - (i+1));
		memcpy(array_at(array, i), array_at(array, i+1), right_bytes);
	}
	array->count--;
}

void
array_push(dsArray* array, void* element)
{
	array_insert_at(array, array->count, element);
}

#undef DSARRAY_IMPLEMENTATION
#endif // DSARRAY_IMPLEMENTATION