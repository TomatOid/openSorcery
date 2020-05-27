#include <enet/enet.h>
#include <stdio.h>

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

    if (argc < 3) { printf("Usage: HOSTNAME PORT\n"); return -1; }
    else 
    {
        if (atoi(argv[2]) == 0) { printf("PORT must be a nonzero integer\n"); return -1; }
    }
    
    if (enet_address_set_host(&address, argv[1]) < 0) { printf("An error occured while parsing the HOSTNAME.\n"); return -1; }
    address.port = atoi(argv[2]);

    peer = enet_host_connect(client, &address, 1, 0);
    // now check if the connection failed
    if (!peer) { fprintf(stderr, "There was an issue connecting to the host\n"); return -1; }
    //if (enet_host_service(client, &event, 1000) <= 0) { printf("A response was not recieved\n"); exit(0); }
    while (enet_host_service(client, &event, 1000) > 0)
    {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                printf("A connection was made to address: %x\n", event.peer->address.host);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                printf("Recieved a packet of length %u containing %s", event.packet->dataLength, event.packet->data);
                break;
            default:
                break;
        }
    }
}