#include <stdlib.h>
#include "HashTable.h"
#include "BlockAllocate.h"

void insertToTable(hashTable* table, uint64_t key, void* value)
{
    size_t hash = key % table->len;
    hashItem* curr = table->items[hash];

    // insert the item at the end of the linked list
    // this implementation assumes that 
    while (curr) { curr = curr->next; }
    // TODO: implement a block allocator for this instead of
    // using calloc to avoid heap fragmentation
    if (!(curr = blkalloc(table->page))) return;
    curr->key = key;
    curr->value = value;
    curr->next = NULL;
    table->num++;
}

void* findInTable(hashTable* table, uint64_t key)
{
    size_t hash = key % table->len;
    hashItem* curr = table->items[hash];

    while (curr && curr->key != key) { curr = curr->next; }

    if (curr) { return curr->value; }
    else { return NULL; }
}

void* removeFromTable(hashTable* table, uint64_t key)
{
    size_t hash = key % table->len;
    hashItem* curr = table->items[hash];

    while (curr->next && curr->next->key != key) { curr = curr->next; }

    if (curr->next) 
    { 
        hashItem* after = curr->next->next;
        void* value = curr->next->value;
        blkfree(table->page, curr->next);
        curr->next = after;
        return value;
    }
    else return NULL;
}