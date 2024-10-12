#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <string.h>

typedef struct {
	char *buf;
	size_t buf_len;
	size_t offset;
} Arena;

void *arena_alloc(Arena *a, size_t size);
void arena_init(Arena *a, void *buf, size_t buf_len);
void arena_free(Arena *a);

#endif
