#include "week3_base.h"
#include "common.h"

#include <fcntl.h>
#include <mqueue.h>
#include <sys/mman.h>
#include <sys/stat.h>


static int write_full(int fd, const void *buf, size_t n)
{
    const char *p = buf;
    size_t left = n;

    while (left > 0)
    {
        ssize_t written = write(fd, p, left);

        if(written < 0)
        {
            perror("write");
            return -1;
        }

        p += written;
        left -= (size_t)written;
    }

    return 0;
}

static int read_full(int fd, void *buf, size_t n)
{
    char *p = buf;
    size_t left = n;

    while (left > 0)
    {
        ssize_t read_count = read(fd, p, left);

        if(read_count <= 0)
        {
            if(read_count < 0)
            {
                perror("read");
            }

            return -1;
        }

        p += read_count;
        left -= (size_t)read_count;
    }
    
    return 0;
}


int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        fprintf(stderr, "Usage: %s <config_fd> <status_fd>\n", argv[0]);
        return 1;
    }


    printf("SERVER start\n");


    int config_fd = atoi(argv[1]);
    int status_fd = atoi(argv[2]);



    ServerConfig config;

    if(read_full(config_fd, &config, sizeof config) == -1)
    {
        return 1;
    }
    close(config_fd);


    ConfigResponse config_response = {0, 0};

    if(config.memory_size != SHM_SIZE)
    {
        config_response.error_code = 1;
    }
    else if(config.client_count != CLIENT_COUNT)
    {
        config_response.error_code = 2;
    }
    else
    {
        config_response.ok = 1;
    }

    printf("SERVER config memory=%zu clients=%d -> %s\n",
            config.memory_size,
            config.client_count,
            config_response.ok ? "OK" : "NOK");


    if(write_full(status_fd, &config_response, sizeof config_response) == -1)
    {
        return 1;
    }
    close(status_fd);


    if(!config_response.ok)
    {
        return 2;
    }


    shm_unlink(SHM_NAME);

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);

    if(shm_fd == -1)
    {
        die("shm_open");
    }


    if(ftruncate(shm_fd, SHM_SIZE) == -1)
    {
        die("ftruncate");
    }


    char *memory = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if(memory == MAP_FAILED)
    {
        die("mmap");
    }


    int allocated[CLIENT_COUNT] = {0};


    mq_unlink(REQUEST_QUEUE);

    struct  mq_attr attributes = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = sizeof(Request),
        .mq_curmsgs = 0
    };

    mqd_t request_queue = mq_open(REQUEST_QUEUE, O_CREAT | O_RDONLY, 0600, &attributes);

    if(request_queue == (mqd_t)-1)
    {
        die("mq_open request");
    }

    printf("SERVER READY; blocking in mq_receive()...\n");



    int running = 1;

    while (running)
    {
        Request request;

        if(mq_receive(request_queue, (char *)&request, sizeof request, NULL) == -1)
        {
            perror("mq_receive");
            break;
        }


        if(request.operation == OP_SHUTDOWN)
        {
            printf("SERVER SHUTDOWN\n");

            running = 0;
            continue;
        }


        if(request.client_id < 1 || request.client_id > CLIENT_COUNT)
        {
            printf("SERVER invalid client=%d\n", request.client_id);
            
            continue;
        }

        int index = request.client_id - 1;



    
        if (request.operation == OP_ALLOC)
        {
            Response response = {
                .ok = 0,
                .offset = -1,
                .size = 0
            };

            if (!allocated[index])
            {
                allocated[index] = 1;

                response.ok = 1;
                response.offset = index * SLOT_SIZE;
                response.size = SLOT_SIZE;

                printf("ALLOC client=%d offset=%d size=%d\n",
                       request.client_id,
                       response.offset,
                       response.size);
            }
            else
            {
                printf("ALLOC_FAIL client=%d slot already allocated\n",
                       request.client_id);
            }



            char response_queue_name[64];

            snprintf(response_queue_name,
                     sizeof response_queue_name,
                     RESPONSE_QUEUE_FORMAT,
                     request.client_id);

            mqd_t response_queue = mq_open(response_queue_name, O_WRONLY);

            if (response_queue == (mqd_t)-1)
            {
                perror("mq_open response");
                continue;
            }



            if (mq_send(response_queue,
                        (const char *)&response,
                        sizeof response,
                        0) == -1)
            {
                perror("mq_send response");
            }

            mq_close(response_queue);

        }
        else if (request.operation == OP_FREE)
        {
            allocated[index] = 0;

            printf("FREE client=%d\n", request.client_id);
        }
    }
    
    for (int i = 0; i < CLIENT_COUNT; i++)
    {
        int offset = i * SLOT_SIZE;

        printf("SERVER SHM client=%d offset=%d data=\"%s\"\n",
               i + 1,
               offset,
               memory + offset);
    }

    mq_close(request_queue);
    mq_unlink(REQUEST_QUEUE);

    munmap(memory, SHM_SIZE);
    close(shm_fd);
    shm_unlink(SHM_NAME);

    printf("SERVER cleanup\n");

    return 0;
}