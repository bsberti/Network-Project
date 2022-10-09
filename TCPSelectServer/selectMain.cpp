#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "5555"

struct ClientInformation {
	SOCKET socket;
	bool connected = false;
};

struct ServerInfo {
	struct addrinfo* info = nullptr;
	struct addrinfo hints;
	SOCKET listenSocket = INVALID_SOCKET;
	fd_set activeSockets;
	fd_set socketsReadyForReading;
	std::vector<ClientInformation> clients;
} g_ServerInfo;


int Initialize() {
	// Initialization
	WSADATA wsaData;
	int result;

	FD_ZERO(&g_ServerInfo.activeSockets);
	FD_ZERO(&g_ServerInfo.socketsReadyForReading);

	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	printf("WSAStartup... ");
	if (result != 0) {
		printf("WSAStartup failed with error %d\n", result);
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	
	ZeroMemory(&g_ServerInfo.hints, sizeof(g_ServerInfo.hints));

	g_ServerInfo.hints.ai_family = AF_INET;
	g_ServerInfo.hints.ai_socktype = SOCK_STREAM;
	g_ServerInfo.hints.ai_protocol = IPPROTO_TCP;
	g_ServerInfo.hints.ai_flags = AI_PASSIVE;

	printf("Calling AddrInfo... ");
	result = getaddrinfo(NULL, DEFAULT_PORT, &g_ServerInfo.hints, &g_ServerInfo.info);
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
	g_ServerInfo.listenSocket = socket(g_ServerInfo.info->ai_family, g_ServerInfo.info->ai_socktype, g_ServerInfo.info->ai_protocol);
	if (g_ServerInfo.listenSocket == INVALID_SOCKET) {
		printf("socket failed with error %d\n", WSAGetLastError());
		freeaddrinfo(g_ServerInfo.info);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	// Bind our socket
	printf("Calling Bind... ");
	result = bind(g_ServerInfo.listenSocket, g_ServerInfo.info->ai_addr, (int)g_ServerInfo.info->ai_addrlen);
	if (result == SOCKET_ERROR) {
		printf("bind failed with error %d\n", WSAGetLastError());
		freeaddrinfo(g_ServerInfo.info);
		closesocket(g_ServerInfo.listenSocket);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	// Listen
	printf("Calling Listen... ");
	result = listen(g_ServerInfo.listenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR) {
		printf("listen failed with error %d\n", WSAGetLastError());
		freeaddrinfo(g_ServerInfo.info);
		closesocket(g_ServerInfo.listenSocket);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	return 0;
}

void Shutdown() {
	printf("Closing... \n");
	freeaddrinfo(g_ServerInfo.info);
	closesocket(g_ServerInfo.listenSocket);
	for (int i = 0; i < g_ServerInfo.clients.size(); i++) {
		closesocket(g_ServerInfo.clients[i].socket);
	}
	g_ServerInfo.clients.clear();
	WSACleanup();
}

int main(int argc, char** argv) {
	int result = Initialize();
	if (result != 0) {
		return result;
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500 * 1000;

	int selectResult;

	// Communication
	printf("Selecting server...");
	for (;;) {

		FD_ZERO(&g_ServerInfo.socketsReadyForReading);

		// Listen socket to check any new connections
		FD_SET(g_ServerInfo.listenSocket, &g_ServerInfo.socketsReadyForReading);

		// Add all conected sockets
		for (int i = 0; i < g_ServerInfo.clients.size(); i++) {
			ClientInformation currentClient = g_ServerInfo.clients[i];
			if (currentClient.connected) {
				FD_SET(currentClient.socket, &g_ServerInfo.socketsReadyForReading);
			}
		}
		
		selectResult = select(0, &g_ServerInfo.socketsReadyForReading, NULL, NULL, &tv);
		if (selectResult == SOCKET_ERROR) {
			printf("select failed with error %d\n", WSAGetLastError());
			return 1;
		}
		printf(".");

		if (FD_ISSET(g_ServerInfo.listenSocket, &g_ServerInfo.socketsReadyForReading)) {
			printf("\nCalling Accept... ");
			SOCKET newClientSocket = accept(g_ServerInfo.listenSocket, NULL, NULL);
			if (newClientSocket == INVALID_SOCKET) {
				printf("accept failed with error %d\n", WSAGetLastError());
			}
			else {
				printf("Succeded!\n");
				ClientInformation newClient;
				newClient.socket = newClientSocket;
				newClient.connected = true;
				g_ServerInfo.clients.push_back(newClient);
			}
		}

		for (int i = 0; i < g_ServerInfo.clients.size(); i++) {
			ClientInformation& currentClient = g_ServerInfo.clients[i];
			if (!currentClient.connected)
				continue;

			if (FD_ISSET(currentClient.socket, &g_ServerInfo.socketsReadyForReading)) {
				// Act as a ping server
				const int buflen = 128;
				char buf[buflen];

				int recvResult = recv(currentClient.socket, buf, buflen, 0);
				if (recvResult == 0) {
					printf("Client disconnected\n");
					currentClient.connected = false;
					g_ServerInfo.clients.erase(g_ServerInfo.clients.begin() + i);
					continue;
				}
				else if (recvResult == SOCKET_ERROR) {
					printf("failed to receive message from the client with error %d\n", WSAGetLastError());
				}
				else {
					printf("Message from Client: %s\n", buf);
				}

				int sendResult = send(currentClient.socket, buf, recvResult, 0);
				if (sendResult == SOCKET_ERROR) {
					printf("failed to send message back to the client with error %d\n", WSAGetLastError());
				}
				else {
					printf("Sent %d bytes to the Client.\n", sendResult);
				}
			}
		}
	}


	Shutdown();
	return 0;
}