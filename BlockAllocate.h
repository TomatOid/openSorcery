#pragma once
#include <stdint.h>

// this structure contains all of the relevant state for
// a simple block allocator, which can be used when many
// items of the same size need to be allocated dynamically
typedef struct BlockPage
{
    void* pool;
    void** free;
    __ssize_t top;
    size_t blksize;
    size_t numblk;
} BlockPage;

void* blkalloc(BlockPage* page);

int blkfree(BlockPage* page, void* blk);

void makePage(BlockPage* res, size_t num_blocks, size_t block_size);