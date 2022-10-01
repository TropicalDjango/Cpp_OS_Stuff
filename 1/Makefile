CC = @g++
CFlAGS = -g -pedantic -static

all: shell379 tester

shell379: shell379.o
	$(CC) shell379.o -o shell379 $(CFLAGS)
	
shell379.o: shell379.cpp
	$(CC) shell379.cpp -o shell379.o -c $(CFLAGS)

tester: tester.o
	$(CC) tester.o -o tester $(CFLAGS)

tester.o: tester.cpp
	$(CC) tester.cpp -o tester.o -c $(CFLAGS)

clean:
	@rm -f *.o shell379 tester