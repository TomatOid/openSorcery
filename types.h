#pragma once
#include <stdint.h>

// Enumerate all the different packet types
enum 
{
    // For sending a player's state, from the client, data is only accepted about that client's
    // username, UUID, and settings; however the server can send all of the relevant info including
    // Position, velocity, equipment / items, alligance, as well as everything the client is allowed to send.
    PLAYER_DATA_PACKET,
    // For authenticating wether or not the peer has any business controlling the specified UUID
    // May use RSA signing for this and store public keys in the database with other player data
    // The server will send a challenge packet which the client is to sign and return, and the server
    // will verify and reset the peer if they do not respond or respond incorrectly
    PLAYER_AUTH_PACKET,
    // This is for actions such as foreward key pressed etc. which are likely to be exclusively client -> server
    PLAYER_VERB_PACKET
};

typedef struct Player
{
    uint64_t uuid;
    char name[20]; // this may need to be changed to a uint8_t
} Player;