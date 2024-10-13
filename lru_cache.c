/*
 * Copyright (c) 2024, Postelnicu Cristina-Marina 313CA
 */

#include <stdio.h>
#include <string.h>
#include "lru_cache.h"
#include "utils.h"

// functia ht_create creeaza un hashtable
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*))
{
	unsigned i;
	hashtable_t *hashtable;
	hashtable = malloc(1 * sizeof(hashtable_t));
	hashtable->buckets = malloc(hmax * sizeof(doubly_linked_list_t*));
	hashtable->hmax = hmax;
	hashtable->size = 0;
	hashtable->hash_function = hash_function;
	hashtable->compare_function = compare_function;
	hashtable->key_val_free_function = key_val_free_function;
	for (i = 0; i < hmax; i++) {
		hashtable->buckets[i] = dll_create(sizeof(info));
	}
	return hashtable;
}

/* functia ht_has_key returneaza 1 daca cheia se afla in hastable
si 0 in caz contarar */
int ht_has_key(hashtable_t *ht, void *key)
{
	unsigned int hash;
	hash = ht->hash_function(key);
	hash = hash % ht->hmax;
	dll_node_t *current_node = ht->buckets[hash]->head;
	while (current_node != NULL) {
		if (ht->compare_function(((info*)(current_node->data))->key, key) == 0)
			return 1;
		current_node = current_node->next;
	}
	return 0;
}

// functia ht_free elibereaza memoria alocata pentru un hashtable
void ht_free(hashtable_t *ht)
{
	unsigned i, j;
	for (i = 0; i < ht->hmax; i++) {
		dll_node_t *current_node = ht->buckets[i]->head;
		for(j = 0; j < ht->buckets[i]->size; j++) {
			dll_node_t *aux = current_node;
			current_node = current_node->next;
			free(((info *)aux->data)->key);
			free(aux->data);
			free(aux);
		}
		free(ht->buckets[i]);
	}
	free(ht->buckets);
	free(ht);
}

// functia key_val_free_function elibereaza cheia si valoarea
void key_val_free_function(void *data) {
	if (((info*)data)->key)
		free(((info*)data)->key);
	if (((info*)data)->value)
		free(((info*)data)->value);
}

/* functia init_lru_cache creeaza un cache ce contine un hashtable
si o lista de recente */
lru_cache *init_lru_cache(unsigned int cache_capacity) {
	lru_cache *cache;
	cache = malloc(1 * sizeof(lru_cache));
	cache->hashtable = ht_create(cache_capacity, hash_string,
								 compare_func_strings, key_val_free_function);
	cache->order = dll_create(sizeof(info));
	return cache;
}

// functia lru_cache_is_full verifica daca cache ul este plin
bool lru_cache_is_full(lru_cache *cache) {
	if (cache->hashtable->size == cache->hashtable->hmax)
		return true;
	return false;
}

/* functia free_lru_cache elibereaza memoria folosita
pentru un cache*/
void free_lru_cache(lru_cache **cache) {
	// eliberam lista de recente
	dll_node_t *current_node = (*cache)->order->head;
	while (current_node != NULL) {
		dll_node_t *aux = current_node;
		current_node = current_node->next;
		key_val_free_function(aux->data);
		free(aux->data);
		free(aux);
		aux = NULL;
	}
	free((*cache)->order);
	// eliberam hashtable ul
	ht_free((*cache)->hashtable);
	// eliberam cache ul
	free(*cache);
}

// functia lru_cache_put adauga un element in cache sau il actualizeaza
bool lru_cache_put(lru_cache *cache, void *key, void *value,
				   void **evicted_key) {
	unsigned int i = 0, nr = 0, j = 0;
	if (ht_has_key(cache->hashtable, key) == 1) {
		/* daca cheia se gaseste in hashtable o mutam
		la finalul listei de recente, pentru a fi cel mai recent
		accesat element */
		unsigned int hash;
		hash = cache->hashtable->hash_function(key);
		hash = hash % cache->hashtable->hmax;
		dll_node_t *current_node = cache->hashtable->buckets[hash]->head;
		// parcurgem hashtable ul pentru a gasi elementul
		for (i = 0; i < cache->hashtable->buckets[hash]->size; i++) {
			if (cache->hashtable->compare_function
				(((info*)(current_node->data))->key, key) == 0) {
				// parcurgem lista de recente pentru a gasi elementul
				dll_node_t *current_node_order = cache->order->head;
				for (j = 0; j < cache->order->size; j++) {
					// obtinem pozitia elementului in lista de recente
					if (strcmp(
						(char *)(((info *)current_node->data)->key), (char *)
						(((info *)current_node_order->data)->key)) == 0) {
						nr = j;
					}
					current_node_order = current_node_order->next;
				}
				dll_node_t *to_remove;
				// eliminam elementul din lista de recente
				to_remove = dll_remove_nth_node(cache->order, nr);
				// adaugam elementul la finalul listei de recente
				dll_add_nth_node(cache->order, cache->order->size,
								 (info*)(to_remove->data));
				free(to_remove);
				return false;
			}
			current_node = current_node->next;
		}
	} else {
		/* daca cheia nu se afla in hashtable adaugam elementul in hashtable
		si in lista de recente */
		unsigned int hash;
		// verificam daca cache ul este plin
		if (lru_cache_is_full(cache)) {
			/* daca este plin eliminam primul element din lista de recente ca fiid
			 cel mai putin accesat */
			*evicted_key = (char *)malloc(DOC_CONTENT_LENGTH * sizeof(char));
			strcpy((char *)*evicted_key,
				   (char *)(((info *)cache->order->head->data)->key));
			lru_cache_remove(cache, *evicted_key);
		}
		hash = cache->hashtable->hash_function(key);
		hash = hash % cache->hashtable->hmax;
		// adaugam elementul in lista de recente
		info *new_node_order;
		new_node_order = malloc(sizeof(info));
		new_node_order->key = malloc(1 * DOC_NAME_LENGTH);
		memcpy(new_node_order->key, key, DOC_NAME_LENGTH);
		new_node_order->value = malloc(1 * DOC_CONTENT_LENGTH);
		memcpy(new_node_order->value, value, DOC_CONTENT_LENGTH);
		dll_add_nth_node(cache->order, cache->order->size, new_node_order);
		// adaugam elementul in hashtable
		info* new_node;
		new_node = malloc(sizeof(info));
		new_node->key = malloc(1 * DOC_NAME_LENGTH);
		memcpy(new_node->key, key, DOC_NAME_LENGTH);
		new_node->value = cache->order->tail;
		dll_add_nth_node(cache->hashtable->buckets[hash],
						 cache->hashtable->buckets[hash]->size, new_node);
		// crestem dimensiunea hashtable ului
		cache->hashtable->size = cache->hashtable->size + 1;
		free(new_node_order);
		free(new_node);
	}
	return true;
}

