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
typedef enum PlayerPacketType
{
    PLAYER_DATA_PACKET,
    PLAYER_AUTH_PACKET,
    PLAYER_VERB_PACKET,
}
PlayerPacketType;
