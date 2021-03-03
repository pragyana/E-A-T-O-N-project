/*
 *  @file: device.cpp
 *  @author: Ryan Dutton
 *  @date: 1/3/2021
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <signal.h>
#include <time.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>


int clientSocket = 0;
std::string deviceName("Unregistered device");

/**
    handler for cntrl-C interruption
    @param SIGINT, the number for cntrl-C
    @return void
*/
void signal_callback_handler(int signum) {
   std::cout << std::endl << "Cntl-C interruption.  " << std::endl;
   std::string message = deviceName + "-" + "exit";
   send(clientSocket, message.c_str(), 1024, 0);

   // Terminate program
   exit(signum);
}

int main(int argc, char* argv[]){

    if(argc < 2)
    {
        std::cerr << "Syntax : ./device <device name>"<< std::endl;
        return 0;
    }

    deviceName = std::string(argv[1]);
	signal(SIGINT, signal_callback_handler);

	clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	bzero((char*) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8000);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(connect(clientSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) return 0;

	const int BUFFER_SIZE = 512;
	char dataBuffer[BUFFER_SIZE];
	memset(dataBuffer, 0, BUFFER_SIZE);
	int read = recv(clientSocket, dataBuffer, BUFFER_SIZE, 0);

	std::string receivedData(dataBuffer);
	std::cout << std::endl << "Connected to the monitor on port no. " << receivedData << std::endl;
	std::cout << "............" << std::endl;
	std::cout << deviceName << " is now sending measurement data...." << std::endl;

	clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	bzero((char*) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(std::stoi(receivedData));
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (connect(clientSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) return 0;

	while(true){

		std::string message;
		std::string measurement;
		srand(time(NULL));
		measurement = std::to_string(rand() % 10 + 100);
		message = deviceName + ":" + measurement;

		send(clientSocket, message.c_str(), 512, 0);
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}


}







