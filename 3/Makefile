CC = @g++
CFLAGS = -g -pedantic -static -pthread -I.
HEADER = tands.h
DEPS = server.o client.o

client: client.cpp $(HEADER)
	$(CC)  -o client client.cpp tands.h $(CFLAGS) 

server: server.cpp $(HEADER)
	$(CC) -o server server.cpp tands.h $(CFLAGS) 

clean:
	@rm -f *.o client server *.log
	@clear

# This is for testing, I'm keeping it here for later
# incase I forget

ctest:
	make client
	./client 8888 127.0.0.1

stest:
	make server
	./server 8888





