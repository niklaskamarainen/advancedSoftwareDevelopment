#include "week3_base.h"
#include "common.h"

#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <client_id>\n", argv[0]);
        return 1;
    }

    int client_id = atoi(argv[1]);

    if (client_id < 1 || client_id > CLIENT_COUNT)
    {
        fprintf(stderr, "Invalid client ID: %d\n", client_id);
        return 1;
    }

    printf("CLIENT %d start\n", client_id);

        
    char response_queue_name[64];

    snprintf(response_queue_name, sizeof response_queue_name, RESPONSE_QUEUE_FORMAT, client_id);


    mq_unlink(response_queue_name);
      

    struct mq_attr attributes = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = sizeof(Response),
        .mq_curmsgs = 0
    };

    mqd_t response_queue = mq_open(response_queue_name, O_CREAT | O_RDONLY, 0600, &attributes);

    if (response_queue == (mqd_t)-1)
    {
        die("mq_open response");
    }

    
    mqd_t request_queue = mq_open(REQUEST_QUEUE, O_WRONLY);

    if (request_queue == (mqd_t)-1)
    {
        mq_close(response_queue);
        mq_unlink(response_queue_name);
        die("mq_open request");
    }


    Request request = {
        OP_ALLOC,
        client_id
    };

    if (mq_send(request_queue,
                (const char *)&request,
                sizeof request,
                0) == -1)
    {
        mq_close(request_queue);
        mq_close(response_queue);
        mq_unlink(response_queue_name);
        die("mq_send ALLOC");
    }


    Response response;

    if (mq_receive(response_queue,
                   (char *)&response,
                   sizeof response,
                   NULL) == -1)
    {
        mq_close(request_queue);
        mq_close(response_queue);
        mq_unlink(response_queue_name);
        die("mq_receive response");
    }


    if (!response.ok)
    {
        printf("CLIENT %d ALLOC failed\n", client_id);

        mq_close(request_queue);
        mq_close(response_queue);
        mq_unlink(response_queue_name);

        return 1;
    }

    printf("CLIENT %d ALLOC OK offset=%d size=%d\n",
           client_id,
           response.offset,
           response.size);


    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0600);

    if (shm_fd == -1)
    {
        mq_close(request_queue);
        mq_close(response_queue);
        mq_unlink(response_queue_name);
        die("shm_open");
    }


    char *memory = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (memory == MAP_FAILED)
    {
        close(shm_fd);
        mq_close(request_queue);
        mq_close(response_queue);
        mq_unlink(response_queue_name);
        die("mmap");
    }


    snprintf(memory + response.offset,
             (size_t)response.size,
             "Hello from client %d",
             client_id);

    printf("SHM_WRITE client=%d offset=%d data=\"%s\"\n",
           client_id,
           response.offset,
           memory + response.offset);


    Request free_request = {
        OP_FREE,
        client_id
    };

    if (mq_send(request_queue,
                (const char *)&free_request,
                sizeof free_request,
                0) == -1)
    {
        perror("mq_send FREE");
    }

 
    munmap(memory, SHM_SIZE);
    close(shm_fd);

    mq_close(request_queue);

    mq_close(response_queue);
    mq_unlink(response_queue_name);

    printf("CLIENT %d complete\n", client_id);

    return 0;
}