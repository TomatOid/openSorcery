#define CHUNK_WIDTH (16)
#define CHUNK_HEIGHT (32)
#define STATE_BUFFER_SIZE (8)
#define TARGET_TICKS_PER_SECOND (20)
#define PLAYER_MAX_VELOCITY (3)
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
#define C2S_VECTOR3_SIZE 12
int writeVector3ToServer(uint8_t **stream, size_t *len, Vector3 *obj);
int readVector3FromServer(uint8_t **stream, size_t *len, Vector3 *obj);
#define S2C_VECTOR3_SIZE 12
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
#define C2S_PLAYER_STATE_SIZE 15
int writePlayerStateToServer(uint8_t **stream, size_t *len, PlayerState *obj);
int readPlayerStateFromServer(uint8_t **stream, size_t *len, PlayerState *obj);
#define S2C_PLAYER_STATE_SIZE 25
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
    uint64_t last_update_tick;
};
#define C2S_PLAYER_SIZE 28
int writePlayerToServer(uint8_t **stream, size_t *len, Player *obj);
int readPlayerFromServer(uint8_t **stream, size_t *len, Player *obj);
#define S2C_PLAYER_SIZE 31
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
#define C2S_CHUNK_SIZE 0
int writeChunkToServer(uint8_t **stream, size_t *len, Chunk *obj);
int readChunkFromServer(uint8_t **stream, size_t *len, Chunk *obj);
#define S2C_CHUNK_SIZE 8192
// @Serializeable 
typedef enum PlayerPacketType
{
    PLAYER_DATA_PACKET,
    PLAYER_AUTH_PACKET,
    PLAYER_VERB_PACKET,
}
PlayerPacketType;
