/*
 * Copyright (c) 2024, Postelnicu Cristina-Marina 313CA
 */

#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <stdbool.h>

typedef struct dll_node_t dll_node_t;
struct dll_node_t
{
    void* data;
    dll_node_t *prev, *next;
};

typedef struct doubly_linked_list_t doubly_linked_list_t;
struct doubly_linked_list_t
{
    dll_node_t* head;
    dll_node_t* tail;
    unsigned int data_size;
    unsigned int size;
};

typedef struct info info;
struct info {
	void *key;
	void *value;
};

typedef struct hashtable_t hashtable_t;
struct hashtable_t {
	doubly_linked_list_t **buckets;
	unsigned int size;
	unsigned int hmax;
	unsigned int (*hash_function)(void*);
	int (*compare_function)(void*, void*);
	void (*key_val_free_function)(void*);
};


typedef struct lru_cache {
	hashtable_t *hashtable;
	doubly_linked_list_t *order;
} lru_cache;

// functii de lista
doubly_linked_list_t *dll_create(unsigned int data_size);

void dll_add_nth_node(doubly_linked_list_t *list,
					  unsigned int n, const void* new_data);

dll_node_t *dll_remove_nth_node(doubly_linked_list_t *list, unsigned int n);

void dll_free(doubly_linked_list_t **pp_list);

// functii de hashtable
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*));

void key_val_free_function(void *data);

int ht_has_key(hashtable_t *ht, void *key);

void ht_free(hashtable_t *ht);

void key_val_free_function(void *data);


lru_cache *init_lru_cache(unsigned int cache_capacity);

bool lru_cache_is_full(lru_cache *cache);

void free_lru_cache(lru_cache **cache);

/**
 * lru_cache_put() - Adds a new pair in our cache.
 * 
 * @param cache: Cache where the key-value pair will be stored.
 * @param key: Key of the pair.
 * @param value: Value of the pair.
 * @param evicted_key: The function will RETURN via this parameter the
 *      key removed from cache if the cache was full.
 * 
 * @return - true if the key was added to the cache,
 *      false if the key already existed.
 */
bool lru_cache_put(lru_cache *cache, void *key, void *value,
                   void **evicted_key);

/**
 * lru_cache_get() - Retrieves the value associated with a key.
 * 
 * @param cache: Cache where the key-value pair is stored.
 * @param key: Key of the pair.
 * 
 * @return - The value associated with the key,
 *      or NULL if the key is not found.
 */
void *lru_cache_get(lru_cache *cache, void *key);

/**
 * lru_cache_remove() - Removes a key-value pair from the cache.
 * 
 * @param cache: Cache where the key-value pair is stored.
 * @param key: Key of the pair.
*/
void lru_cache_remove(lru_cache *cache, void *key);

#endif /* LRU_CACHE_H */
