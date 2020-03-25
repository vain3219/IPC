all: sender receiver

sender: send.o
	g++ send.o -o sender

receiver: recv.o
	g++ recv.o -o receiver

send.o: send.cpp
	g++ -c send.cpp

receiver.o: recv.cpp
	g++ -c recv.cpp

clean:
	rm *o sender receiver
