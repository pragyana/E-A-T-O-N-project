/*
 *  @file: client.cpp
 *  @author: Ryan Dutton
 *  @date: 1/3/2021
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <thread>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>
#include <signal.h>

#include "ThreadWrapper.h"

int clientSocket = 0;

/**
    handler for cntrl-C interruption
    @param SIGINT, the number for cntrl-C
    @return void
*/
void signal_callback_handler(int signum) {
   std::cout << std::endl << "Cntl-C interruption.  " << std::endl;
   std::string message = "Client-exit";
   send(clientSocket, message.c_str(), 1024, 0);

   // Terminate program
   exit(signum);
}

/**
    function used to create the thread for receiving data from the server
    @param socket id to be used for the 'recv' function
    @return void
*/
void socket_receive_handler(int * sockID){

	int clientSocket = *((int *) sockID);

	while(true)
	{
		const int BUFFER_SIZE = 200000;
		char dataBuffer[BUFFER_SIZE];
		memset(dataBuffer, 0, BUFFER_SIZE);
		int read = recv(clientSocket, dataBuffer, BUFFER_SIZE, 0);
		std::cout << dataBuffer << std::endl;
	}
	close(clientSocket);
}

int main(){
	signal(SIGINT, signal_callback_handler);

	clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;
	bzero((char*) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8000);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(connect(clientSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) return 0;

	char data[512];
	memset(data, 0, 512);
	int read = recv(clientSocket,data,512,0);
	data[read] = '\0';
	std::string receivedData(data);
	std::cout << std::endl << "Connected to the monitor on port no. " << receivedData << std::endl;

	clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	bzero((char*) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(std::stoi(receivedData));
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (connect(clientSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) return 0;

	ThreadWrapper newThread(std::thread(socket_receive_handler, &clientSocket), &std::thread::join);


	while(true){

		std::string input;
		std::cin >> input;

		if (input.compare("get-count") == 0)
		{
			std::cout << "(Device name)--(Message count)" << std::endl;
			send(clientSocket,input.c_str(),512,0);
		}

	}

}







