#include "memory.h"

void *arena_alloc(Arena *a, size_t size) {
	if (a->offset + size <= a->buf_len) {
		void *ptr = &a->buf[a->offset];
		a->offset += size;
		memset(ptr, 0, size);
		return ptr;
	}
	return NULL;
}

void arena_init(Arena *a, void *buf, size_t buf_len) {
	a->buf = (char *)buf;
	a->buf_len = buf_len;
	a->offset = 0;
}

void arena_free(Arena *a) {
	a->offset = 0;
}
