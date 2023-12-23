#ifndef DS_ALLOCATOR_H
#define DS_ALLOCATOR_H

#include <stdint.h>

typedef struct ds_allocator
{
	void* self;
	void* (*alloc)(void* self, int size);
	void (*free)(void* self, void* base, int size);
} ds_allocator;

#ifdef __cplusplus
extern "C"
{
#endif

ds_allocator
ds_allocator_clib();

// @ref https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator
typedef struct ds_arena
{
	ds_allocator meta_allocator;
	uint8_t* base;
	uint8_t* ptr;
	int size;
} ds_arena;

ds_arena
ds_arena_new(ds_allocator meta_allocator, int size);

void
ds_arena_free(ds_arena* arena);

ds_allocator
ds_arena_allocator(ds_arena* arena);

ds_arena
ds_arena_subarena(ds_arena* arena);

#ifdef __cplusplus
}
#endif
#endif // DS_ALLOCATOR_H

#ifdef DS_ALLOCATOR_IMPLEMENTATION
#include <stdlib.h>

void*
ds__allocator_clib_alloc(void* self, int size)
{
	(void)self;
	return malloc(size);
}

void
ds__allocator_clib_free(void* self, void* base, int size)
{
	(void)self; (void)size;
	free(base);
}

ds_allocator
ds_allocator_clib()
{
	ds_allocator allocator = {0};
	allocator.alloc = ds__allocator_clib_alloc;
	allocator.free = ds__allocator_clib_free;
	return allocator;
}

ds_arena
ds_arena_new(ds_allocator meta_allocator, int size)
{
	ds_arena arena = {0};
	arena.meta_allocator = meta_allocator;
	arena.size = size;
	arena.base = meta_allocator.alloc(meta_allocator.self, arena.size);
	arena.ptr = arena.base;
	return arena;
}

void
ds_arena_free(ds_arena* arena)
{
	if (arena->base && arena->size)
		arena->meta_allocator.free(arena->meta_allocator.self, arena->base, arena->size);
	arena->base = NULL;
	arena->ptr = NULL;
	arena->size = 0;
}

void*
ds__arena_alloc(void* self, int size)
{
	ds_arena* arena = (ds_arena*)self;
	if (arena->ptr + size > arena->base + arena->size)
		return NULL;
	uint8_t* ptr = arena->ptr;
	arena->ptr += size;
	return ptr;
}

void
ds__arena_free(void* self, void* base, int size)
{
	(void)self; (void)base; (void)size;
}

ds_allocator
ds_arena_allocator(ds_arena* arena)
{
	ds_allocator allocator = {0};
	allocator.self = arena;
	allocator.alloc = ds__arena_alloc;
	allocator.free = ds__arena_free;
	return allocator;
}

ds_arena
ds_arena_subarena(ds_arena* arena)
{
	ds_arena subarena = {0};
	subarena.base = arena->ptr;
	subarena.ptr = arena->ptr;
	subarena.size = arena->size - (int)(arena->ptr - arena->base);
	return subarena;
}

#undef DS_ALLOCATOR_IMPLEMENTATION
#endif // DS_ALLOCATOR_IMPLEMENTATION