// functia lru_cache_get returneaza valoarea unui element din cache
void *lru_cache_get(lru_cache *cache, void *key) {
	unsigned int hash, i = 0, nr = 0, j = 0;
	hash = cache->hashtable->hash_function(key);
	hash = hash % cache->hashtable->hmax;
	dll_node_t *current_node = cache->hashtable->buckets[hash]->head;
	for (i = 0; i < cache->hashtable->buckets[hash]->size; i++) {
		/* daca cheia se afla in hashtable returnam valoarea corespunzatoare
		acesteia */
		if (cache->hashtable->compare_function
			(((info*)current_node->data)->key, key) == 0) {
			dll_node_t *current_node_order = cache->order->head;
			// parcurgem lista de recente pentru a gasi elementul
			for (j = 0; j < cache->order->size; j++) {
				if (strcmp((char *)(((info *)current_node->data)->key),
				   (char *)(((info *)current_node_order->data)->key)) == 0) {
					// obtinem pozitia elementului in lista de recente
					nr = j;
				}
				current_node_order = current_node_order->next;
			}
			dll_node_t *to_remove;
			// eliminam elementul din lista de recente
			to_remove = dll_remove_nth_node(cache->order, nr);
			// adaugam elementul la finalul listei de recente
			dll_add_nth_node(cache->order, cache->order->size,
							 (info*)(to_remove->data));
			// actualizam valoarea elementului in hashtable
			((info*)current_node->data)->value = cache->order->tail;
			free(to_remove->data);
			free(to_remove);
			// returnam valoarea elementului
			return ((info*)current_node->data)->value;
		}
		current_node = current_node->next;
	}
	return NULL;
}

// functia lru_cache_remove elimina un element din cache
void lru_cache_remove(lru_cache *cache, void *key) {
	int nr1 = 0, nr2 = 0;
	unsigned int hash, i = 0, j = 0;
	hash = cache->hashtable->hash_function(key);
	hash = hash % cache->hashtable->hmax;
	dll_node_t *current_node = cache->hashtable->buckets[hash]->head;
	for (i = 0; i < cache->hashtable->buckets[hash]->size; i++) {
		// daca cheia se afla in hashtable o eliminam
		if (cache->hashtable->compare_function
			(((info*)(current_node->data))->key, key) == 0) {
			// parcurgem lista de recente pentru a gasi elementul
			dll_node_t *current_node_order = cache->order->head;
			for (j = 0; j < cache->order->size; j++) {
				if (strcmp((char *)(((info *)current_node->data)->key),
					(char *)(((info *)current_node_order->data)->key)) == 0) {
					/* obtinem pozitia elementului in lista de recente si
					in hashtable */
					nr2 = j;
					nr1 = i;
					dll_node_t *to_remove, *to_remove_order;
					// eliminam elementul din lista de recente si din hashtable
					to_remove_order = dll_remove_nth_node(cache->order, nr2);
					to_remove =
					dll_remove_nth_node(cache->hashtable->buckets[hash], nr1);
					// actualizam dimensiunea hashtable ului
					cache->hashtable->size = cache->hashtable->size - 1;
					// eliberam memoria alocata pentru elemente
					cache->hashtable->
					key_val_free_function(to_remove_order->data);
					free(to_remove_order->data);
					free(to_remove_order);
					if (((info *)to_remove->data)->key)
						free(((info *)to_remove->data)->key);
					free(to_remove->data);
					free(to_remove);
					return;
				}
				current_node_order = current_node_order->next;
			}
		}
		current_node = current_node->next;
	}
}
