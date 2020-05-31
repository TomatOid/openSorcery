#include <stdio.h>
#include <string.h>
#include "data_desk.h"

#define ACCESS_STRING_SIZE 128

static FILE *client_header_file = 0;
static FILE *server_header_file = 0;
static FILE *global_implementation_file = 0;

enum Mode { MODE_READ, MODE_WRITE };

static void generateSerializationCode(FILE *file, DataDeskNode *root, char *access_string, char *tag, enum Mode mode);

char *transmitable_types[] = { "char", "int8_t", "uint8_t", "int16_t", "int32_t", "uint32_t", "int64_t", "uint64_t", "float", "double" };

DATA_DESK_FUNC void
DataDeskCustomInitCallback(void)
{
	// Initialization code goes here.
    client_header_file = fopen("generated_serialize_client.h", "w");
    server_header_file = fopen("generated_serialize_server.h", "w");
    global_implementation_file = fopen("generated_serialize.c", "w");
    fprintf(global_implementation_file, "#include <stdint.h>\n");
    fprintf(global_implementation_file, "#include <stdlib.h>\n");
    fprintf(global_implementation_file, "#include \"generated_serialize_client.h\"\n");
    fprintf(global_implementation_file, "#include \"generated_serialize_server.h\"\n");
    fprintf(global_implementation_file, 
    "int cpyFromStream(uint8_t** stream, size_t* len, void* dest, size_t nbytes)\n"
    "{\n"
    "    if (nbytes > *len) { return 0; }\n"
    "    memcpy(dest, *stream, nbytes);\n"
    "    *stream += nbytes;\n"
    "    *len -= nbytes;\n"
    "    return 1;\n"
    "}\n");
    fprintf(global_implementation_file, 
    "int writeToStream(uint8_t** stream, size_t* len, void* src, size_t nbytes)\n"
    "{\n"
    "    if (nbytes > *len) { return 0; }\n"
    "    memcpy(*stream, src, nbytes);\n"
    "    *stream += nbytes;\n"
    "    *len -= nbytes;\n"
    "    return 1;\n"
    "}\n");
}

DATA_DESK_FUNC void
DataDeskCustomParseCallback(DataDeskNode *root, char *filename)
{
    DataDeskFWriteGraphAsC(client_header_file, root);
    DataDeskFWriteGraphAsC(server_header_file, root);

	// Called for code that is parsed from a Data Desk file.
    if (root->type == DataDeskNodeType_StructDeclaration && DataDeskNodeHasTag(root, "Serializeable"))
    {
        // first do the server header and implementation
        fprintf(server_header_file, "int Read%sFromClient(uint8_t **stream, size_t *len, %s *obj);\n", root->string, root->string);
        fprintf(global_implementation_file, "int Read%sFromClient(uint8_t **stream, size_t *len, %s *obj)\n{\n", root->string, root->string);
        generateSerializationCode(global_implementation_file, root->children_list_head, "obj->", "c2s", MODE_READ);
        fprintf(global_implementation_file, "    return 1;\n}\n");
        fprintf(client_header_file, "int Write%sToServer(uint8_t **stream, size_t *len, %s *obj);\n", root->string, root->string);
        fprintf(global_implementation_file, "int Write%sToServer(uint8_t **stream, size_t *len, %s *obj)\n{\n", root->string, root->string);
        generateSerializationCode(global_implementation_file, root->children_list_head, "obj->", "c2s", MODE_WRITE);
        fprintf(global_implementation_file, "    return 1;\n}\n");
        
    }
}

DATA_DESK_FUNC void
DataDeskCustomCleanUpCallback(void)
{
	// De-initialization code goes here.
    fclose(client_header_file);
    fclose(server_header_file);
    fclose(global_implementation_file);
}

static void stripArrowOrDot(char *src, char *dest, int size)
{
    int len = strnlen(src, size);
    if (len > 1 && src[len] == '>') { len -= 2; }
    else if (len > 0 && src[len] == '.') { len -= 1; }
    memset(dest, 0, size);
    strncpy(dest, src, len);
}

int isTransmitable(DataDeskNode *node)
{
    for (int i = 0; i < sizeof(transmitable_types) / sizeof(char*); i++)
    {
        if (DataDeskMatchType(node, transmitable_types[i])) return 1;
    }
    // now match arrays of the same type
    int size = 20;
    char buf[20] = { 0 };
    for (int i = 0; i < sizeof(transmitable_types) / sizeof(char*); i++)
    {
        strncpy(buf, "[]", size);
        strncat(buf, transmitable_types[i], size);
        if (DataDeskMatchType(node, buf)) return 1;
    }
    return 0;
}

static void generateSerializationCode(FILE *file, DataDeskNode *root, char *access_string, char *tag, enum Mode mode)
{
    // check if it is a pointer, and if it is, we want to add a null check
    if (DataDeskGetIndirectionCountForType(root) == 1)
    {
        char stripped_access_string[ACCESS_STRING_SIZE];
        stripArrowOrDot(access_string, stripped_access_string, ACCESS_STRING_SIZE);
        fprintf(file, "if (!%s) return 0;\n", stripped_access_string);
    }
    for (DataDeskNode *node = root; node; node = node->next)
    {
        printf("loop\n");
        if (DataDeskNodeHasTag(node, tag))
        {
            printf("passed condition 0\n");
            if (node->type == DataDeskNodeType_Declaration)
            {
                printf("passed condition 1\n");
                // TODO make this list exhaustive
                if (DataDeskMatchType(node, "int") || DataDeskMatchType(node, "size_t"))
                {
                    DataDeskWarning(node, "Non-fixed-size types in Serializeable struct in file %s", node->file);
                }
                else if (isTransmitable(node))
                {
                    if (mode == MODE_READ)
                    {
                        fprintf(file, "    if (!cpyFromStream(stream, len, %s%s, sizeof(%s%s))) return 0;\n", access_string, node->name, access_string, node->name);
                    }
                    else if (mode == MODE_WRITE)
                    {
                        fprintf(file, "    if (!writeToStream(stream, len, %s%s, sizeof(%s%s))) return 0;\n", access_string, node->name, access_string, node->name);
                    }
                }
                else if (node->type == DataDeskNodeType_StructDeclaration)
                {
                    printf("reached this point0\n");
                    char next_access_string[ACCESS_STRING_SIZE] = { 0 };
                    snprintf(next_access_string, ACCESS_STRING_SIZE, "%s%s%s", access_string, node->string, DataDeskGetAccessStringForDeclaration(node));
                    generateSerializationCode(file, node->declaration.type->children_list_head, next_access_string, tag, mode);
                }
                else if ((node->declaration.type->type == DataDeskNodeType_Identifier && node->declaration.type->reference))
                {
                    printf("reached this point\n");
                    char next_access_string[ACCESS_STRING_SIZE] = { 0 };
                    snprintf(next_access_string, ACCESS_STRING_SIZE, "%s%s%s", access_string, node->string, DataDeskGetAccessStringForDeclaration(node));
                    generateSerializationCode(file, node->declaration.type->reference->children_list_head, next_access_string, tag, mode);
                }
                else
                {
                    DataDeskError(node, "Unhandled type for printing code generation.");
                }
            }

        }
    }
    
}