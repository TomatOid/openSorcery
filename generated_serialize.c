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
