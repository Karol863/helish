#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <string.h>

#define CAPACITY 4096
#define COMMANDS 2048

typedef size_t usize;

typedef struct {
	char *buf;
	usize buf_len;
	usize offset;
} Arena;

typedef struct {
	char *buf;
	usize head;
	usize tail;
} Buffer;

void *arena_alloc(Arena *a, usize size);
void arena_init(Arena *a, void *buf, usize buf_len);
void arena_free(Arena *a);

void buffer_write(Buffer *b, char data);
void buffer_init(Buffer *b, void *buf);

#endif
