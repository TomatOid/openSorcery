#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "BlockAllocate.h"

typedef struct hashItem
{
    uint64_t key;
    void* value;
    struct hashItem* next;
} hashItem;

// A seprate chaining hash table implememtation
typedef struct hashTable
{
    hashItem** items;
    BlockPage page;
    size_t len;
    size_t num;
} hashTable;

bool insertToTable(hashTable* table, uint64_t key, void* value);
void* findInTable(hashTable* table, uint64_t key);
void* removeFromTable(hashTable* table, uint64_t key);
