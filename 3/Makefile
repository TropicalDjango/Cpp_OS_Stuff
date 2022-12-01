CC = @g++
CFLAGS = -O 
HEADER = tands.h
DEPS = server.o client.o

all: server client

client: client.cpp $(HEADER)
	$(CC)  -o client client.cpp tands.h $(CFLAGS) 
	@man -Tps ./client_man | ps2pdf - client.pdf

server: server.cpp $(HEADER)
	$(CC) -o server server.cpp tands.h $(CFLAGS) 
	@man -Tps ./server_man | ps2pdf - server.pdf

clean:
	@rm -f *.o client server *.log *.pdf
	@find . -name '*.[[:digit:]]*' | xargs rm || true
	@clear

# This is for testing, I'm keeping it here for later
# incase I forget
ctest:
	make client
	./client 8888 127.0.0.1 < in1.txt &
	./client 8888 127.0.0.1 < in1.txt &
	./client 8888 127.0.0.1 < in1.txt &
	./client 8888 127.0.0.1 < in2.txt &	
	./client 8888 127.0.0.1 < in2.txt &
	./client 8888 127.0.0.1 < in2.txt &
stest:
	make server
	./server 8888





