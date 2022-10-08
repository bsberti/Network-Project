#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

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
	struct addrinfo* ptr = nullptr;
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	
	// 127.0.0.1 - local
	printf("Calling AddrInfo... ");
	result = getaddrinfo("127.0.0.1", "5555", &hints, &info);
	if (result != 0) {
		printf("getaddrinfo failed with error %d\n", result);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	// Create socket
	SOCKET connectSocket;
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

	// Waiting for user to press any button
	system("Pause");

	const char* buf = "Hello!";
	int buflen = 6;

	const int recvBufLen = 128;
	char recvBuf[recvBufLen];

	printf("Sending message to the server... ");
	result = send(connectSocket, buf, buflen, 0);
	if (result == SOCKET_ERROR) {
		printf("failed to send message to the server with error %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
		printf("Sent %d bytes to the Server.\n", result);
	}

	printf("Receiving message from the server... ");
	result = recv(connectSocket, recvBuf, recvBufLen, 0);
	if (result == SOCKET_ERROR) {
		printf("failed to receive message from the server with error %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
		printf("Received %d bytes from the Server.\n", result);
	}

	printf("Shuting down... ");
	result = shutdown(connectSocket, SD_SEND);
	if (result == SOCKET_ERROR) {
		printf("shutdown failed with error %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	printf("Closing... \n");
	closesocket(connectSocket);
	WSACleanup();

	return 0;
}