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
#define C2S_VECTOR3_SIZE 12
int WriteVector3ToServer(uint8_t **stream, size_t *len, Vector3 *obj);
int ReadVector3FromServer(uint8_t **stream, size_t *len, Vector3 *obj);
#define S2C_VECTOR3_SIZE 12
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
#define C2S_PLAYER_SIZE 28
int WritePlayerToServer(uint8_t **stream, size_t *len, Player *obj);
int ReadPlayerFromServer(uint8_t **stream, size_t *len, Player *obj);
#define S2C_PLAYER_SIZE 30
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
int WriteChunkToServer(uint8_t **stream, size_t *len, Chunk *obj);
int ReadChunkFromServer(uint8_t **stream, size_t *len, Chunk *obj);
#define S2C_CHUNK_SIZE 8192
typedef enum PlayerPacketType
{
    PLAYER_DATA_PACKET,
    PLAYER_AUTH_PACKET,
    PLAYER_VERB_PACKET,
}
PlayerPacketType;
