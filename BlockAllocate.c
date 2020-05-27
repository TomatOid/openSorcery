#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include "BlockAllocate.h"

void* blkalloc(BlockPage* page)
{
    if (page->top < 0) { return NULL; }
    void* blk = page->free[page->top];
    page->top--;

    memset(blk, 0, page->blksize);
    return blk;
}

int blkfree(BlockPage* page, void* blk)
{
    if (page->top >= page->numblk) { return 0; } // possible double free
    // check if blk is within the page's memory pool and is alligned properly
    if (blk < page->pool || blk >= page->pool + page->numblk * page->blksize) { return 0; } // blk was outside of the valid reigon
    if (((intptr_t)blk - (intptr_t)page->pool) % page->blksize != 0) { return 0; } // blk was not alligned
    
    page->free[page->top++] = blk;
    return 1;
}

void makePage(BlockPage* res, size_t num_blocks, size_t block_size)
{
    // using posix_memallign so that the result of blkalloc can be simply cast
    // to a type of size block_size without being worried about allignment
    posix_memalign(&res->pool, block_size, num_blocks * block_size);
    res->free = calloc(num_blocks, sizeof(void*));

    // now we need to fill up the free stack 
    intptr_t ptr = res->pool;
    for (size_t i = num_blocks - 1; i >= 0; i--)
    {
        res->free[i] = (void*)ptr;
        ptr += block_size;
    }
    res->top = num_blocks - 1;
    res->numblk = num_blocks;
    res->blksize = block_size;
} 