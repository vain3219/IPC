all: sender receiver

sender: send.o
	g++ send.o -o sender

receiver: recv.o
	g++ recv.o -o receiver

send.o: send.cpp
	g++ -c send.cpp

receiver.o: recv.cpp
	g++ -c recv.cpp

tar: 
	tar cvf assignment1_Duty-Thompson-Webber.tar *.cpp *.h *.md *.png makefile DesignforAssignment1.docx

clean:
	rm *o sender receiver keyfile.txt recvfile assignment1_Duty-Thompson-Webber.tar
