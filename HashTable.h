#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "BlockAllocate.h"

typedef struct HashItem
{
    uint64_t key;
    void* value;
    struct HashItem* next;
} HashItem;

// A seprate chaining hash table struct
typedef struct HashTable
{
    HashItem** items;
    BlockPage page;
    size_t len;
    size_t num;
} HashTable;

bool insertToTable(HashTable* table, uint64_t key, void* value)
{
    size_t hash = key % table->len;
    // using a block allocator for this instead of
    // using calloc to avoid heap fragmentation
    HashItem* curr;
    if (!(curr = blkalloc(&table->page))) return false;
    curr->key = key;
    curr->value = value;
    // insert the item at the beginning of the linked list
    curr->next = table->items[hash];
    table->items[hash] = curr;
    table->num++;
    return true;
}

void* findInTable(HashTable* table, uint64_t key)
{
    size_t hash = key % table->len;
    HashItem* curr = table->items[hash];

    while (curr && curr->key != key) { curr = curr->next; }

    if (curr) { return curr->value; }
    else { return NULL; }
}

void* removeFromTable(HashTable* table, uint64_t key)
{
    size_t hash = key % table->len;
    HashItem* curr = table->items[hash];
    HashItem* prev = NULL;
    // loop untill either we hit the end of the list or the keys match
    while (curr && curr->key != key) { prev = curr; curr = curr->next; }

    if (curr) 
    { 
        HashItem* after = (curr)->next;
        void* value = (curr)->value;
        if (prev) (prev)->next = after;
        if (!blkfree(&table->page, curr)) exit(EXIT_FAILURE);
        if (!prev) table->items[hash] = after;
        table->num--;
        return value;
    }
    else return NULL;
}
