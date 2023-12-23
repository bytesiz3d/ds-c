#ifndef DS_SET_H
#define DS_SET_H

#include <stdint.h>

#include "ds_allocator.h"
#include "ds_array.h"

typedef uint32_t (*ds_hash_fn)(const void* base, int SIZE);

uint32_t
ds_hash_murmur3(const void* base, int SIZE);

typedef struct ds_set
{
	ds_array values;
	ds_array _slots;
	ds_hash_fn hash_fn;
	ds_allocator allocator;
} ds_set;

#ifdef __cplusplus
extern "C"
{
#endif

ds_set
ds_set_new_ex(int SIZE, ds_hash_fn hash_fn, ds_allocator allocator);

#define ds_set_new(T)                      ds_set_new_ex(sizeof(T), hash_murmur3, ds_allocator_clib())
#define ds_set_new_hash_fn(T, hash_fn)     ds_set_new_ex(sizeof(T), hash_fn, ds_allocator_clib())
#define ds_set_new_allocator(T, allocator) ds_set_new_ex(sizeof(T), hash_murmur3, allocator)

void
ds_set_free(ds_set* set);

void*
ds_set_begin(ds_set* set);

void*
ds_set_end(ds_set* set);

void*
ds_set_lookup(ds_set* set, void* value);

void*
ds_set_insert(ds_set* set, void* value);

void
ds_set_remove(ds_set* set, void* value);

#ifdef __cplusplus
}
#endif
#endif // DS_SET_H

#ifdef DS_SET_IMPLEMENTATION
#include <memory.h>
#include <stdbool.h>

typedef struct ds__set_slot
{
	uint32_t used : 2;
	uint32_t index : 30;
} ds__set_slot;

bool
ds__set_slot_is_free(ds_set* set, int si) { return ds_array_get(ds__set_slot, &set->_slots, si).used == 0; }

bool
ds__set_slot_is_used(ds_set* set, int si) { return ds_array_get(ds__set_slot, &set->_slots, si).used == 1; }

bool
ds__set_slot_is_deleted(ds_set* set, int si) { return ds_array_get(ds__set_slot, &set->_slots, si).used == 2; }

void
ds__set_slot_clear(ds_set* set, int si) { ds_array_get(ds__set_slot, &set->_slots, si).used = 0; }

void
ds__set_slot_delete(ds_set* set, int si) { ds_array_get(ds__set_slot, &set->_slots, si).used = 2; }

int
ds__set_slot_value_index(ds_set* set, int si) { return ds_array_get(ds__set_slot, &set->_slots, si).index; }

void
ds__set_slot_use_value_index(ds_set* set, int si, int index)
{
	ds_array_get(ds__set_slot, &set->_slots, si).index = index;
	ds_array_get(ds__set_slot, &set->_slots, si).used = 1;
}

int
ds__set_slot_find_lookup(ds_set* set, void* value)
{
	if (set->_slots.count == 0)
		return -1;

	uint32_t hash_0 = set->hash_fn(value, set->values.SIZE);

	uint32_t probe = hash_0;
	uint32_t cap = set->_slots.count;
	do
	{
		int si = probe & (cap - 1);
		if (ds__set_slot_is_free(set, si)) // free slot (not deleted), value was not hashed before
			return -1;

		if (ds__set_slot_is_used(set, si))
		{
			void* existing_value = ds_array_at(&set->values, ds__set_slot_value_index(set, si));
			if (set->hash_fn(existing_value, set->values.SIZE) == hash_0) // hashes match, found the right element
				return si;
		}

		probe++;
	}
	while ((probe - hash_0) & (cap - 1));
	return -1;
}

int
ds__set_slot_find_insert(ds_set* set, void* value)
{
	if (set->_slots.count == 0)
		return -1;

	uint32_t hash_0 = set->hash_fn(value, set->values.SIZE);

	uint32_t probe = hash_0;
	uint32_t cap = set->_slots.count;
	do
	{
		int si = probe & (cap - 1);
		if (ds__set_slot_is_free(set, si) || ds__set_slot_is_deleted(set, si)) // free slot (not deleted), value was not hashed before
			return si;

		probe++;
	}
	while ((probe - hash_0) & (cap - 1));
	return -1;
}

