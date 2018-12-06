/* 
 * Implementations for functions in "spell.h"
 * 
 * Author: James Barnes (820946)
 * 
 */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "spell.h"
#include "hash.h"

// source: stackoverflow user David Titarenco (david-titarenco)
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#define AB_SIZE  26  // length of alphabet
#define AB_START 'a' // start character of alphabet

#define EDIT_COUNT(L) ((2*(L)+1)*AB_SIZE) // number of strings edit dist 1 
										  // from a string of length L
										  // REASONING:
										  // 	 deletions: L
										  //    insertions: (L+1)*AB_SIZE
										  // substitutions: L*(AB_SIZE-1)
										  // 		 total: (2*L+1)*AB_SIZE

#define MAX_DIST 3 // number of edits checked for a misspelt word

/* * * FUNCTION PROTOTYPES * * */

// returns the Levenshtein distance between strings a and b
// inspiration: Levenshtein (1965), Wagner & Fischer (1974)
int edit_dist(char *a, char *b);

// returns an array of all strings edit distance 1 away from word
// NOTE: size = EDIT_COUNT(strlen(word))
//
// WARNING: this function assumes word is only made of characters in the 
// alphabet
char **get_edits(char *word);

// build and return a HashTable* on the list of words
HashTable *build_dictionary_hash(List *words);

// finds a correction of a given word by enumerating and hashing words
// Levenshtein distance dist from word, returning the word that has the lowest
// data (position),
// NULL if there is none
char *get_correction_hash(char *word, HashTable *dict, int dist);

// finds a correction of a given word by checking the entire dictionary,
// returning the word that occurs first with the lowest edit distance,
// NULL if there is none
char *get_correction_list(char *word, List *dict, int dist);

// mallocs and duplicates src string, then returns a pointer to it
// included because strdup() is only available in POSIX C :(
char *str_dup(char *src);


/* * * FUNCTION DEFINITIONS * * */

// see Assignment Task 1: Computing edit distance
void print_edit_distance(char *word1, char *word2) 
{
	printf("%d\n", edit_dist(word1, word2));
}

// see Assignment Task 2: Enumerating all possible edits
void print_all_edits(char *word) 
{
	// get all words at most distance 1 from word
	char **edits = get_edits(word), *str;
	
	for (int i = 0, max = EDIT_COUNT(strlen(word)); i < max; i++)
	{
		printf("%s\n", (str = edits[i]));
		free(str);
	}
	
	free(edits);
}

// see Assignment Task 3: Spell checking
void print_checked(List *dictionary, List *document) 
{
	HashTable *dict_ht = build_dictionary_hash(dictionary);
	
	// travers document List
	for (Node *n = document->head; n; n = n->next)
	{
		char *word = n->data;
		// check if word is in dictionary
		// equivalent to getting a correction at distance 0
		if (get_correction_hash(word, dict_ht, 0))
		{
			printf("%s\n", word);
		}
		else
		{
			printf("%s?\n", word);
		}
	}
	
	free_hash(dict_ht);
}

// see Assignment Task 4: Spelling correction
void print_corrected(List *dictionary, List *document) 
{
	HashTable *dict_ht = build_dictionary_hash(dictionary);
	
	// traverse document List
	for (Node *n = document->head; n; n = n->next)
	{
		char *word = n->data, *corrected = NULL;
		// initialise the cost estimate to generate and test edits.
		// grows by number of edits each time (mult)
		int cost = 1, mult = EDIT_COUNT(strlen(word));
		// check distances 0...MAX_DIST, until we find a correction or we
		// have exhaustively searched the dictionary
		for (int dist = 0; dist <= MAX_DIST; dist++)
		{
			// heuristic: if the time it would take to generate and test the 
			// edits of distance dist (~cost) is greater than the number of 
			// words in the dictionary, check the whole dictionary instead.
			// also, overflow safety for cost as cost grows quickly
			if (dictionary->size <= cost || cost < 0)
			{
				corrected = get_correction_list(word, dictionary, dist);
				// break the loop after this
				// can't do any better than what we've found
				break;
			}
			if ((corrected = get_correction_hash(word, dict_ht, dist)))
			{
				// break if we find a correction
				break;
			}
			// update cost for next dist
			cost *= mult;
		}

		if (corrected)
		{
			printf("%s\n", corrected);
		}
		else
		{
			printf("%s?\n", word);
		}
	}
	
	free_hash(dict_ht);
}

