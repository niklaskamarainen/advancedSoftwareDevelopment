Files:
- master.c
- server.c
- client.c
- common.h
- week3_base.h

Compile the programs with:
gcc -std=c17 -Wall -Wextra -pedantic server.c -o server
gcc -std=c17 -Wall -Wextra -pedantic client.c -o client
gcc -std=c17 -Wall -Wextra -pedantic master.c -o master

Start the whole system with:
./master

The server and clients are started automatically by the master process.

An example output is:
MASTER start
SERVER start
SERVER config memory=1024 clients=2 -> OK
SERVER READY; blocking in mq_receive()...
MASTER CONFIG_OK -> service is ready; clients may start
CLIENT 2 start
ALLOC client=2 offset=512 size=512
CLIENT 2 ALLOC OK offset=512 size=512
SHM_WRITE client=2 offset=512 data="Hello from client 2"
CLIENT 2 complete
FREE client=2
CLIENT 1 start
ALLOC client=1 offset=0 size=512
CLIENT 1 ALLOC OK offset=0 size=512
SHM_WRITE client=1 offset=0 data="Hello from client 1"
FREE client=1
CLIENT 1 complete
SERVER SHUTDOWN
SERVER SHM client=1 offset=0 data="Hello from client 1"
SERVER SHM client=2 offset=512 data="Hello from client 2"
SERVER cleanup
MASTER complete

