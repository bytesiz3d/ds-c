#ifndef DSALLOCATOR_H
#define DSALLOCATOR_H

typedef struct dsAllocator
{
	void* self;
	void* (*alloc)(void* self, int size);
	void (*free)(void* self, void* base, int size);
} dsAllocator;

#ifdef __cplusplus
extern "C"
{
#endif

dsAllocator
allocator_clib();

#ifdef __cplusplus
}
#endif
#endif // DSALLOCATOR_H

#ifdef DSALLOCATOR_IMPLEMENTATION
#include <stdlib.h>

void*
_clib_alloc(void* self, int size)
{
	(void)self;
	return malloc(size);
}

void
_clib_free(void* self, void* base, int size)
{
	(void)self; (void)size;
	free(base);
}

dsAllocator
allocator_clib()
{
	dsAllocator allocator = {0};
	allocator.alloc = _clib_alloc;
	allocator.free = _clib_free;
	return allocator;
}

#undef DSALLOCATOR_IMPLEMENTATION
#endif // DSALLOCATOR_IMPLEMENTATION