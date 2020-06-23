#ifndef BLOCKALLOCATE_H_INCLUDED
#define BLOCKALLOCATE_H_INCLUDED
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdatomic.h>

// this structure contains all of the relevant state for
// a simple block allocator, which can be used when many
// items of the same size need to be allocated dynamically
typedef struct
{
    void* pool;
    void** free;
    ptrdiff_t top;
    size_t blksize;
    size_t numblk;
} BlockPage;

#include <string.h>

void* blockAlloc(BlockPage* page)
{
    // atomically get a block location
    ptrdiff_t my_block = page->top--;
    // now, we can check if we got a valid block, and release it if it is invalid
    if (my_block < 0 || my_block >= (ptrdiff_t)page->numblk)
    {
        page->top++;
        return NULL; 
    }
    void* blk = page->free[my_block];

    memset(blk, 0, page->blksize);
    return blk;
}

int blockFree(BlockPage* page, void* blk)
{
    ptrdiff_t my_block = page->top++;
    if (my_block >= (ptrdiff_t)page->numblk)
    {
        page->top--;
        return 0; // if this returns 0, there has likely been a double free
    }
    page->free[my_block] = blk;
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