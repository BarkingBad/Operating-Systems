all:
	gcc server.c -o server -lpthread
	gcc client.c -o client

tests:
	./server 696969 /tmp/p0fsock
	./client client1 LOCAL /tmp/p0fsock 