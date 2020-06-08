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
int ReadVector3FromClient(uint8_t **stream, size_t *len, Vector3 *obj)
{
    if (!cpyFromStream(stream, len, &obj->x, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->y, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->z, 4)) return 0;
    return 1;
}
int WriteVector3ToServer(uint8_t **stream, size_t *len, Vector3 *obj)
{
    if (!writeToStream(stream, len, &obj->x, 4)) return 0;
    if (!writeToStream(stream, len, &obj->y, 4)) return 0;
    if (!writeToStream(stream, len, &obj->z, 4)) return 0;
    return 1;
}
int ReadVector3FromServer(uint8_t **stream, size_t *len, Vector3 *obj)
{
    if (!cpyFromStream(stream, len, &obj->x, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->y, 4)) return 0;
    if (!cpyFromStream(stream, len, &obj->z, 4)) return 0;
    return 1;
}
int WriteVector3ToClient(uint8_t **stream, size_t *len, Vector3 *obj)
{
    if (!writeToStream(stream, len, &obj->x, 4)) return 0;
    if (!writeToStream(stream, len, &obj->y, 4)) return 0;
    if (!writeToStream(stream, len, &obj->z, 4)) return 0;
    return 1;
}
int ReadPlayerFromClient(uint8_t **stream, size_t *len, Player *obj)
{
    if (!cpyFromStream(stream, len, &obj->name, 20)) return 0;
    if (!cpyFromStream(stream, len, &obj->uuid, 8)) return 0;
    return 1;
}
int WritePlayerToServer(uint8_t **stream, size_t *len, Player *obj)
{
    if (!writeToStream(stream, len, &obj->name, 20)) return 0;
    if (!writeToStream(stream, len, &obj->uuid, 8)) return 0;
    return 1;
}
int ReadPlayerFromServer(uint8_t **stream, size_t *len, Player *obj)
{
    if (!cpyFromStream(stream, len, &obj->name, 20)) return 0;
    if (!cpyFromStream(stream, len, &obj->uuid, 8)) return 0;
    if (!cpyFromStream(stream, len, &obj->health, 2)) return 0;
    return 1;
}
int WritePlayerToClient(uint8_t **stream, size_t *len, Player *obj)
{
    if (!writeToStream(stream, len, &obj->name, 20)) return 0;
    if (!writeToStream(stream, len, &obj->uuid, 8)) return 0;
    if (!writeToStream(stream, len, &obj->health, 2)) return 0;
    return 1;
}
int ReadChunkFromClient(uint8_t **stream, size_t *len, Chunk *obj)
{
    return 1;
}
int WriteChunkToServer(uint8_t **stream, size_t *len, Chunk *obj)
{
    return 1;
}
int ReadChunkFromServer(uint8_t **stream, size_t *len, Chunk *obj)
{
    if (!cpyFromStream(stream, len, &obj->blocks, 8192)) return 0;
    return 1;
}
int WriteChunkToClient(uint8_t **stream, size_t *len, Chunk *obj)
{
    if (!writeToStream(stream, len, &obj->blocks, 8192)) return 0;
    return 1;
}
