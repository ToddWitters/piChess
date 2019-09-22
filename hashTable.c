#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "diag.h"

#include "hashtable.h"

// VISUALIZATION OF THE HASH TABLE....
//                                                           +-+-+-+-+-+--+
//                                                       +-->|m y K e y \0|
//                                                       |   +-+-+-+-+-+--+
//                                                       |
//                                                       |       +-+-+-+-+-+-+-+--+
//    hashTable_t                                        |   +-->|m y V a l u e \0|
// +---------------+                                     |   |   +-+-+-+-+-+-+-+--+
// |     size      |      entry_t*[]           entry_t   |   |                       +-+-+-+-+--+
// +---------------+      +-------+           +-------+  |   |        +-------+  +-->|k e y 2 \0|
// |     table     |----->|       |---------->|  key  |--+   |    +-->|  key  |--+   +-+-+-+-+--+
// +---------------+      +-------+           +-------+      |    |   +-------+   +-+-+-+-+--+
//                        |       |--x        | value |------+    |   | value |-->|v a l 2 \0|
//                        +-------+           +-------+           |   +-------+   +-+-+-+-+--+
//                      / |       |--x        |  nxt  |-----------+   |  nxt  |--x
//                     /  +-------+           +-------+               +-------+
//                    /       .
//                   /        .
// size elements ___/         .
//                        +-------+
//                        |       |--x
//                        +-------+
//


// Helper Functions

// returns a hash index for given key and table size
static uint16_t ht_hash(uint16_t tableSize, char *key);

// Creates an entry and returns the pointer.  NULL return
//   indicates a failure.
static entry_t *ht_createEntry( char *key, char *value );

// Keys should have no spaces...
static bool confirmValidKey( char *key );

// Helper functions for optimization...
static uint16_t ht_getIndex(hashtable_t *ht, char *key);
static uint16_t ht_getDepth(hashtable_t *ht, uint16_t index);

// API FUNCTIONS...

// Create a hash table of given size
hashtable_t *ht_create(uint16_t size)
{
   hashtable_t *hashtable;
   uint16_t i;

   // Sanity check the size...
   if( size < 1 )
      return NULL;

   // Create the memory for the handle
   if( ( hashtable = malloc( sizeof( hashtable_t ) ) ) == NULL )
      return NULL;

   // Create the memory for the table it points to
   if( ( hashtable->table = malloc( sizeof( entry_t* ) * size ) ) == NULL )
   {
      // On error, release the hash table handle we already allocated
      free(hashtable);
      return NULL;
   }

   // Set the size
   hashtable->size = size;

   // Set all linked-list pointers to NULL
   for( i = 0; i < size; i++ ) hashtable->table[i] = NULL;

   // Return the pointer
   return hashtable;
}


// Destroy a hash table
hashtableErr_t ht_destroy(hashtable_t *ht)
{
   uint16_t i;

   // Sanity check...
   if(ht == NULL) return HT_NULL_PTR;

   // for each linked list
   for(i=0; i < ht->size; i++)
   {
      // Point to top of list
      entry_t *ptr = ht->table[i];

      // While more items in list
      while(ptr != NULL)
      {
         // Grab the next pointer before we do anything else...
         entry_t *next = ptr->next;

         // Free the space taken by the key and value strings
         free(ptr->key);
         free(ptr->value);

         // Free the entry itself
         free(ptr);

         // move to the next one
         ptr = next;
      }
   }

   // Free the table
   free(ht->table);

   // Free the handle...
   free(ht);

   return HT_NO_ERROR;
}

// Delete a key/value pair from the hashtable
hashtableErr_t ht_delKey(hashtable_t *hashtable, char *key)
{
   uint16_t hash;

   // Sanity check
   if(hashtable == NULL) return HT_NULL_PTR;

   if(!confirmValidKey(key)) return HT_INVALID_KEY;

   // Hash the key
   hash = ht_hash(hashtable->size, key);

   // Point both this and previous to the first item in the list
   entry_t *thisEntry = hashtable->table[hash];
   entry_t *prevEntry = thisEntry;

   // Look for end of list or match...
   while(thisEntry != NULL)
   {
      // Bail if we have a match
      if(!strcmp(thisEntry->key, key)) break;

      // Move to next
      prevEntry = thisEntry;
      thisEntry = thisEntry->next;
   }

   // If we found a match, thisEntry will be non-null
   if(thisEntry != NULL)
   {
      // Was this the top of the list?
      if(thisEntry == hashtable->table[hash])
         // Yes...
         hashtable->table[hash] = thisEntry->next;
      else
         // Point the "next" pointer of the previous entry to the
         //   same place this entry was pointing, skipping this one.
         prevEntry->next = thisEntry->next;

      // Free the entry...
      free(thisEntry->key);
      free(thisEntry->value);
      free(thisEntry);
      return HT_NO_ERROR;
   }
   else
      return HT_KEY_NOT_FOUND;
}

