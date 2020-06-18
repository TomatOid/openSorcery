#include <enet/enet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <SDL2/SDL_timer.h>
#include "HashTable.h"
#include "BlockAllocate.h"
#include "generated_serialize_server.h"
#define MAX_CLIENTS 4000
#define CLIENT_VIEW_MAX 3
#define MAX_CHANNELS 2

int input_is_ready = 0;
pthread_cond_t await_response_cond;
pthread_mutex_t await_response_mutex;
int argument_count = 0;
char *argument_pointers[256];

// this function is called by a seperate thread and takes input
// TODO: Add ncurses
void *inputHandle(void *a)
{
    char command_buffer[256];
    for (;;)
    {
        pthread_mutex_lock(&await_response_mutex);
        // wait for the main thread to be finished with the last command
        while (input_is_ready) pthread_cond_wait(&await_response_cond, &await_response_mutex);
        // reset all buffers to their zeroed state
        memset(command_buffer, 0, sizeof(command_buffer));
        memset(argument_pointers, 0, sizeof(argument_pointers));
        argument_count = 0;
        // take input
        fgets(command_buffer, sizeof(command_buffer) - 1, stdin);
        char* last_seperator = command_buffer;
        // loop over the buffer, replacing spaces and line breaks with null, while also recording their locations
        for (int i = 0; i < sizeof(command_buffer) / sizeof(*command_buffer) && argument_count < sizeof(argument_pointers) / sizeof(*argument_pointers); i++)
        {
            if (command_buffer[i] == ' ' || command_buffer[i] == '\n')
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
    pthread_create(&inputThread, NULL, inputHandle, NULL);

    // do the enet initialization as perscribed by the enet gods
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occured when starting enet, aborting.\n");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);
    ENetEvent event;
    ENetAddress address;
    ENetHost *server;
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
    // as well as a way to keep track of online players
    // let's set that up now
    static BlockPage player_page;
    makePage(&player_page, MAX_CLIENTS, sizeof(Player));
    // we need a way for clients and the server 
    // to communicate which player they are talking
    // about, so each player will have a unique id,
    // and we can figure out where the associated
    // player data is by using a hash table 
    static HashTable player_by_UUID;
    player_by_UUID.len = 2 * MAX_CLIENTS;
    makePage(&player_by_UUID.page, player_by_UUID.len, sizeof(HashItem));
    player_by_UUID.items = calloc(player_by_UUID.len, sizeof(HashItem*));
    // Each player can load up to CLIENT_VIEW_MAX * CLIENT_VIEW_MAX chunks, let's allocate all of those
    static BlockPage chunks_page;
    makePage(&chunks_page, MAX_CLIENTS * CLIENT_VIEW_MAX * CLIENT_VIEW_MAX, sizeof(Chunk));
    // if the server is running behind, this value may be changed
    uint32_t actual_ticks_per_second = TARGET_TICKS_PER_SECOND;
    uint32_t complete_cycle_by = SDL_GetTicks() + 1000 / actual_ticks_per_second;
    while (1)
    {
        // Do one tick
        for (int i = 0; i < server->peerCount; i++)
        {
            ENetPeer *target_peer = &server->peers[i];
            if (target_peer->data)
            {
                Player *target_player = (Player *)target_peer->data;
                if (target_player->read_index < target_player->write_index)
                {
                    PlayerState *last_command = &target_player->state_buffer[target_player->read_index % STATE_BUFFER_SIZE];
                    // first, make sure the player is following all the rules
                    if (abs(last_command->x_velocity) > PLAYER_MAX_VELOCITY / 2)
                    {
                        last_command->x_velocity = (last_command->x_velocity > 0) ? PLAYER_MAX_VELOCITY / 2 : -PLAYER_MAX_VELOCITY / 2;
                    }
                    if (abs(last_command->z_velocity) > PLAYER_MAX_VELOCITY)
                    {
                        last_command->z_velocity = (last_command->z_velocity > 0) ? PLAYER_MAX_VELOCITY / 2 : -PLAYER_MAX_VELOCITY / 2;
                    }
                    // TODO: once chunks are added, add a check to make sure the player is on the ground when jumping
                    // this might also be where we check whether the player has entered or left a chunk
                    // perhaps we could do the chunk update after we broadcast so that then the recieving clients
                    // will know this player needs to be unloaded because the player's coordinates are outside of its view
                    target_player->position.x += last_command->x_velocity * (TARGET_TICKS_PER_SECOND / actual_ticks_per_second);
                    target_player->position.z += last_command->z_velocity * (TARGET_TICKS_PER_SECOND / actual_ticks_per_second);
                    last_command->position = target_player->position;
                    last_command->target_uuid = target_player->uuid;
                    uint8_t player_state_buffer[S2C_PLAYER_STATE_SIZE + sizeof(uint32_t)];
                    uint8_t *stream = player_state_buffer;
                    size_t length = sizeof(player_state_buffer);
                    uint32_t packet_type = PLAYER_VERB_PACKET;
                    memcpy(stream, &packet_type, sizeof(packet_type));
                    stream += sizeof(packet_type);
                    writePlayerStateToClient(&stream, &length, last_command);
                    // TODO: should we pre-allocate a pool for these packets instead?
                    // at the moment, enet does an allocate and then copies the buffer into that memory
                    ENetPacket *target_player_state_packet = enet_packet_create(player_state_buffer, sizeof(player_state_buffer), 0);
                    // TODO: broadcast only to the players who are visable
                    enet_host_broadcast(server, 0, target_player_state_packet);
                    target_player->read_index++;
                }
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
        // respond to events untill the time is up
        while (SDL_GetTicks() < complete_cycle_by)
        {
            if (enet_host_service(server, &event, 1) > 0)
            {
                switch (event.type)
                {
                case ENET_EVENT_TYPE_CONNECT:
                    // Now, set the data to NULL so we know that this peer is uninitalized
                    // then when a packet is recieved, we know it should be a PLAYER_DATA_PACKET
                    event.peer->data = NULL;
                    enet_peer_timeout(event.peer, 0, 0, 0);
                    if (event.peer->data) { enet_peer_reset(event.peer); break; }
                    printf("A connection request was recieved from address: %x\n", event.peer->address.host);
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    if (event.packet->dataLength < sizeof(uint32_t))
                    {
                        printf("A packet that was too short was recieved from address: %x\n", event.peer->address.host);
                        break;
                    }
                    // copy the first four bytes, which are reserved for the packet type
                    uint32_t packet_type;
                    memcpy(&packet_type, event.packet->data, sizeof(uint32_t));
                    uint8_t* packet_data = event.packet->data + sizeof(uint32_t);
                    size_t length = event.packet->dataLength - sizeof(uint32_t);
                    if (event.peer->data)
                    {
                        switch (packet_type)
                        {
                        case PLAYER_VERB_PACKET:
                            ((Player *)event.peer->data)->write_index++;
                            readPlayerStateFromClient(&packet_data, &length, &((Player *)event.peer->data)->state_buffer[((Player *)event.peer->data)->write_index % STATE_BUFFER_SIZE]);
                            break;
                        
                        default:
                            break;
                        }
                    }
                    else
                    {
                        if (packet_type != PLAYER_DATA_PACKET)
                        {
                            printf("A non-data packet was recieved from an uninitialized player from address: %x\n", event.peer->address.host);
                            break;
                        }
                        else
                        {
                            // allocate room for a new player and read the allowed info from the client
                            // TODO: clean this up a bit with a temp variable
                            Player *new_player = blockAlloc(&player_page);

                            if (!new_player)
                            {
                                enet_peer_reset(event.peer);
                                break;
                            }
                            readPlayerFromClient(&packet_data, &length, new_player);
                            // set the last byte to NULL for safety
                            new_player->name[sizeof(new_player->name) - 1]= '\0';
                            insertToTable(&player_by_UUID, new_player->uuid, new_player);
                            printf("%s joined the server with uuid %lu!\n", new_player->name, new_player->uuid);

                            event.peer->data = new_player;
                        }
                    }
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    if (event.peer->data)
                    {
                        printf("Player %s disconnected.\n", ((Player *)event.peer->data)->name);
                        removeFromTable(&player_by_UUID, ((Player *)event.peer->data)->uuid);
                        if (!blockFree(&player_page, event.peer->data)) exit(EXIT_FAILURE);
                    } else printf("A connection attempt failed.\n");
                    break;
                default:
                    break;
                }
            }
        }
        complete_cycle_by += 1000 / actual_ticks_per_second;
    }
}