void
ds__set_grow(ds_set* set, int new_count)
{
	ds_set new_set = *set;
	new_set._slots = ds_array_clone(&set->_slots);
	ds_array_grow(&new_set._slots, new_count);
	memset(new_set._slots.base, 0, ds__array_bytes(&new_set._slots));

	// rehash _slots
	for (int si = 0; si < set->_slots.count; si++)
	{
		if (ds__set_slot_is_used(set, si))
		{
			int index = ds__set_slot_value_index(set, si);
			void* value = ds_array_at(&set->values, index);

			int new_si = ds__set_slot_find_insert(&new_set, value);
			assert(new_si >= 0);

			ds__set_slot_use_value_index(&new_set, new_si, index);
		}
	}
	ds_array_free(&set->_slots);
	*set = new_set;
}

ds_set
ds_set_new_ex(int SIZE, ds_hash_fn hash_fn, ds_allocator allocator)
{
	ds_set set = {0};
	set.values = ds_array_new_ex(SIZE, 0, allocator);
	set._slots = ds_array_new_allocator(ds__set_slot, allocator);
	set.hash_fn = hash_fn;
	set.allocator = allocator;
	return set;
}

void
ds_set_free(ds_set* set)
{
	ds_array_free(&set->values);
	ds_array_free(&set->_slots);
}

void*
ds_set_begin(ds_set* set)
{
	return ds_array_begin(&set->values);
}

void*
ds_set_end(ds_set* set)
{
	return ds_array_end(&set->values);
}

void*
ds_set_lookup(ds_set* set, void* value)
{
	int si = ds__set_slot_find_lookup(set, value);
	if (si < 0)
		return NULL;
	return ds_array_at(&set->values, ds__set_slot_value_index(set, si));
}

int
_next_power_of_two(int value)
{
	int next_power_of_two = 8;
	while (next_power_of_two <= value)
		next_power_of_two <<= 1;
	return next_power_of_two;
}

void*
ds_set_insert(ds_set* set, void* value)
{
	int si = ds__set_slot_find_lookup(set, value);
	if (si >= 0) // already exists, overwrite and return existing value
	{
		void* existing_value = ds_array_at(&set->values, ds__set_slot_value_index(set, si));
		memcpy(existing_value, value, set->values.SIZE);
		return existing_value;
	}

	si = ds__set_slot_find_insert(set, value);
	if (si < 0)
	{
		int new_count = _next_power_of_two(set->values.count + 1);
		ds__set_grow(set, new_count);
		si = ds__set_slot_find_insert(set, value);
		assert(si >= 0);
	}

	ds__set_slot_use_value_index(set, si, set->values.count);
	ds_array_push(&set->values, value);
	return ds_array_at(&set->values, -1);
}

void
ds_set_remove(ds_set* set, void* value)
{
	int si = ds__set_slot_find_lookup(set, value);
	if (si < 0)
		return;

	// TODO: Shrink
	int index = ds__set_slot_value_index(set, si);
	ds__set_slot_delete(set, si);
	ds_array_remove_at(&set->values, index);
}

uint32_t
ds__murmur32_scramble(uint32_t k)
{
	k *= 0xcc9e2d51;
	k = (k << 15) | (k >> 17);
	k *= 0x1b873593;
	return k;
}

uint32_t
ds_hash_murmur3(const void* base, int SIZE)
{
	const uint8_t* key = (const uint8_t*)base;
	size_t len = SIZE;
	uint32_t seed = 0xa7d94a9c; // https://www.random.org/cgi-bin/randbyte?nbytes=4&format=h

	uint32_t h = seed;
	uint32_t k;
	/* Read in groups of 4. */
	for (size_t i = len >> 2; i; i--)
	{
		memcpy(&k, key, sizeof(uint32_t));
		key += sizeof(uint32_t);
		h ^= ds__murmur32_scramble(k);
		h = (h << 13) | (h >> 19);
		h = h * 5 + 0xe6546b64;
	}
	/* Read the rest. */
	k = 0;
	for (size_t i = len & 3; i; i--)
	{
		k <<= 8;
		k |= key[i - 1];
	}
	h ^= ds__murmur32_scramble(k);
	/* Finalize. */
	h ^= len;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}
#undef DS_SET_IMPLEMENTATION
#endif // DS_SET_IMPLEMENTATION