// Set a key/value pair to the hash table.  If key already exists, the
//   value will be updated .
hashtableErr_t ht_setKey( hashtable_t *hashtable, char *key, char *value)
{

   uint16_t hash;
   // Sanity check
   if(hashtable == NULL)     return HT_MEM_ERROR;
   if(!confirmValidKey(key)) return HT_INVALID_KEY;

   // Point both this and previous to the first item in the list
   entry_t *thisEntry = hashtable->table[(hash = ht_hash(hashtable->size, key))];
   entry_t *prevEntry = thisEntry;

   // Look for end of list or match...
   while(thisEntry != NULL)
   {
      if(!strcmp(thisEntry->key, key)) break;
      prevEntry = thisEntry;
      thisEntry = thisEntry->next;
   }

   // If we had a match, replace data...
   if(thisEntry != NULL)
   {
      // Free existing value, replace with new value
      free(thisEntry->value);
      if((thisEntry->value = strdup(value)) == NULL)
         return HT_MEM_ERROR;
      else
         return HT_DUP_KEY;
   }

   else
   {
      // No matches... add a new entry

      entry_t *tmp;

      if((tmp = ht_createEntry(key,value)) == NULL)
         return HT_MEM_ERROR;

      // If we started with a NULL list...
      if(prevEntry == NULL)
         // Set the pointer to this (only) entry
         hashtable->table[hash] = tmp;
      else
         // Set the next item in the list to point here...
         prevEntry->next = tmp;

      return HT_NO_ERROR;
   }
}

// Get the value for a specific key
char *ht_getKey( hashtable_t *hashtable, char *key )
{
   entry_t *thisEntry;

   // Sanity checks
   if(hashtable == NULL)     return NULL;
   if(!confirmValidKey(key)) return NULL;

   // Start walking pointer at first item in linked list
   thisEntry = hashtable->table[ht_hash(hashtable->size, key)];

   // Search until found or end of list reached...
   while( thisEntry != NULL && (strcmp(thisEntry->key, key) != 0) )
      thisEntry = thisEntry->next;

   // If we hit the end of the list, just return
   if(thisEntry == NULL) return NULL;

   // Else return the value.
   else                  return thisEntry->value;
}

// Return number of entries
uint32_t ht_countEntries(hashtable_t *ht)
{
   uint32_t total = 0;
   uint32_t i;

   if(ht == NULL) return 0;

   for(i=0;i<ht->size;i++)
   {
      entry_t *walkingPointer = ht->table[i];
      while(walkingPointer)
      {
         total++;
         walkingPointer = walkingPointer->next;
      }
   }
   return total;
}

#if 0
// Optimize an existing hash table by finding a solution using <= maxSize bins AND
//   <= maxDepth entries in every bin.  Return pointer to new solution.  Will return
//   original if solution cannot be found
hashtable_t *ht_optimize(hashtable_t *ht, uint32_t maxSize, uint8_t maxDepth)
{

   hashtable_t *newTable;
   uint32_t testSize;
   bool solutionFound = false;
   bool bail = false;

   // Bail out on errors
   if(ht == NULL) return NULL;
   if(ht->size == 0) return NULL;

   // printf("Looking for solution with a maximum of %d bins all with maximum depth of %d\n", maxSize, maxDepth);

   testSize = ht_countEntries(ht) / maxDepth;

   // Keep going until we find a size that doesn't require multiple entries in a "bin"
   while(testSize <= maxSize)
   {
      int i;

      // Create a new, temporary hash table...
      newTable = ht_create(testSize);

      bail = false;

      // Walk through our existing table, copying each entry into the new hash table
      for(i=0;i<ht->size;i++)
      {
         // If this index is not null
         if(ht->table[i] != NULL)
         {
            // walk through all the entries...
            entry_t *walkingPointer = ht->table[i];
            while(walkingPointer && !bail)
            {
               // Add the key to the new table.
               ht_setKey(newTable, walkingPointer->key, walkingPointer->value);

               // Find the depth at this new insertion point
               if(ht_getDepth(newTable, ht_getIndex(newTable, walkingPointer->key)) > maxDepth)
               {
                  bail = true;
                  break; // break out of while loop
               }
               walkingPointer = walkingPointer->next;
            }

            if(bail == true)
               break; // break out of for loop
         }
      }

      if(bail == true)
      {
         // this isn't the best solution yet
         ht_destroy(newTable);
      }
      else
      {
         // Good to go..
         solutionFound = true;
         ht_destroy(ht);
         break; // break out of while loop
      }

      testSize++;
   }

   if(solutionFound == true)
   {
//      printf("Optimized table has capacity of %d with max depth of %d\n", newTable->size, maxDepth);
      return newTable;
   }
   else
   {
//      printf("Optimal solution not found\n");
      return ht;
   }
}

