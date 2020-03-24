all: receiver

receiver: main.o
	g++ main.o -o receiver

main.o: main.cpp
	g++ -c main.cpp

clean:
	rm *o receiver
