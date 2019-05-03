#include <uthash.h>

#include "utils.h"
#include "cache.h"

struct cache_entry {
	char *key;
	void *value;
	UT_hash_handle hh;
};

struct cache {
	cache_getter_t getter;
	cache_free_t free;
	void *user_data;
	struct cache_entry *entries;
};

void *cache_get(struct cache *c, const char *key) {
	struct cache_entry *e;
	HASH_FIND_STR(c->entries, key, e);
	if (e) {
		return e->value;
	}

	e = ccalloc(1, struct cache_entry);
	e->key = strdup(key);
	e->value = c->getter(c->user_data, key);
	HASH_ADD_STR(c->entries, key, e);
	return e->value;
}

static inline void _cache_invalidate(struct cache *c, struct cache_entry *e) {
	if (c->free) {
		c->free(c->user_data, e->value);
	}
	HASH_DEL(c->entries, e);
	free(e);
}

void cache_invalidate(struct cache *c, const char *key) {
	struct cache_entry *e;
	HASH_FIND_STR(c->entries, key, e);

	if (e) {
		_cache_invalidate(c, e);
	}
}

void cache_invalidate_all(struct cache *c) {
	struct cache_entry *e, *tmpe;
	HASH_ITER(hh, c->entries, e, tmpe) {
		_cache_invalidate(c, e);
	}
}

void *cache_free(struct cache *c) {
	void *ret = c->user_data;
	cache_invalidate_all(c);
	free(c);
	return ret;
}
