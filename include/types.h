#pragma once
#include <stdint.h>

typedef struct Player
{
    uint64_t uuid;
    char name[20]; // this may need to be changed to a uint8_t
} Player;