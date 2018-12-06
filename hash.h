/*
 * This module implements a simple hash table of a fixed size, 
 * with string keys and non-negative integer data
 *
 * Author: James Barnes (820946)
 *
 */

#ifndef HASH_H
#define HASH_H
 
#define NOT_IN_HASHTABLE -1
 
typedef struct hashTable HashTable;

typedef struct hashItem HashItem;

struct hashTable
{
	HashItem **table;
	unsigned int size;
};

struct hashItem
{
	char *key;
	int data;
	HashItem *next; // chained item
};

// create and return a new HashTable of size size
//
// WARNING: assertion fails on size <= 0
HashTable *new_hash(unsigned int size);

// insert a new item into a hashTable
//
// WARNING: this does not check if the key is already in the hash table
// WARNING: assertion fails on key == NULL or data <= 0
void hash_insert(HashTable *ht, char *key, int data);

// find an item, and if it exists, move to front and return it's data,
// else NOT_IN_HASHTABLE
int hash_get(HashTable *ht, char *key);

// find an item, and if it exists, move to front and return it's key, 
// else NULL
char *hash_key(HashTable *ht, char *key);

// frees all memory associated with a hash
//
// WARNING: does not free key memory
void free_hash(HashTable *ht);

// the xor hash, a string hashing function
//
// source: Zobel & Ramakrishnan (1997)
unsigned int xor_hash(char *str, int size);

#endif
