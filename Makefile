all: server client device

server: server.o 
	g++ server.o -pthread -o server

server.o: server.cpp ThreadWrapper.h
	g++ -c -pthread server.cpp

client: client.o
	g++ client.o -pthread -o client

client.o: client.cpp ThreadWrapper.h
	g++ -c -pthread client.cpp

device: device.o
	g++ device.o -pthread -o device

device.o: device.cpp
	g++ -c -pthread device.cpp

clean: 
	rm *.o



