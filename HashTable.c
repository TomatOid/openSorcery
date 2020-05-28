#include <stdlib.h>
#include "HashTable.h"
#include "BlockAllocate.h"

bool insertToTable(hashTable* table, uint64_t key, void* value)
{
    size_t hash = key % table->len;
    // using a block allocator for this instead of
    // using calloc to avoid heap fragmentation
    if (!(curr = blkalloc(&table->page))) return false;
    curr->key = key;
    curr->value = value;
    // insert the item at the beginning of the linked list
    curr->next = table->items[hash];
    table->items[hash] = curr;
    table->num++;
    return true;
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
    hashItem* prev = NULL;
    // loop untill either we hit the end of the list or the keys match
    while (curr && curr->key != key) { prev = curr; curr = curr->next; }

    if (curr) 
    { 
        hashItem* after = (curr)->next;
        void* value = (curr)->value;
        if (prev) (prev)->next = after;
        if (!blkfree(&table->page, curr)) return NULL;
        if (!prev) table->items[hash] = after;
        return value;
    }
    else return NULL;
}