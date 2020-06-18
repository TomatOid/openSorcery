#include <enet/enet.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "generated_serialize_client.h"
#include <SDL2/SDL.h>
#include <time.h>

void playerToLatestState(Player *player)
{
    while (player->read_index < player->write_index)
    {
        PlayerState *current_read_state = &player->state_buffer[player->read_index++ % STATE_BUFFER_SIZE];
        // TODO: Compensate for slower TPS
        player->position.x += current_read_state->x_velocity;
        player->position.z += current_read_state->z_velocity;
        current_read_state->position = player->position;
    }
}

int playerChangePastStateThenPropagate(Player *player, PlayerState correct_past_state)
{
    // check if this state is intended for the player
    //if (correct_past_state.target_uuid != player->uuid) return 0;
    PlayerState *past_state = &player->state_buffer[correct_past_state.command_id % STATE_BUFFER_SIZE];
    // check if the incorrect version is still in the buffer
    // we are just assuming that the past state is incorrect
    if (past_state->command_id == correct_past_state.command_id)
    {
        // go to that past position
        player->position = correct_past_state.position;
        // set the incorrect state to the correct state
        *past_state = correct_past_state;
        player->read_index = correct_past_state.command_id;
        while (player->read_index < player->write_index)
        {
            PlayerState *current_read_state = &player->state_buffer[player->read_index++ % STATE_BUFFER_SIZE];
            // TODO: Compensate for slower TPS
            player->position.x += current_read_state->x_velocity;
            player->position.z += current_read_state->z_velocity;
            current_read_state->position = player->position;
        }
        return 1;
    } 
    else return 0;
}

int writeHeaderToStream(uint8_t **stream, size_t *length, uint32_t type_header)
{
    if (*length < sizeof(type_header)) { return 0; }
    memcpy(*stream, &type_header, sizeof(type_header));
    *stream += sizeof(type_header);
    *length -= sizeof(type_header);
    return 1;
}