#endif

hashtableErr_t ht_save(hashtable_t *ht, char *htName)
{
   FILE *output;
   int i;

   if( ht == NULL)
      return HT_NULL_PTR;

   if( (output = fopen(htName, "w")) == NULL)
      return HT_FILE_ERROR;

   // Walk through our existing, copying each entry into the new hash table
   for(i=0;i<(ht->size);i++)
   {
      // walk through all the entries...
      entry_t *walkingPointer = ht->table[i];
      while(walkingPointer)
      {
         fprintf(output,"%s %s\n", walkingPointer->key, walkingPointer->value);
         walkingPointer = walkingPointer->next;
      }
   }

   fclose(output);
   return HT_NO_ERROR;

}

#define MAX_LINE_LEN 200
hashtableErr_t ht_load(hashtable_t *ht, char *htName)
{

   char lineContents[MAX_LINE_LEN];

   FILE *input;

   if( ht == NULL)
      return HT_NULL_PTR;

   if( (input = fopen(htName, "r")) == NULL)
   {
      DPRINT("File error on ht_load\n");
      return HT_FILE_ERROR;
   }

   while(!feof(input))
   {
      // Holds the key and value token pointers
      char* key;
      char* value;

      // grab next line from input file
      if( fgets(lineContents, MAX_LINE_LEN + 1, input) == NULL ) break;

      // Parse the two tokens on the line.  Should be a key followed by a value
      key = strtok(lineContents, " ");

      // If there was a problem, move on....
      if(key == NULL) continue;

      // Try to extract the value string
      value = strtok(NULL,"\n\r\0");

      // If there was a problem, move on....
      if(value == NULL) continue;

      ht_setKey(ht, key, value);
   }

   fclose(input);
   return HT_NO_ERROR;
}


//////////////////// LOCAL HELPER FUNCTIONS ////////////////////////

static bool confirmValidKey( char *key )
{
   while(*key != ' ' && *key != '\0') key++;

   return(*key == '\0');
}

// Given a table size and a character string, find a hash value
static uint16_t ht_hash(uint16_t tableSize, char *key)
{
   uint32_t hash = 5381;
   uint8_t c;

   while ( (c = *key++) )
       hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

   return hash % tableSize;
}

// Allocate space for an entry and return it...
static entry_t *ht_createEntry( char *key, char *value )
{
   entry_t *newEntry;

   if( ( newEntry = malloc( sizeof( entry_t ) ) ) == NULL )
      return NULL;

   if( ( newEntry->key = strdup( key ) ) == NULL )
   {
      free(newEntry);
      return NULL;
   }

   if( ( newEntry->value = strdup( value ) ) == NULL )
   {
      free(newEntry->key);
      free(newEntry);
      return NULL;
   }

   newEntry->next = NULL;

   return newEntry;
}

#if 0
static uint16_t ht_getIndex(hashtable_t *ht, char *key)
{
   return ht_hash(ht->size, key);
}

static uint16_t ht_getDepth(hashtable_t *ht, uint16_t index)
{
   uint16_t depth = 0;
   entry_t *walkingPointer = ht->table[index];

   while(walkingPointer)
   {
      depth++;
      walkingPointer = walkingPointer->next;
   }
   return depth;
}
#endif
