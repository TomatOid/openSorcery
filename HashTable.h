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

typedef struct hashTable
{
    hashItem** items;
    BlockPage page;
    size_t len;
    size_t num;
} hashTable;

//void insertHashItem(hashTable* table, long key, void* value, long timestamp);
//int findHashItem(hashTable* table, long key, long timestamp, long maxage);
