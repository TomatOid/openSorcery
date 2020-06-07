#include <enet/enet.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "generated_serialize_client.h"

int main(int argc, char* argv[])
{
    // same enet initialization, as perscribed by the enet gods
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occured when starting enet, aborting.\n");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    ENetHost* client;

    client = enet_host_create(NULL, 1, 1, 0, 0);

    if (!client)
    {
        fprintf(stderr, "An error occured while trying to create the host\n");
        return EXIT_FAILURE;
    }

    ENetAddress address;
    ENetEvent event;
    ENetPeer* peer;
    Player this_player = { 0 };

    if (argc < 4) { printf("Usage: HOSTNAME PORT USERNAME\n"); return -1; }
    else 
    {
        if (atoi(argv[2]) == 0) { printf("PORT must be a nonzero integer\n"); return -1; }
    }
    
    if (enet_address_set_host(&address, argv[1]) < 0) { printf("An error occured while parsing the HOSTNAME.\n"); return -1; }
    address.port = atoi(argv[2]);
    if (strlen(argv[3]) >= sizeof(this_player.name)) { printf("USERNAME must be less than 20 characters\n"); return -1; }
    strncpy(this_player.name, argv[3], sizeof(this_player.name));
    this_player.uuid = (uint64_t)rand();

    peer = enet_host_connect(client, &address, 1, 0);
    // now check if the connection failed
    if (!peer) { fprintf(stderr, "There was an issue connecting to the host\n"); return -1; }
    //if (enet_host_service(client, &event, 1000) <= 0) { printf("A response was not recieved\n"); exit(0); }
    for (;;)
    {
        if (enet_host_service(client, &event, 50) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    printf("A connection was made to address: %x\n", event.peer->address.host);
                    ENetPacket *packet = enet_packet_create(NULL, C2S_PLAYER_SIZE + sizeof(uint32_t), 0);
                    // make copies of the data pointer and packet length as stream-writing will mutate the copies
                    // if we passed them directly, we wouldn't know where the packet started
                    uint8_t *packet_data = packet->data;
                    size_t length = packet->dataLength;
                    uint32_t packet_type = PLAYER_DATA_PACKET;
                    // copy in the packet type
                    memcpy(packet_data, &packet_type, sizeof(packet_type));
                    packet_data += sizeof(packet_type);
                    length -= sizeof(packet_type);

                    WritePlayerToServer(&packet_data, &length, &this_player);
                    enet_peer_send(event.peer, 0, packet);
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    printf("Recieved a packet of length %u containing %s", event.packet->dataLength, event.packet->data);
                    break;
                default:
                    break;
            }
        }

    }
}