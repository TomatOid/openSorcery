#if defined(_WIN32) || defined(_WIN64)
    #include "enet/enet.h"
    #define SDL_MAIN_HANDLED
    #include "SDL.h"
    // if on windows, you will also need the corresponding .lib files in the directory with build_windows.bat
#else
    #include <enet/enet.h>
    #include <SDL2/SDL.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "generated_serialize_client.h"
#include "HashTable.h"
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

int playerToStateAtTime(Player *player, uint32_t time_tick_number)
{
    for (int i = player->write_index - 1; i > player->write_index - STATE_BUFFER_SIZE; i--)
    {
        PlayerState *state_lower = &player->state_buffer[i % STATE_BUFFER_SIZE];
        if (state_lower->command_id <= time_tick_number)
        {
            if (state_lower->command_id < time_tick_number && i < player->write_index - 1)
            {
                Vector3 position;
                PlayerState *state_upper = &player->state_buffer[i + 1 % STATE_BUFFER_SIZE];
                position.x = (state_lower->position.x * (time_tick_number - state_lower->command_id) 
                    + state_upper->position.x * (state_upper->command_id - time_tick_number)) / (state_upper->command_id - state_lower->command_id);
                position.y = (state_lower->position.y * (time_tick_number - state_lower->command_id) 
                    + state_upper->position.y * (state_upper->command_id - time_tick_number)) / (state_upper->command_id - state_lower->command_id);
                position.z = (state_lower->position.z * (time_tick_number - state_lower->command_id) 
                    + state_upper->position.z * (state_upper->command_id - time_tick_number)) / (state_upper->command_id - state_lower->command_id);
                player->position = position;
            }
            else
            {
                player->position = state_lower->position;
            }
            return 1;
        }
    }
    return 0;
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

void freePlayerByUUID(HashTable *player_table, BlockPage *player_page, Player **loaded_players_list, size_t *number_of_loaded_players, uint64_t unload_uuid)
{
    void *old_player = removeFromTable(player_table, unload_uuid);
    if (old_player)
    {
        for (int i = 0; i < *number_of_loaded_players; i++)
        {
            if (loaded_players_list[i] == old_player)
            {
                // replace with the end of the list
                loaded_players_list[i] = loaded_players_list[(*number_of_loaded_players)--];
                loaded_players_list[*number_of_loaded_players] = NULL;
                break;
            }
        }
    blockFree(player_page, old_player);
    }
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

    // set up storage for the players
    static BlockPage player_page;
    makePage(&player_page, MAX_CLIENTS, sizeof(Player));
    Player *loaded_players_list[MAX_CLIENTS];
    size_t number_of_loaded_players = 0;

    static HashTable player_table = { 0 };
    player_table.len = MAX_CLIENTS;
    player_table.items = calloc(player_table.len, sizeof(HashItem *));
    makePage(&player_table.page, player_table.len, sizeof(HashItem));

    // Now setup SDL
    if (SDL_Init(SDL_INIT_EVERYTHING)) { fprintf(stderr, "There was an error initializing SDL"); exit(EXIT_FAILURE); }
    SDL_Window *game_window;
    SDL_Renderer *main_renderer;
    SDL_CreateWindowAndRenderer(256, 256, SDL_WINDOW_SHOWN, &game_window, &main_renderer);
    uint16_t button_state = 0;
    uint32_t actual_ticks_per_second = TARGET_TICKS_PER_SECOND;
    uint32_t interpolation_ticks = 4;
    uint32_t complete_cycle_by = SDL_GetTicks() + 1000 / actual_ticks_per_second;
    uint32_t client_tick_count = interpolation_ticks;
    for (;;)
    {
        SDL_Event user_event;
        // Take user input
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
        // now draw every other player
        for (int i = 0; i < number_of_loaded_players; i++)
        {
            if (1 || playerToStateAtTime(loaded_players_list[i], client_tick_count - interpolation_ticks))
            {
                SDL_RenderDrawPoint(main_renderer, loaded_players_list[i]->position.x, loaded_players_list[i]->position.z);
            }
            else
            {
                freePlayerByUUID(&player_table, &player_page, loaded_players_list, &number_of_loaded_players, loaded_players_list[i]->uuid);
            }
        }

        // Recieve packets from the server for the rest of the time
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
                        {
                            PlayerState recieved_state = {0};
                            readPlayerStateFromServer(&stream, &length, &recieved_state);
                            if (recieved_state.target_uuid == this_player.uuid)
                            {
                                playerChangePastStateThenPropagate(&this_player, recieved_state);
                            }
                            else
                            {
                                Player *target_player = findInTable(&player_table, recieved_state.target_uuid);
                                // if we have not seen it yet, create a new space for it
                                if (!target_player)
                                {
                                    target_player = blockAlloc(&player_page);
                                    if (!target_player) break;
                                    // we are going to use a seperate state counter to make interpolation easier
                                    // as it will guarantee that each state is sequential with no skipping
                                    // this may cause jumps due to packets arriving out-of-order and may need to be changed
                                    target_player->read_index = 0;
                                    target_player->write_index = 0;
                                    // maybe if it is full we need to purge the BlockPage
                                    insertToTable(&player_table, recieved_state.target_uuid, target_player);

                                    loaded_players_list[number_of_loaded_players] = target_player;
                                    number_of_loaded_players++; 
                                }
                                recieved_state.command_id = client_tick_count;
                                target_player->position = recieved_state.position;
                                target_player->state_buffer[target_player->write_index % STATE_BUFFER_SIZE] = recieved_state;
                                target_player->write_index++;
                                // SDL_RenderDrawPoint(main_renderer, recieved_state.position.x, recieved_state.position.z);
                            }
                            break;
                        }
                        case PLAYER_UNLOAD_PACKET:
                        {
                            uint64_t unload_uuid;
                            if (length < sizeof(unload_uuid)) { break; }
                            memcpy(&unload_uuid, stream, sizeof(unload_uuid));
                            // I will not increment any more as there is no more to copy
                            freePlayerByUUID(&player_table, &player_page, loaded_players_list, &number_of_loaded_players, unload_uuid);
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
        client_tick_count++;
    }
}