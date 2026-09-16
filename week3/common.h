#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

#define CLIENT_COUNT 2
#define SLOT_SIZE 512
#define SHM_SIZE (CLIENT_COUNT * SLOT_SIZE)

#define REQUEST_QUEUE "/week3_requests"
#define RESPONSE_QUEUE_FORMAT "/week3_response_%d"
#define SHM_NAME "/week3_shared_memory"

enum
{
    OP_ALLOC = 1,
    OP_FREE = 2,
    OP_SHUTDOWN = 9
};

typedef struct 
{
    size_t memory_size;
    int client_count;
} ServerConfig;

typedef struct 
{
    int ok;
    int error_code;
} ConfigResponse;

typedef struct 
{
    int operation;
    int client_id;
} Request;

typedef struct 
{
    int ok;
    int offset;
    int size;
} Response;


#endif