uint32_t readHeaderFromStream(uint8_t **stream, size_t *length)
{
    uint32_t result = 0xFFFFFFFF;
    if (*length < sizeof(uint32_t)) { return result; }
    memcpy(&result, *stream, sizeof(uint32_t));
    *stream += sizeof(uint32_t);
    *length -= sizeof(uint32_t);
    return result;
}

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
    srand(enet_time_get());
    this_player.uuid = (uint64_t)rand();
    printf("uuid: %lu\n", this_player.uuid);

    peer = enet_host_connect(client, &address, 1, 0);
    enet_peer_timeout(peer, 0, 0, 0);
    // now check if the connection failed
    if (!peer) { fprintf(stderr, "There was an issue connecting to the host\n"); return -1; }
    // if (enet_host_service(client, &event, 1000) <= 0) { printf("A response was not recieved\n"); exit(0); }

    // Now setup SDL
    if (SDL_Init(SDL_INIT_EVERYTHING)) { fprintf(stderr, "There was an error initializing SDL"); exit(EXIT_FAILURE); }
    SDL_Window *game_window;
    SDL_Renderer *main_renderer;
    SDL_CreateWindowAndRenderer(256, 256, SDL_WINDOW_SHOWN, &game_window, &main_renderer);
    uint16_t button_state = 0;
    uint32_t actual_ticks_per_second = TARGET_TICKS_PER_SECOND;
    uint32_t complete_cycle_by = SDL_GetTicks() + 1000 / actual_ticks_per_second;
    for (;;)
    {
        SDL_Event user_event;
        while (SDL_PollEvent(&user_event))
        {
            switch (user_event.type)
            {
            case SDL_QUIT:
                enet_peer_disconnect_now(&client->peers[0], 0);
                exit(EXIT_SUCCESS);
            case SDL_KEYDOWN:
                switch (user_event.key.keysym.sym)
                {
                case SDLK_w:
                    button_state |= FOREWARD_BUTTON;
                    break;
                case SDLK_a:
                    button_state |= LEFT_BUTTON;
                    break;
                case SDLK_s:
                    button_state |= BACK_BUTTON;
                    break;
                case SDLK_d:
                    button_state |= RIGHT_BUTTON;
                    break;
                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (user_event.key.keysym.sym)
                {
                case SDLK_w:
                    button_state &= 0xffff ^ (uint16_t)FOREWARD_BUTTON;
                    break;
                case SDLK_a:
                    button_state &= 0xffff ^ (uint16_t)LEFT_BUTTON;
                    break;
                case SDLK_s:
                    button_state &= 0xffff ^ (uint16_t)BACK_BUTTON;
                    break;
                case SDLK_d:
                    button_state &= 0xffff ^ (uint16_t)RIGHT_BUTTON;
                    break;
                default:
                    break;
                }
                break;
            }
        }
        // TODO: Maybe we don't send a packet if there is no change in state, and just store it
        // right now, we are sending an command every frame regardless
        this_player.write_index++;
        //printf("%d %d\n", this_player.read_index, this_player.write_index);
        PlayerState *new_player_state = &this_player.state_buffer[this_player.write_index % STATE_BUFFER_SIZE];
        new_player_state->command_id = this_player.write_index;
        new_player_state->button_flags = button_state;
        if (button_state & FOREWARD_BUTTON) { new_player_state->z_velocity = -PLAYER_MAX_VELOCITY; }
        else if (button_state & BACK_BUTTON) { new_player_state->z_velocity = PLAYER_MAX_VELOCITY; }
        else { new_player_state->z_velocity = 0; }
        if (button_state & LEFT_BUTTON) { new_player_state->x_velocity = -PLAYER_MAX_VELOCITY; }
        else if (button_state & RIGHT_BUTTON) { new_player_state->x_velocity = PLAYER_MAX_VELOCITY; }
        else { new_player_state->x_velocity = 0; }
        // now, we make predictions about our position, which we will check against what we get back from the server later
        playerToLatestState(&this_player);
        // now send it off
        uint8_t player_state_buffer[C2S_PLAYER_STATE_SIZE + sizeof(uint32_t)];
        size_t player_state_length = sizeof(player_state_buffer);
        uint8_t *stream = player_state_buffer;
        writeHeaderToStream(&stream, &player_state_length, PLAYER_VERB_PACKET);
        writePlayerStateToServer(&stream, &player_state_length, new_player_state);
        enet_peer_send(&client->peers[0], 0, enet_packet_create(player_state_buffer, sizeof(player_state_buffer), 0));
        // just draw a pixel where the player should be
        SDL_SetRenderDrawColor(main_renderer, 0, 0, 0, 0);
        SDL_RenderClear(main_renderer);
        SDL_SetRenderDrawColor(main_renderer, 255, 255, 255, 0);
        SDL_RenderDrawPoint(main_renderer, this_player.position.x, this_player.position.z);
        //printf("%d, %d\n", this_player.position.x, this_player.position.y);


        while (SDL_GetTicks() < complete_cycle_by)
        {
            if (enet_host_service(client, &event, 1) >= 0)
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

                        writePlayerToServer(&packet_data, &length, &this_player);
                        enet_peer_send(event.peer, 0, packet);
                        break;
                    case ENET_EVENT_TYPE_RECEIVE:
                        stream = event.packet->data;
                        length = event.packet->dataLength;
                        switch (readHeaderFromStream(&stream, &length))
                        {
                        case PLAYER_VERB_PACKET:
                            ;
                            PlayerState recieved_state = {0};
                            readPlayerStateFromServer(&stream, &length, &recieved_state);
                            if (recieved_state.target_uuid == this_player.uuid)
                            {
                                playerChangePastStateThenPropagate(&this_player, recieved_state);
                            }
                            else
                            {
                                SDL_RenderDrawPoint(main_renderer, recieved_state.position.x, recieved_state.position.z);
                            }
                        }
                        break;
                    case ENET_EVENT_TYPE_DISCONNECT:
                        exit(EXIT_SUCCESS);
                    default:
                        break;
                }
            }
            else exit(EXIT_FAILURE);
        }
        complete_cycle_by += 1000 / actual_ticks_per_second;
        SDL_RenderPresent(main_renderer);
    }
}