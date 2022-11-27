CC = g++
# I'm not sure if I need to remove -g or not
CFLAGS = -O
HEADERS = threader.h consumer.h producer.h tands.h extern.h
DEPS = threader.o consumer.o producer.o tands.o

# all .o files are dependant on all .cpp files
# and all .h
#
%.o: %.cpp %.h
	$(CC) -c -o $@ $< $(CFLAGS)

# goto $.o for .o files

all: $(DEPS) 
	$(CC) $(DEPS) -o prodcon $(CFLAGS)

clean:
	@rm -f *.o prodcon *.log
	@clear

# This is for testing, I'm keeping it here for later
# incase I forget

test:
	@ make
	@ ./prodcon 3 1 < input.txt
	@ cat prodcon.1.log
