#ifndef DSSET_H
#define DSSET_H

#include <stdint.h>

#include "dsAllocator.h"
#include "dsArray.h"

typedef uint32_t (*dsHash)(void* base, int SIZE);

uint32_t
hash_murmur3(void* base, int SIZE);

typedef struct dsSet
{
	dsArray values;
	dsArray _slots;
	dsHash hash;
	dsAllocator allocator;
	int SIZE;
} dsSet;

#ifdef __cplusplus
extern "C"
{
#endif

#define set_new(T)                      set_new_ex(sizeof(T), hash_murmur3, allocator_clib())
#define set_new_hash(T, hash)           set_new_ex(sizeof(T), hash, allocator_clib())
#define set_new_allocator(T, allocator) set_new_ex(sizeof(T), hash_murmur3, allocator)
dsSet
set_new_ex(int SIZE, dsHash hash, dsAllocator allocator);

void
set_free(dsSet* set);

void*
set_begin(dsSet* set);

void*
set_end(dsSet* set);

void*
set_lookup(dsSet* set, void* value);

void*
set_insert(dsSet* set, void* value);

void
set_remove(dsSet* set, void* value);

#ifdef __cplusplus
}
#endif
#endif // DSSET_H

#ifdef DSSET_IMPLEMENTATION
#include <memory.h>
#include <stdbool.h>

typedef struct dsSetSlot
{
	uint32_t used : 2;
	uint32_t index : 30;
} dsSetSlot;

bool
_set_slot_is_free(dsSet* set, int si) { return array_get(dsSetSlot, &set->_slots, si).used == 0; }

bool
_set_slot_is_used(dsSet* set, int si) { return array_get(dsSetSlot, &set->_slots, si).used == 1; }

bool
_set_slot_is_deleted(dsSet* set, int si) { return array_get(dsSetSlot, &set->_slots, si).used == 2; }

void
_set_slot_clear(dsSet* set, int si) { array_get(dsSetSlot, &set->_slots, si).used = 0; }

void
_set_slot_delete(dsSet* set, int si) { array_get(dsSetSlot, &set->_slots, si).used = 2; }

int
_set_slot_value_index(dsSet* set, int si) { return array_get(dsSetSlot, &set->_slots, si).index; }

void
_set_slot_use_value_index(dsSet* set, int si, int index)
{
	array_get(dsSetSlot, &set->_slots, si).index = index;
	array_get(dsSetSlot, &set->_slots, si).used = 1;
}

int
_set_slot_find_lookup(dsSet* set, void* value)
{
	if (set->_slots.count == 0)
		return -1;

	uint32_t hash_0 = set->hash(value, set->SIZE);

	uint32_t probe = hash_0;
	uint32_t cap = set->_slots.count;
	do
	{
		int si = probe & (cap - 1);
		if (_set_slot_is_free(set, si)) // free slot (not deleted), value was not hashed before
			return -1;

		if (_set_slot_is_used(set, si))
		{
			void* existing_value = array_at(&set->values, _set_slot_value_index(set, si));
			if (set->hash(existing_value, set->SIZE) == hash_0) // hashes match, found the right element
				return si;
		}

		probe++;
	}
	while ((probe - hash_0) & (cap - 1));
	return -1;
}

int
_set_slot_find_insert(dsSet* set, void* value)
{
	if (set->_slots.count == 0)
		return -1;

	uint32_t hash_0 = set->hash(value, set->SIZE);

	uint32_t probe = hash_0;
	uint32_t cap = set->_slots.count;
	do
	{
		int si = probe & (cap - 1);
		if (_set_slot_is_free(set, si) || _set_slot_is_deleted(set, si)) // free slot (not deleted), value was not hashed before
			return si;

		probe++;
	}
	while ((probe - hash_0) & (cap - 1));
	return -1;
}

void
_set_grow(dsSet* set, int new_count)
{
	dsSet new_set = *set;
	new_set._slots = array_clone(&set->_slots);
	array_grow(&new_set._slots, new_count);
	memset(new_set._slots.base, 0, _array_bytes(&new_set._slots));

	// rehash _slots
	for (int si = 0; si < set->_slots.count; si++)
	{
		if (_set_slot_is_used(set, si))
		{
			int index = _set_slot_value_index(set, si);
			void* value = array_at(&set->values, index);

			int new_si = _set_slot_find_insert(&new_set, value);
			assert(new_si >= 0);

			_set_slot_use_value_index(&new_set, new_si, index);
		}
	}
	array_free(&set->_slots);
	*set = new_set;
}

dsSet
set_new_ex(int SIZE, dsHash hash, dsAllocator allocator)
{
	dsSet set = {0};
	set.SIZE = SIZE;
	set.values = array_new_ex(SIZE, 0, allocator);
	set._slots = array_new_allocator(dsSetSlot, allocator);
	set.hash = hash;
	set.allocator = allocator;
	return set;
}

void
set_free(dsSet* set)
{
	array_free(&set->values);
	array_free(&set->_slots);
}

void*
set_begin(dsSet* set)
{
	return array_begin(&set->values);
}

void*
set_end(dsSet* set)
{
	return array_end(&set->values);
}

void*
set_lookup(dsSet* set, void* value)
{
	int si = _set_slot_find_lookup(set, value);
	if (si < 0)
		return NULL;
	return array_at(&set->values, _set_slot_value_index(set, si));
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
set_insert(dsSet* set, void* value)
{
	int si = _set_slot_find_insert(set, value);
	if (si < 0)
	{
		int new_count = _next_power_of_two(set->values.count + 1);
		_set_grow(set, new_count);
		si = _set_slot_find_insert(set, value);
		assert(si >= 0);
	}

	_set_slot_use_value_index(set, si, set->values.count);
	array_push(&set->values, value);
	return array_at(&set->values, -1);
}

void
set_remove(dsSet* set, void* value)
{
	int si = _set_slot_find_lookup(set, value);
	if (si < 0)
		return;

	// TODO: Shrink
	int index = _set_slot_value_index(set, si);
	_set_slot_delete(set, si);
	array_remove_at(&set->values, index);
}

uint32_t
_murmur_32_scramble(uint32_t k)
{
	k *= 0xcc9e2d51;
	k = (k << 15) | (k >> 17);
	k *= 0x1b873593;
	return k;
}

uint32_t
hash_murmur3(void *base, int SIZE)
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
		h ^= _murmur_32_scramble(k);
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
	h ^= _murmur_32_scramble(k);
	/* Finalize. */
	h ^= len;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;
	return h;
}
#undef DSSET_IMPLEMENTATION
#endif // DSSET_IMPLEMENTATION