CC = @g++
# I'm not sure if I need to remove -g or not
CFLAGS = -g -pedantic -static -I.
HEADERS = shell379.h
DEPS = shell379.o

# all .o files are dependant on all .cpp files
# and all .h
#
%.o: %.cpp %.h
	$(CC) -c -o $@ $< $(CFLAGS)

# goto $.o for .o files

all: $(DEPS) 
	$(CC) $(DEPS) -o shell379 $(CFLAGS)

clean:
	@rm -f *.o shell379
	@clear

# This is for testing, I'm keeping it here for later
# incase I forget

test:
	@ make
	@ ./prodcon 3 1 < input.txt
	@ cat prodcon.1.log
