CC = @g++
# I'm not sure if I need to remove -g or not
CFLAGS = -O -pedantic -static -pthread -I.
HEADERS = server.h client.h tands.h
DEPS = server.o client.o

# all .o files are dependant on all .cpp files
# and all .h
%.o: %.cpp %.h
	$(CC) -c -o $@ $< $(CFLAGS)

# goto $.o for .o files

client: $(DEPS) 
	$(CC) client.o -o client $(CFLAGS)

server: $(DEPS)
	$(CC) server.o -o server $(CFLAGS)
clean:
	@rm -f *.o client server *.log
	@clear

# This is for testing, I'm keeping it here for later
# incase I forget

test:
	@ make
	@ ./prodcon 3 1 < input.txt
	@ cat prodcon.1.log
