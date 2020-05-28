#include <enet/enet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "HashTable.h"
#include "types.h"
int MAX_CLIENTS = 32;
int MAX_CHANNELS = 2;

int main(int argc, char* argv[])
{
    // do the enet initialization as perscribed by the enet gods

    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occured when starting enet, aborting.\n");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    ENetEvent event;

    ENetAddress address;
    ENetHost* server;

    // We are going to check if this program has been run with sufficient args
    // And if none are provided, we assume port 2222,
    // else if the args are invalid, return an error and abort.

    if (argc < 2)
    {
        printf("No port provided, assuming port 2222\n");
        address.port = 2222;
    } 
    else if (atoi(argv[1]))
    {
        address.port = atoi(argv[1]);
    }
    else
    {
        printf("Usage: %s PORT, where PORT is a nonzero short\n", argv[0]);
        return -1;
    }
    address.host = ENET_HOST_ANY;

    server = enet_host_create(&address, MAX_CLIENTS, MAX_CHANNELS, 0, 0);
    

    while (enet_host_service(server, &event, 1000) >= 0)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                printf("A connection request was recieved from address: %x\n", event.peer->address.host);
                // Now, set the data to NULL so we know that this peer is uninitalized
                // then when a packet is recieved, we know it should be a PLAYER_DATA_PACKET
                event.peer->data = NULL;
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                if (event.packet->dataLength < sizeof(uint32_t))
                {
                    printf("A packet that was too short was recieved from address: %x\n", event.peer->address.host);
                    break;
                }
                // copy the first four bytes, which are reserved for the packet type
                uint32_t packetType;
                memcpy(&packetType, event.packet->data, sizeof(uint32_t));
                uint8_t* packetData = event.packet->data + sizeof(uint32_t);
                size_t len = event.packet->dataLength - sizeof(uint32_t);
                size_t idx = 0;
                if (!event.peer->data)
                {
                    if (packetType != PLAYER_DATA_PACKET)
                    {
                        printf("A non-data packet was recieved from an uninitialized player from address: %x\n", event.peer->address.host);
                        break;
                    }
                    
                }
                break;
            default:
                break;
        }
    }
}

