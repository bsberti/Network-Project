#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "5555"

int main(int argc, char** argv) {
	// Initialization
	WSADATA wsaData;
	int result;

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
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	printf("Calling AddrInfo... ");
	result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &info);
	if (result != 0) {
		printf("getaddrinfo failed with error %d\n", result);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	// Create our socket
	printf("Calling Listen Socket... ");
	SOCKET listenSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (listenSocket == INVALID_SOCKET) {
		printf("socket failed with error %d\n", WSAGetLastError());
		freeaddrinfo(info);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	// Bind our socket
	printf("Calling Bind... ");
	result = bind(listenSocket, info->ai_addr, (int)info->ai_addrlen);
	if (result == SOCKET_ERROR) {
		printf("bind failed with error %d\n", WSAGetLastError());
		freeaddrinfo(info);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	// Listen
	printf("Calling Listen... ");
	result = listen(listenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR) {
		printf("listen failed with error %d\n", WSAGetLastError());
		freeaddrinfo(info);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	} 
	else {
		printf("Succeded!\n");
	}

	printf("Calling Accept... ");
	SOCKET clientSocket = accept(listenSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET) {
		printf("accept failed with error %d\n", WSAGetLastError());
		freeaddrinfo(info);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	const int buflen = 128;
	char buf[buflen];

	printf("Receiving message from the client... ");
	result = recv(clientSocket, buf, buflen, 0);
	if (result == SOCKET_ERROR) {
		printf("failed to receive message from the client with error %d\n", WSAGetLastError());
		freeaddrinfo(info);
		closesocket(listenSocket);
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
		printf("Message from Client: %s\n", buf);

		printf("Sending message back to the client... ");
		result = send(clientSocket, buf, result, 0);
		if (result == SOCKET_ERROR) {
			printf("failed to send message back to the client with error %d\n", WSAGetLastError());
			freeaddrinfo(info);
			closesocket(listenSocket);
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
		else {
			printf("Succeded!\n");
			printf("Sent %d bytes to the Client.\n", result);
		}
	}

	printf("Closing... \n");
	freeaddrinfo(info);
	closesocket(listenSocket);
	closesocket(clientSocket);
	WSACleanup();

	return 0;
}