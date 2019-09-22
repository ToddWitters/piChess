#include "stdint.h"

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

// An entry in the table is a key/value pair.  The key must be unique
struct entry_s
{
   char *key;
   char *value;
   struct entry_s *next;  // Pointer to next entry in this list
};

typedef struct entry_s entry_t;

// The handle for the hash table.  Consists only of a size indicator
//   and a pointer to an array of pointers to linked lists (i.e. entry_t)
typedef struct hashtable_s
{
   uint16_t size;
   entry_t **table;
}hashtable_t;

// Return types
typedef enum hashtableErr_e
{
   HT_NO_ERROR,
   HT_FILE_ERROR,
   HT_NULL_PTR,
   HT_MEM_ERROR,
   HT_DUP_KEY,
   HT_KEY_NOT_FOUND,
   HT_INVALID_KEY
}hashtableErr_t;

// API

// Create a new, empty hash table
hashtable_t*   ht_create(uint16_t size);

// Delete hashtable and free its memory
hashtableErr_t ht_destroy(hashtable_t *ht);

// Delete a key from the table
hashtableErr_t ht_delKey(hashtable_t *ht, char *key);

// Set a key/value pair.  If key already exists, updates the value
hashtableErr_t ht_setKey(hashtable_t *ht, char *key, char *value);

// Returns the value associated with a key.  NULL indicates the key wasn't found
char*          ht_getKey(hashtable_t *ht, char *key );

// read/write hashtable to memory
hashtableErr_t ht_save(hashtable_t *ht, char *htName);
hashtableErr_t ht_load(hashtable_t *ht, char *htName);

// Return number of entries in table
uint32_t       ht_countEntries(hashtable_t *ht);

// Resize hashtable for optimal performance
hashtable_t *ht_optimize(hashtable_t *ht, uint32_t maxSize, uint8_t maxDepth);

#endif