int edit_dist(char *a, char *b)
{
	int a_len = strlen(a), b_len = strlen(b);
	// return if a or b is empty is empty
	if (!a_len)
	{
		return b_len;
	}
	else if (!b_len)
	{
		return a_len;
	}
	
	// get space for two rows
	// there's less freeing and mallocing this way
	int *rows;
	assert(rows = malloc(sizeof(int)*2*(a_len+1)));
	// set the pointers to the two rows
	int *curr_row = rows, *prev_row = rows+a_len+1;
	
	// initialise 1st row
	for (int i = 0; i <= a_len; i++)
	{
		curr_row[i] = i;
	}
	// main loop
	for (int j = 1; j <= b_len; j++)
	{
		// swap rows
		int *tmp_row = prev_row;
		prev_row = curr_row;
		curr_row = tmp_row;
		// initialise 1st column
		curr_row[0] = j;
		// calculate next value based on previously calculated values
		for (int i = 1; i <= a_len; i++)
		{
			// adjacent cells in matrix, insertion / deletion
			int cost_adj = 1 + MIN(prev_row[i], curr_row[i-1]);
			// diagonal cell in matrix, character matches / substitution
			int cost_diag = prev_row[i-1] + ((a[i-1] == b[j-1]) ? 0 : 1);
			curr_row[i] = MIN(cost_adj, cost_diag);
		}
	}
	// get the final edit distance
	int dist = curr_row[a_len];
	free(rows);
	
	return dist;
}

char **get_edits(char *word)
{
	int len = strlen(word), count = 0;
	char **edits, *edited, c; 
	assert(edits = malloc(sizeof(char *) * EDIT_COUNT(len)));
	assert(edited = malloc(sizeof(char) * (len+2)));
				// at least enough space for the longest edit (insertion)
	
	// insertion
	strcpy(edited, word);
	for (int i = len; i >= 0; i--)
	{
		// shift empty space across
		edited[i+1] = edited[i];
		// iterate over alphabet in place between chars
		for (int j = 0; j < AB_SIZE; j++)
		{				
			edited[i] = j + AB_START;
			edits[count++] = str_dup(edited);
		}
	}
	
	// substition
	strcpy(edited, word);
	for (int i = len-1; i >= 0; i--)
	{
		// remove the current letter
		c = edited[i];
		//iterate over alphabet
		for (int j = 0; j < AB_SIZE; j++)
		{
			// ignore if it matches the original char in word (duplicate)
			if ((edited[i] = j + AB_START) != c)
			{
				edits[count++] = str_dup(edited);
			}
		}
		// reinsert the letter
		edited[i] = c;
	}
	
	// deletion
	strcpy(edited, word);
	c = '\0';
	for (int i = len-1; i >= 0; i--)
	{
		// swap in previous charcter for current
		char tmp = edited[i];
		edited[i] = c;
		c = tmp;
		edits[count++] = str_dup(edited);
	}
	
	free(edited);
	
	return edits;
}

HashTable *build_dictionary_hash(List *words)
{
	HashTable *ht = new_hash((words->size ? words->size : 1));
							// ensure at least 1 slot
							// x % 0 is bad 
	// insert all words into dict_ht
	int i = 0;
	for (Node *n = words->head; n; n = n->next)
	{
		// add word with data corresponding to the location in dict
		hash_insert(ht, n->data, i++);
	}
	
	return ht;
}

char *get_correction_hash(char *word, HashTable *dict, int dist)
{
	// base case
	// check if word is in dict
	if (dist <= 0)
	{
		return hash_key(dict, word);
	}

	// else, enumerate edits of distance 1, and recursively check them
	char *corrected = NULL, **edits = get_edits(word);
	int pos = -1;
	for (int i = 0, max = EDIT_COUNT(strlen(word)); i < max; i++)
	{
		char *edit = edits[i];
		// check for correction at dist-1 of each edit
		char *new_correction = get_correction_hash(edit, dict, dist-1); 
		// retain the edit that occurs first in the dictionary List 
		if (new_correction) 
		{
			int new_pos = hash_get(dict, new_correction);
			if (!corrected || pos > new_pos)
			{
				corrected = new_correction;
				pos = new_pos;
			}
		}
		free(edit);
	}
	
	free(edits);
	
	return corrected;
}

char *get_correction_list(char *word, List *dict, int dist)
{
	char *corrected = NULL;
	int min_dist = MAX_DIST+1, len = strlen(word);
	// loop over dict list
	for (Node *n = dict->head; n; n = n->next)
	{
		char *node_word = n->data;
		// heuristic: edit_dist(a,b) >= ||a|-|b||
		if (abs(strlen(node_word)-len) < min_dist)
		{
			int e_dist = edit_dist(node_word, word);
			// if we find a node_word <= dist away from word
			if (e_dist <= dist)
			{
				return node_word;
			}
			// retain node_word if it is < distant than our current correction
			else if (e_dist < min_dist)
			{
				min_dist = e_dist;
				corrected = node_word;
			}
		}
	}

	return corrected;
}

char *str_dup(char *src)
{
	char *dest;
	assert(dest = malloc(sizeof(char) * (strlen(src)+1)));
	strcpy(dest, src);
	
	return dest;
}
