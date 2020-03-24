all: sender

sender: main.o
	g++ main.o -o sender

main.o: main.cpp
	g++ -c main.cpp

clean:
	rm *o sender
