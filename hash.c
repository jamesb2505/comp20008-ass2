/* 
 * Implementation of functions for "hash.h"
 * 
 * Author: James Barnes (820946)
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "hash.h"

#define XOR_SEED 73802

HashTable *new_hash(unsigned int size)
{
	assert(size > 0);

	HashTable *ht;
	assert(ht = malloc(sizeof(HashTable)));
	ht->size = size;
	assert(ht->table = malloc(sizeof(HashItem *) * size));
	for (int i = 0; i < size; i++)
	{
		ht->table[i] = NULL;
	}

	return ht;
}

void hash_insert(HashTable *ht, char *key, int data)
{	
	assert(data >= 0 && key);
	
	unsigned int index = xor_hash(key, ht->size);

	// build new item
	HashItem *item;
	assert(item = malloc(sizeof(HashItem)));
	item->key = key;
	item->data = data;

	// insert item at the from of the slot in table
	item->next = ht->table[index];
	ht->table[index] = item;
}

int hash_get(HashTable *ht, char *key)
{
	if (!key)
	{
		return NOT_IN_HASHTABLE;
	}
	
	unsigned int index = xor_hash(key, ht->size);
	HashItem *prev = NULL;
	// traverse list till we find item (if ever)
	for (HashItem *item = ht->table[index]; item; item = item->next)
	{
		// compare keys
		if (!strcmp(key, item->key))
		{		
			// move to front
			if (prev)
			{
				// remove item from chain
				prev->next = item->next;
				// insert item at from of chain
				item->next = ht->table[index];
				ht->table[index] = item;
			}
			
			return item->data;
		}
		prev = item;
	}

	return NOT_IN_HASHTABLE;
}

char *hash_key(HashTable *ht, char *key)
{
	// check if key is in ht, and move to front
	if (hash_get(ht, key) != NOT_IN_HASHTABLE)
	{
		// return front item if it's in there
		return ht->table[xor_hash(key, ht->size)]->key;
	}
	
	return NULL;
}

void free_hash(HashTable *ht)
{
	// free all HashItems
	for (int i = 0; i < ht->size; i++)
	{
		HashItem *next = NULL;
		for (HashItem *item = ht->table[i]; item; item = next)
		{
			next = item->next;
			free(item);
		}
	}
	free(ht->table);
	free(ht);
}

unsigned int xor_hash(char *key, int size)
{
	unsigned int h = XOR_SEED, i = 0;
	char c;	
	
	// iterate over key
	while ((c = key[i++]) != '\0')
	{
		h ^= ((h << 5) + c + (h >> 2));
	}
	
	return h % size;
}