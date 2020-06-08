#define CHUNK_WIDTH (16)
#define CHUNK_HEIGHT (32)
// @Serializeable 
typedef struct Vector3 Vector3;
struct Vector3
{
    // @c2s @s2c 
    uint32_t x;
    // @c2s @s2c 
    uint32_t y;
    // @c2s @s2c 
    uint32_t z;
};
int ReadVector3FromClient(uint8_t **stream, size_t *len, Vector3 *obj);
#define C2S_VECTOR3_SIZE 12
#define S2C_VECTOR3_SIZE 12
int WriteVector3ToClient(uint8_t **stream, size_t *len, Vector3 *obj);
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
};
int ReadPlayerFromClient(uint8_t **stream, size_t *len, Player *obj);
#define C2S_PLAYER_SIZE 28
#define S2C_PLAYER_SIZE 30
int WritePlayerToClient(uint8_t **stream, size_t *len, Player *obj);
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
int ReadChunkFromClient(uint8_t **stream, size_t *len, Chunk *obj);
#define C2S_CHUNK_SIZE 0
#define S2C_CHUNK_SIZE 8192
int WriteChunkToClient(uint8_t **stream, size_t *len, Chunk *obj);
typedef enum PlayerPacketType
{
    PLAYER_DATA_PACKET,
    PLAYER_AUTH_PACKET,
    PLAYER_VERB_PACKET,
}
PlayerPacketType;
