#ifndef BLOCKALLOCATE_H_INCLUDED
#define BLOCKALLOCATE_H_INCLUDED
#include <stdint.h>
#include <stdlib.h>

// this structure contains all of the relevant state for
// a simple block allocator, which can be used when many
// items of the same size need to be allocated dynamically
typedef struct
{
    void* pool;
    void** free;
    __ssize_t top;
    size_t blksize;
    size_t numblk;
} BlockPage;

#include <string.h>

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
    if (page->top >= (__ssize_t)page->numblk) { return 0; } // possible double free
    // check if blk is within the page's memory pool and is alligned properly
    if (blk < page->pool || blk >= page->pool + page->numblk * page->blksize) { return 0; } // blk was outside of the valid reigon
    if (((intptr_t)blk - (intptr_t)page->pool) % page->blksize != 0) { return 0; } // blk was not alligned
    
    page->free[page->top++] = blk;
    return 1;
}

void makePage(BlockPage* res, size_t num_blocks, size_t block_size)
{
    // rounding block_size up to be a multiple of size void* to avoid any potential allignment issues
    block_size = ((block_size + sizeof(void*) - 1) / sizeof(void*)) * sizeof(void*);
    // allocate all the memory
    res->pool = calloc(num_blocks, block_size);
    res->free = calloc(num_blocks, sizeof(void*));

    // now we need to fill up the free stack 
    intptr_t ptr = (intptr_t)res->pool;
    for (size_t i = 0; i < num_blocks; i++)
    {
        res->free[num_blocks - i - 1] = (void*)ptr;
        ptr += block_size;
    }
    res->top = num_blocks - 1;
    res->numblk = num_blocks;
    res->blksize = block_size;
} 
#endif