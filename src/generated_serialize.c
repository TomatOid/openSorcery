#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "generated_serialize_client.h"
// make sure that *stream is not the only copy of the memory location
// of the start of the stream, else this will cause a memory leak!
int cpyFromStream(uint8_t** stream, size_t* len, void* dest, size_t nbytes)
{
    if (nbytes > *len) { return 0; }
    memcpy(dest, *stream, nbytes);
    *stream += nbytes;
    *len -= nbytes;
    return 1;
}
int writeToStream(uint8_t** stream, size_t* len, void* src, size_t nbytes)
{
    if (nbytes > *len) { return 0; }
    memcpy(*stream, src, nbytes);
    *stream += nbytes;
    *len -= nbytes;
    return 1;
}
int readVector3FromClient(uint8_t **stream, size_t *len, Vector3 *obj)
{
    if (!cpyFromStream(stream, len, &obj->x, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->y, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->z, 4)) return 0;
    return 1;
}
int writeVector3ToServer(uint8_t **stream, size_t *len, Vector3 *obj)
{
    if (!writeToStream(stream, len, &obj->x, 4)) return 0;
    if (!writeToStream(stream, len, &obj->y, 4)) return 0;
    if (!writeToStream(stream, len, &obj->z, 4)) return 0;
    return 1;
}
int readVector3FromServer(uint8_t **stream, size_t *len, Vector3 *obj)
{
    if (!cpyFromStream(stream, len, &obj->x, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->y, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->z, 4)) return 0;
    return 1;
}
int writeVector3ToClient(uint8_t **stream, size_t *len, Vector3 *obj)
{
    if (!writeToStream(stream, len, &obj->x, 4)) return 0;
    if (!writeToStream(stream, len, &obj->y, 4)) return 0;
    if (!writeToStream(stream, len, &obj->z, 4)) return 0;
    return 1;
}
int readPlayerStateFromClient(uint8_t **stream, size_t *len, PlayerState *obj)
{
    if (!cpyFromStream(stream, len, &obj->command_id, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->on_ground, 1)) return 0;
    if (!cpyFromStream(stream, len, &obj->button_flags, 2)) return 0;
    if (!cpyFromStream(stream, len, &obj->x_velocity, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->z_velocity, 4)) return 0;
    return 1;
}
int writePlayerStateToServer(uint8_t **stream, size_t *len, PlayerState *obj)
{
    if (!writeToStream(stream, len, &obj->command_id, 4)) return 0;
    if (!writeToStream(stream, len, &obj->on_ground, 1)) return 0;
    if (!writeToStream(stream, len, &obj->button_flags, 2)) return 0;
    if (!writeToStream(stream, len, &obj->x_velocity, 4)) return 0;
    if (!writeToStream(stream, len, &obj->z_velocity, 4)) return 0;
    return 1;
}
int readPlayerStateFromServer(uint8_t **stream, size_t *len, PlayerState *obj)
{
    if (!cpyFromStream(stream, len, &obj->command_id, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->target_uuid, 8)) return 0;
    if (!cpyFromStream(stream, len, &obj->position.x, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->position.y, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->position.z, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->on_ground, 1)) return 0;
    if (!cpyFromStream(stream, len, &obj->x_velocity, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->y_velocity, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->z_velocity, 4)) return 0;
    return 1;
}
int writePlayerStateToClient(uint8_t **stream, size_t *len, PlayerState *obj)
{
    if (!writeToStream(stream, len, &obj->command_id, 4)) return 0;
    if (!writeToStream(stream, len, &obj->target_uuid, 8)) return 0;
    if (!writeToStream(stream, len, &obj->position.x, 4)) return 0;
    if (!writeToStream(stream, len, &obj->position.y, 4)) return 0;
    if (!writeToStream(stream, len, &obj->position.z, 4)) return 0;
    if (!writeToStream(stream, len, &obj->on_ground, 1)) return 0;
    if (!writeToStream(stream, len, &obj->x_velocity, 4)) return 0;
    if (!writeToStream(stream, len, &obj->y_velocity, 4)) return 0;
    if (!writeToStream(stream, len, &obj->z_velocity, 4)) return 0;
    return 1;
}
int readPlayerFromClient(uint8_t **stream, size_t *len, Player *obj)
{
    if (!cpyFromStream(stream, len, &obj->name, 20)) return 0;
    if (!cpyFromStream(stream, len, &obj->uuid, 8)) return 0;
    return 1;
}
int writePlayerToServer(uint8_t **stream, size_t *len, Player *obj)
{
    if (!writeToStream(stream, len, &obj->name, 20)) return 0;
    if (!writeToStream(stream, len, &obj->uuid, 8)) return 0;
    return 1;
}
int readPlayerFromServer(uint8_t **stream, size_t *len, Player *obj)
{
    if (!cpyFromStream(stream, len, &obj->name, 20)) return 0;
    if (!cpyFromStream(stream, len, &obj->uuid, 8)) return 0;
    if (!cpyFromStream(stream, len, &obj->health, 2)) return 0;
    if (!cpyFromStream(stream, len, &obj->position.x, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->position.y, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->position.z, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->velocity.x, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->velocity.y, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->velocity.z, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->on_ground, 1)) return 0;
    return 1;
}
int writePlayerToClient(uint8_t **stream, size_t *len, Player *obj)
{
    if (!writeToStream(stream, len, &obj->name, 20)) return 0;
    if (!writeToStream(stream, len, &obj->uuid, 8)) return 0;
    if (!writeToStream(stream, len, &obj->health, 2)) return 0;
    if (!writeToStream(stream, len, &obj->position.x, 4)) return 0;
    if (!writeToStream(stream, len, &obj->position.y, 4)) return 0;
    if (!writeToStream(stream, len, &obj->position.z, 4)) return 0;
    if (!writeToStream(stream, len, &obj->velocity.x, 4)) return 0;
    if (!writeToStream(stream, len, &obj->velocity.y, 4)) return 0;
    if (!writeToStream(stream, len, &obj->velocity.z, 4)) return 0;
    if (!writeToStream(stream, len, &obj->on_ground, 1)) return 0;
    return 1;
}
int readChunkFromClient(uint8_t **stream, size_t *len, Chunk *obj)
{
    return 1;
}
int writeChunkToServer(uint8_t **stream, size_t *len, Chunk *obj)
{
    return 1;
}
int readChunkFromServer(uint8_t **stream, size_t *len, Chunk *obj)
{
    if (!cpyFromStream(stream, len, &obj->blocks, 8192)) return 0;
    return 1;
}
int writeChunkToClient(uint8_t **stream, size_t *len, Chunk *obj)
{
    if (!writeToStream(stream, len, &obj->blocks, 8192)) return 0;
    return 1;
}
