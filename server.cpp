/*
 *  @file: server.cpp
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
#include <vector>
#include <unordered_map>
#include <mutex>
#include <string>

#include "ThreadWrapper.h"

struct socketClient {
	int clientIndex;
	int sockID;
	struct sockaddr_in clientAddr;
	uint32_t portNumber;
};

std::mutex m;

/**
    Serializes the messageCount map to be sent to the client.
    @param messageCount, a map that uses device name as the key and message count as the value
    @return c++ string object
*/
std::string serialize_message_count(const std::unordered_map<std::string, int>& messageCount)
{
	std::string messageCountString;

	for (auto& m : messageCount)
	{
		messageCountString = messageCountString + m.first + " -- " + std::to_string(m.second) + "\n";
	}

	return messageCountString;
}

/**
    function used to create the thread for managing individual device and the client
    @param messageCount, a map that uses device name as the key and message count as the value
    @return void
*/

void socket_client_handler(socketClient* clientDetail, std::unordered_map<std::string, int>& messageCount)
{

	const int BUFFER_SIZE = 512;

	socklen_t len = sizeof(struct sockaddr_in);

	int clientNumber = clientDetail->clientIndex;
	int oldClientSocket = clientDetail->sockID;
	uint32_t portNumber = clientDetail->portNumber;

	std::cout << "Socket client no." << clientNumber + 1 << " connected." << std::endl;

	int serverSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;
	bzero((char*) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portNumber);
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);


	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		std::cout << "Bind failed inside a thread." << std::endl;
		return;
	}

	if (listen(serverSocket, 16) == -1)
	{
		std::cout << "Listen failed inside a thread." << std::endl;
		return;
	}

	std::string messageBuffer = std::to_string(portNumber);
	send(oldClientSocket, messageBuffer.c_str(), BUFFER_SIZE, 0);

	int clientSocket = accept(serverSocket, (struct sockaddr*)&(clientDetail->clientAddr), &len);
	clientDetail->sockID = clientSocket;

	while (true) {
		char dataBuffer[BUFFER_SIZE];
		memset(dataBuffer, 0, BUFFER_SIZE);
		int read = recv(clientSocket, dataBuffer, BUFFER_SIZE, 0);

		size_t pos_start = 0;
		size_t pos_end = 0;

		std::string receivedData(dataBuffer);
		std::cout << receivedData << std::endl;
		std::string deviceName;

		if ((pos_end = receivedData.find(":", 0)) != std::string::npos)
		{
			deviceName = receivedData.substr(pos_start, pos_end);

			std::lock_guard<std::mutex> lock(m);
			if (messageCount.find(deviceName) == messageCount.end())
			{
				messageCount[deviceName] = 1;
			}
			else
			{
				messageCount[deviceName]++;
			}

		}
		else if (receivedData.compare("get-count") == 0)
		{
			const int GETCOUNTBUFFERSIZE = 200000;
			std::string messageCountString = serialize_message_count(messageCount);
			send(clientSocket, messageCountString.c_str(), GETCOUNTBUFFERSIZE, 0);
		}
		else if ((pos_end = receivedData.find("-exit", 0)) != std::string::npos)
		{
			deviceName = receivedData.substr(pos_start, pos_end);
			std::cout << deviceName << " has shut down." << std::endl;
			break;
		}

	}

	close(clientSocket);

}

int main() {
	socklen_t len = sizeof(struct sockaddr_in);
	int clientCount = 0;
	const int MAX_DEVICE_COUNT = 2001;

	struct socketClient Client[MAX_DEVICE_COUNT];
	std::unordered_map<std::string, int> messageCount;

	std::vector<ThreadWrapper> Threads;

	int serverSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;
	bzero((char*) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8000);
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);

	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		std::cout << "Bind failed in the main thread." << std::endl;
		return -1;
	}

	if (listen(serverSocket, MAX_DEVICE_COUNT) == -1)
	{
		std::cout << "Listen failed in the main thread." << std::endl;
		return -1;
	}

	std::cout << "The monitor is now listening on port 8000 ..........." << std::endl;

	while (true) {

		Client[clientCount].sockID = accept(serverSocket, (struct sockaddr*)&Client[clientCount].clientAddr, &len);
		Client[clientCount].clientIndex = clientCount;

		uint32_t newClientPortNumber = 8000 + clientCount + 1;
		Client[clientCount].portNumber = newClientPortNumber;

		ThreadWrapper newThread(std::thread(socket_client_handler, &Client[clientCount], std::ref(messageCount)), &std::thread::join);
		Threads.push_back(std::move(newThread));

		clientCount++;
		if (clientCount >= MAX_DEVICE_COUNT)
		{
			std::cerr << "Number of device connection has exceeded the limit." << std::endl;
			break;
		}
	}

}
