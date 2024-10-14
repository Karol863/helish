#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <string.h>

#include "macros.h"

typedef struct {
	char *buf;
	usize buf_len;
	usize offset;
} Arena;

void *arena_alloc(Arena *a, usize size);
void arena_init(Arena *a, void *buf, usize buf_len);
void arena_free(Arena *a);

#endif
