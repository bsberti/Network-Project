#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

WSADATA wsaData;
int result;

// Create socket
SOCKET connectSocket;

int Initialize(std::string ipaddr, std::string port) {
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	printf("WSAStartup... ");
	if (result != 0) {
		printf("WSAStartup failed with error %d\n", result);
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	struct addrinfo* info = nullptr;
	struct addrinfo* ptr = nullptr;
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// 127.0.0.1 - local
	printf("Calling AddrInfo... ");
	result = getaddrinfo(ipaddr.c_str(), port.c_str(), &hints, &info);
	if (result != 0) {
		printf("getaddrinfo failed with error %d\n", result);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	
	printf("Creating socket... ");
	connectSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (connectSocket == INVALID_SOCKET) {
		printf("socket failed with error %d\n", result);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	// Connect to the server
	printf("Connecting to the server... ");
	result = connect(connectSocket, info->ai_addr, (int)info->ai_addrlen);
	if (result == SOCKET_ERROR) {
		printf("failed to connect to the server with error %d\n", WSAGetLastError());
		closesocket(connectSocket);
		freeaddrinfo(info);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	DWORD NonBlock = 1;
	printf("Input-Output control socket nonBlock... ");
	result = ioctlsocket(connectSocket, FIONBIO, &NonBlock);
	if (result == SOCKET_ERROR) {
		printf("ioctlsocket failed with error %d\n", WSAGetLastError());
		closesocket(connectSocket);
		freeaddrinfo(info);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	return result;
}

void Shutingdown() {
	printf("Shuting down... ");
	result = shutdown(connectSocket, SD_SEND);
	if (result == SOCKET_ERROR) {
		printf("shutdown failed with error %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return;
	}
	else {
		printf("Succeded!\n");
	}

	printf("Closing... \n");
	closesocket(connectSocket);
	WSACleanup();
}

int main(int argc, char** argv) {
	// Initialization
	result = Initialize("127.0.0.1", "5555");
	if (result != 0) {
		Shutingdown();
		return result;
	}

	// Do-while loop to send and receive data
	
	const int recvBufLen = 1024;
	char receivedBuffer[recvBufLen];
	bool tryAgain = true;
	std::string userInput;
	std::string userName;

	// Prompt the user for some text
	std::cout << "Insert Username> ";
	getline(std::cin, userName);

	do
	{
		std::cout << "Message: ";
		getline(std::cin, userInput);

		if (userInput.size() > 0)
		{
			// Send the text
			int sendResult = send(connectSocket, userInput.c_str(), userInput.size() + 1, 0);
			if (sendResult == SOCKET_ERROR)
			{
				printf("failed to send message to the server with error %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return 1;
			} else {
				ZeroMemory(receivedBuffer, recvBufLen);
				while (tryAgain) {
					result = recv(connectSocket, receivedBuffer, recvBufLen, 0);
					// 0 = closed connection, disconnection
					// > 0 = number of bytes received
					// -1 = SCOKET_ERROR

					if (result == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAEWOULDBLOCK) {
							printf(".");
							tryAgain = true;
						}
						else {
							printf("failed to receive message from the server with error %d\n", WSAGetLastError());
							closesocket(connectSocket);
							WSACleanup();
							return 1;
						}
					}
					else {
						tryAgain = false;
						printf("Succeded!\n");
						printf("Received %d bytes from the Server.\n", result);
					}
				}
				//result = recv(connectSocket, receivedBuffer, recvBufLen, 0);
				//if (result > 0)
				//{
				//	// Echo response to console
				//	std::cout << "SERVER> " << std::string(receivedBuffer, 0, result) << std::endl;
				//}
			}
		}

	} while (userInput.size() > 0);	

	Shutingdown();
	return 0;
}