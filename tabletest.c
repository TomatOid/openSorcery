#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "BlockAllocate.h"
#include "HashTable.h"

int main(int argc, char** argv)
{
    hashTable table = { 0 };
    table.len = 30;
    makePage(&table.page, table.len, sizeof(hashItem));
    table.items = calloc(table.len, sizeof(hashItem*));

    uint64_t keys[] = { 403720, 548697, 515602, 768233, 907616, 176406, 322897, 4296, 151607, 539100, 399007, 94369, 750098, 69929, 126589, 4829, 771313, 251611, 200890, 723033, 243563, 353468, 215221, 14491, 99999 };
    for (int n = 0; n < 10; n++)
    {    
        for (int i = 0; i < sizeof(keys) / sizeof(uint64_t); i++)
        {
            if (!insertToTable(&table, keys[i], &keys)) { printf("there was an error adding element %d\n", i); }
        }
        printf("insert test finished\n");
        for (int i = 0; i < sizeof(keys) / sizeof(uint64_t); i++)
        {
            if (!findInTable(&table, keys[i])) { printf("there was an error finding element %d\n", i); }
        }
        printf("find test finished\n");
        for (int i = 0; i < sizeof(keys) / sizeof(uint64_t); i++)
        {
            if (!removeFromTable(&table, keys[i])) { printf("there was an error deleting element %d\n", i); }
        }
        printf("remove test passed\n");
    }
}   