#include <enet/enet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include "HashTable.h"
#include "BlockAllocate.h"
#include "generated_serialize_server.h"
int MAX_CLIENTS = 20;
int MAX_CHANNELS = 2;

int input_is_ready = 0;
pthread_cond_t await_response_cond;
pthread_mutex_t await_response_mutex;
int argument_count = 0;
char command_buffer[256];
char *argument_pointers[256];

void* inputHandle(void* a)
{
    for (;;)
    {
        pthread_mutex_lock(&await_response_mutex);
        while (input_is_ready) pthread_cond_wait(&await_response_cond, &await_response_mutex);
        memset(command_buffer, 0, sizeof(command_buffer));
        memset(argument_pointers, 0, sizeof(argument_pointers));
        argument_count = 0;
        fgets(command_buffer, sizeof(command_buffer) - 1, stdin);
        char* last_seperator = command_buffer;
        for (int i = 0; i < sizeof(command_buffer) / sizeof(*command_buffer) && argument_count < sizeof(argument_pointers) / sizeof(*argument_pointers); i++)
        {
            if (!command_buffer[i] || command_buffer[i] == ' ' || command_buffer[i] == '\n')
            {
                argument_pointers[argument_count] = last_seperator;
                argument_count++;
                command_buffer[i] = '\0';
                last_seperator = &command_buffer[i];
            }
        }
        input_is_ready = 1;
        pthread_mutex_unlock(&await_response_mutex);
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    // split into two threads, the new thread handling input
    pthread_cond_init(&await_response_cond, NULL);
    pthread_mutex_init(&await_response_mutex, NULL);
    pthread_t inputThread;
    pthread_create(inputThread, NULL, inputHandle, NULL);

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
    // we need a place to store all of the player information
    // let's set that up now
    BlockPage playerPage;
    makePage(&playerPage, MAX_CLIENTS, sizeof(Player));
    // we need a way for clients and the server 
    // to communicate which player they are talking
    // about, so each player will have a unique id,
    // and we can figure out where the associated
    // player data is by using a hash table 
    HashTable playerByUUID;
    playerByUUID.len = 2 * MAX_CLIENTS;
    makePage(&playerByUUID.page, playerByUUID.len, sizeof(HashItem));
    playerByUUID.items = calloc(playerByUUID.len, sizeof(HashItem*));
    while (1)
    {
        if (enet_host_service(server, &event, 20) >= 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    // Now, set the data to NULL so we know that this peer is uninitalized
                    // then when a packet is recieved, we know it should be a PLAYER_DATA_PACKET
                    event.peer->data = NULL;
                    enet_peer_timeout(event.peer, 0, 0, 0);
                    printf("A connection request was recieved from address: %x\n", event.peer->address.host);
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
                    size_t length = event.packet->dataLength - sizeof(uint32_t);
                    if (!event.peer->data)
                    {
                        if (packetType != PLAYER_DATA_PACKET)
                        {
                            printf("A non-data packet was recieved from an uninitialized player from address: %x\n", event.peer->address.host);
                            break;
                        }
                        else
                        {
                            // allocate room for a new player and read the allowed info from the client
                            event.peer->data = blkalloc(&playerPage);
                            if (!event.peer->data)
                            {
                                enet_peer_reset(event.peer);
                                break;
                            }
                            ReadPlayerFromClient(&packetData, &length, event.peer->data);
                            // set the last bit to NULL for safety
                            ((Player *)event.peer->data)->name[sizeof(((Player *)0)->name) - 1] = '\0';
                            printf("%s joined the server!\n", ((Player *)event.peer->data)->name);
                        }
                    }
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    if (event.peer->data)
                    {
                        printf("Player %s disconnected.\n", ((Player *)event.peer->data)->name);
                        if (!blkfree(&playerPage, event.peer->data)) exit(EXIT_FAILURE);
                    } else printf("A connection attempt failed.\n");
                    break;
                default:
                    break;
            }
        }
        if (input_is_ready)
        {
            pthread_mutex_lock(&await_response_mutex);
            // handle the command
            printf("got command %s\n", argument_pointers[0]);
            input_is_ready = 0;
            pthread_cond_signal(&await_response_cond);
            pthread_mutex_unlock(&await_response_mutex);
        }
    }
}

