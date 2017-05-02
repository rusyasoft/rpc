
CC=gcc
CFLAGS=-w

all:	allclean server client

allclean:
	rm -rf server client

server: rpc.c example_server.c
	$(CC) $(CFLAGS) -o server example_server.c rpc.c 

client:	rpc.c example_client.c
	$(CC) $(CFLAGS) -o client example_client.c rpc.c


