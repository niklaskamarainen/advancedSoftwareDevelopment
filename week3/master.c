#include "week3_base.h"
#include "common.h"

#include <fcntl.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/wait.h>

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
        ssize_t read_count = read(fd, p , left);

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

int main(void)
{
    printf("MASTER start\n");

    int config_pipe[2];
    int status_pipe[2];

    if(pipe(config_pipe) == -1)
    {
        die("config pipe");
    }

    if(pipe(status_pipe) == -1)
    {
        die("status pipe");
    }


    pid_t server_pid = fork();

    if(server_pid == -1)
    {
        die("fork server");
    }

    if(server_pid == 0)
    {
        close(config_pipe[1]);
        close(status_pipe[0]);

        char config_fd_text[16];
        char status_fd_text[16];

        snprintf(config_fd_text, sizeof config_fd_text,
                    "%d", config_pipe[0]);

        snprintf(status_fd_text, sizeof status_fd_text,
                    "%d", status_pipe[1]);

        execl("./server", "server",
                config_fd_text, status_fd_text, (char *)NULL);

        perror("execl server");
        _exit(127);
    }

    close(config_pipe[0]);
    close(status_pipe[1]);


    ServerConfig config = {
        SHM_SIZE,
        CLIENT_COUNT
    };

    if (write_full(config_pipe[1], &config, sizeof config) == -1)
    {
        return 1;
    }

    close(config_pipe[1]);

    
    
    ConfigResponse response;

    if(read_full(status_pipe[0], &response, sizeof response) == -1)
    {
        return 1;
    }

    close(status_pipe[0]);



    if(response.ok == 0)
    {
        printf("MASTER CONFIG_NOK error=%d -> clients are NOT started\n", response.error_code);

        waitpid(server_pid, NULL, 0);

        return 2;
    }

    printf("MASTER CONFIG_OK -> service is ready; clients may start\n");



    pid_t client_pids[CLIENT_COUNT];

    for(int i = 0; i < CLIENT_COUNT; i++)
    {
        client_pids[i] = fork();

        if(client_pids[i] == -1)
        {
            die("fork client");
        }

        if(client_pids[i] == 0)
        {
            char id_text[16];

            snprintf(id_text, sizeof id_text, "%d", i + 1);

            execl("./client", "client", id_text, (char *)NULL);

            perror("execl client");
            _exit(127);
        }
    }



    for(int i = 0; i < CLIENT_COUNT; i++)
    {
        if(waitpid(client_pids[i], NULL, 0) == -1)
        {
            die("waitpid client");
        }
    }


    mqd_t request_queue = mq_open(REQUEST_QUEUE, O_WRONLY);

    if(request_queue == (mqd_t)-1)
    {
        die("mq_open shutdown");
    }

    Request shutdown = {
        OP_SHUTDOWN,
        0
    };

    if(mq_send(request_queue, (const char*)&shutdown, sizeof shutdown, 0) == -1)
    {
        die("mq_send shutdown");
    }

    mq_close(request_queue);



    if(waitpid(server_pid, NULL, 0) == -1)
    {
        die("waitpid server");
    }

    printf("MASTER complete\n");

    
    return 0;
}