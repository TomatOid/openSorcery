#define CHUNK_WIDTH (16)
#define CHUNK_HEIGHT (32)
#define STATE_BUFFER_SIZE (8)
#define TARGET_TICKS_PER_SECOND (20)
#define PLAYER_MAX_VELOCITY (3)
#define MAX_CLIENTS (2)
#define CLIENT_VIEW_MAX (3)
enum
{
    FOREWARD_BUTTON = (1<<0),
    BACK_BUTTON = (1<<1),
    LEFT_BUTTON = (1<<2),
    RIGHT_BUTTON = (1<<3),
    JUMP_BUTTON = (1<<4),
    ATTACK_BUTTON = (1<<5),
};
typedef unsigned int ButtonFlag;
// @Serializeable 
typedef struct Vector3 Vector3;
struct Vector3
{
    // @c2s @s2c 
    int32_t x;
    // @c2s @s2c 
    int32_t y;
    // @c2s @s2c 
    int32_t z;
};
int readVector3FromClient(uint8_t **stream, size_t *len, Vector3 *obj);
#define C2S_VECTOR3_SIZE 12
#define S2C_VECTOR3_SIZE 12
int writeVector3ToClient(uint8_t **stream, size_t *len, Vector3 *obj);
// @Serializeable 
typedef struct PlayerState PlayerState;
struct PlayerState
{
    // @c2s @s2c 
    uint32_t command_id;
    // @s2c 
    uint64_t target_uuid;
    // @s2c 
    Vector3 position;
    // @c2s @s2c 
    uint8_t on_ground;
    // @c2s 
    uint16_t button_flags;
    // @c2s @s2c 
    int32_t x_velocity;
    // @s2c 
    int32_t y_velocity;
    // @c2s @s2c 
    int32_t z_velocity;
};
int readPlayerStateFromClient(uint8_t **stream, size_t *len, PlayerState *obj);
#define C2S_PLAYER_STATE_SIZE 15
#define S2C_PLAYER_STATE_SIZE 25
int writePlayerStateToClient(uint8_t **stream, size_t *len, PlayerState *obj);
// @Serializeable 
typedef struct Player Player;
struct Player
{
    // @c2s @s2c 
    char name[20];
    // @c2s @s2c 
    uint64_t uuid;
    // @s2c 
    int16_t health;
    // @s2c 
    Vector3 position;
    // @s2c 
    Vector3 velocity;
    // @s2c 
    uint8_t on_ground;
    uint32_t write_index;
    uint32_t read_index;
    PlayerState state_buffer[STATE_BUFFER_SIZE];
};
int readPlayerFromClient(uint8_t **stream, size_t *len, Player *obj);
#define C2S_PLAYER_SIZE 28
#define S2C_PLAYER_SIZE 31
int writePlayerToClient(uint8_t **stream, size_t *len, Player *obj);
typedef struct PlayerLinkedListNode PlayerLinkedListNode;
struct PlayerLinkedListNode
{
    Player *player_pointer;
    PlayerLinkedListNode *next;
};
// @Serializeable 
typedef struct Chunk Chunk;
struct Chunk
{
    // @s2c 
    uint8_t blocks[((CHUNK_WIDTH*CHUNK_WIDTH)*CHUNK_HEIGHT)];
    PlayerLinkedListNode *PlayerLinkedList;
};
int readChunkFromClient(uint8_t **stream, size_t *len, Chunk *obj);
#define C2S_CHUNK_SIZE 0
#define S2C_CHUNK_SIZE 8192
int writeChunkToClient(uint8_t **stream, size_t *len, Chunk *obj);
// @Serializeable 
typedef enum PlayerPacketType
{
    PLAYER_DATA_PACKET,
    PLAYER_AUTH_PACKET,
    PLAYER_VERB_PACKET,
    PLAYER_UNLOAD_PACKET,
}
PlayerPacketType;
