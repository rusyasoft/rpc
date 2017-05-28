
CC=gcc
CFLAGS=-w

all:	allclean server client test1s test1c runtest

allclean:
	rm -rf server client

server: rpc.c example_server.c
	$(CC) $(CFLAGS) -o server example_server.c rpc.c 

client:	rpc.c example_client.c
	$(CC) $(CFLAGS) -o client example_client.c rpc.c


test1s:	rpc.c tests/serverTest1.c
	$(CC) $(CFLAGS) -o tests/serverTest1 rpc.c tests/serverTest1.c

test1c:	rpc.c tests/clientTest1.c
	$(CC) $(CFLAGS) -o tests/clientTest1 rpc.c tests/clientTest1.c

runtest:
	tests/serverTest1 &
	sleep 1s
	tests/clientTest1
	sleep 1s
	sudo killall serverTest1
