#include "memory.h"

void *arena_alloc(Arena *a, usize size) {
	if (a->offset + size <= a->buf_len) {
		a->offset += size;
		memset(&a->buf[a->offset], 0, size);
		return &a->buf[a->offset];
	}
	return NULL;
}

void arena_init(Arena *a, void *buf, usize buf_len) {
	a->buf = (char *)buf;
	a->buf_len = buf_len;
	a->offset = 0;
}

void arena_free(Arena *a) {
	a->offset = 0;
}

void buffer_write(Buffer *b, char data) {
	b->buf[b->head] = data;
	b->head = (b->head + 1) % COMMANDS;
	memset(&b->buf[b->head], 0, data);

	if (b->head == b->tail) {
		b->tail = (b->tail + 1) % COMMANDS;
	}
}

void buffer_init(Buffer *b, void *buf) {
	b->buf = (char *)buf;
	b->head = 0;
	b->tail = 0;
}

