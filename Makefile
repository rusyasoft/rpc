
CC=gcc
CFLAGS=-w

all:	allclean server client test1s test1c test2s test2c runtest

allclean:
	rm -rf server client

server: rpc.c example_server.c
	$(CC) $(CFLAGS) -o server example_server.c rpc.c rpc_common.c rpcsvc.c

client:	rpc.c example_client.c
	$(CC) $(CFLAGS) -o client example_client.c rpc.c rpc_common.c rpcclt.c


test1s:	rpc.c tests/serverTest1.c
	$(CC) $(CFLAGS) -o tests/serverTest1 rpc.c rpc_common.c rpcsvc.c tests/serverTest1.c

test1c:	rpc.c tests/clientTest1.c
	$(CC) $(CFLAGS) -o tests/clientTest1 rpc.c rpc_common.c rpcclt.c tests/clientTest1.c

test2s:	rpc.c tests/serverTest2.c
	$(CC) $(CFLAGS) -o tests/serverTest2 rpc.c rpc_common.c rpcsvc.c tests/serverTest2.c

test2c:	rpc.c tests/clientTest2.c
	$(CC) $(CFLAGS) -o tests/clientTest2 rpc.c rpc_common.c rpcclt.c tests/clientTest2.c

runtest:
	sudo sleep 1s

	tests/serverTest1 &
	sleep 1s
	tests/clientTest1
	sleep 1s
	sudo killall serverTest1

	tests/serverTest2 &
	sleep 1s
	tests/clientTest2
	sleep 1s
	sudo killall serverTest2
