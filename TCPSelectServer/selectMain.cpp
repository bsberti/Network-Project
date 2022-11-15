#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>

//#include "gen/addressbook.pb.h"

#pragma comment(lib, "Ws2_32.lib")

#include "MyBuffer.h"
#include "cCreateAccountPacket.h"

#define DEFAULT_PORT "5555"

// Create socket
SOCKET connectSocket;
const int recvBufLen = 1024;
char receivedBuffer[recvBufLen];

MyBuffer* buffer = new MyBuffer(128);
cCreateAccountPacket pCreateAccountPacket;

bool tryAgain = true;

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
	std::vector<std::string> rooms;
} g_ServerInfo;

struct Packet {
	int packetLength;
	int messageId;
};

struct SendCommandPacket : public Packet {
	std::string roomName;
	std::string message;
};

struct JoinRoomCommandPacket : public Packet {
	std::string roomName;
};

struct LeaveRoomCommandPacket : public Packet {
	std::string roomName;
};

int ConnectAuth(std::string ipaddr, std::string port) {
	WSADATA wsaData;
	int resultAuth;
	bool tryAgain = true;

	resultAuth = WSAStartup(MAKEWORD(2, 2), &wsaData);
	printf("WSAStartup... ");
	if (resultAuth != 0) {
		printf("WSAStartup failed with error %d\n", resultAuth);
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
	resultAuth = getaddrinfo(ipaddr.c_str(), port.c_str(), &hints, &info);
	if (resultAuth != 0) {
		printf("getaddrinfo failed with error %d\n", resultAuth);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}


	printf("Creating socket... ");
	connectSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if (connectSocket == INVALID_SOCKET) {
		printf("socket failed with error %d\n", resultAuth);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
	}

	// Connect to the server
	printf("Connecting to the server... ");
	resultAuth = connect(connectSocket, info->ai_addr, (int)info->ai_addrlen);
	if (resultAuth == SOCKET_ERROR) {
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
	resultAuth = ioctlsocket(connectSocket, FIONBIO, &NonBlock);
	if (resultAuth == SOCKET_ERROR) {
		printf("ioctlsocket failed with error %d\n", WSAGetLastError());
		closesocket(connectSocket);
		freeaddrinfo(info);
		WSACleanup();
		return 1;
	}
	else {
		printf("Succeded!\n");
		ZeroMemory(receivedBuffer, recvBufLen);

		while (tryAgain) {
			resultAuth = recv(connectSocket, receivedBuffer, recvBufLen, 0);
			// 0 = closed connection, disconnection
			// > 0 = number of bytes received
			// -1 = SCOKET_ERROR

			if (resultAuth == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAEWOULDBLOCK) {
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
				std::cout << "\n Auth Server Message >>>>>>\n\n " << std::string(receivedBuffer, 0, resultAuth) << std::endl;
				//printf("Received %d bytes from the Server.\n", result);
				resultAuth = 0;
			}
		}
	}

	return resultAuth;
}

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
	printf("Selecting...");
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

				std::string welcomeMessage = "Welcome to MyServer!\nPossible Commands:\n1 - Join a Room \n2 - Leave a Room \n3 - Send Message to a Room \n";

				int sendResult = send(newClient.socket, welcomeMessage.c_str(), welcomeMessage.size() + 1, 0);
				if (sendResult == SOCKET_ERROR) {
					printf("failed to send message back to the client with error %d\n", WSAGetLastError());
				}
				else {
					printf("Sent %d bytes to the Client.\n", sendResult);
				}
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

				// Receiving message from currentClient
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
					// Information received
					buffer->m_Buffer = std::vector<uint8_t>(&buf[0], &buf[buflen]);
					
					//Check Message ID first (LOGIN or SIGN IN)
					int messageTotalLenght = buffer->ReadUInt32LE();
					int messageId = buffer->ReadUInt32LE();

					pCreateAccountPacket.setPacketLength(messageTotalLenght);
					pCreateAccountPacket.setMessageId(messageId);

					if (messageId == 1) {
						// SEND Login to Auth Server
						//pLoginPacket.deserializePacket(buffer);

						//std::string username = pLoginPacket.username;
						//std::string password = pLoginPacket.password;

						result = ConnectAuth("127.0.0.1", "5556");
						if (result != 0) {
							return result;
						}
						

					}
					else if (messageId == 2) {
						// SEND Create Account to Auth Server
						pCreateAccountPacket.deserializePacket(buffer);

						std::string name = pCreateAccountPacket.name;
						std::string email = pCreateAccountPacket.email;
						std::string username = pCreateAccountPacket.username;
						std::string password = pCreateAccountPacket.password;
						
						result = ConnectAuth("127.0.0.1", "5556");
						if (result != 0) {
							return result;
						}
						
						// Send Buffer
						int sendResult = send(connectSocket, (const char*)&(buffer->m_Buffer[0]), buffer->m_Buffer.size(), 0);
						if (sendResult == SOCKET_ERROR)
						{
							printf("failed to send message to the server with error %d\n", WSAGetLastError());
							closesocket(connectSocket);
							WSACleanup();
							return 1;
						}
						else {
							ZeroMemory(receivedBuffer, recvBufLen);
							tryAgain = true;
							std::cout << "Waiting for Server response ";
							while (tryAgain) {
								result = recv(connectSocket, receivedBuffer, recvBufLen, 0);
								// 0 = closed connection, disconnection
								// > 0 = number of bytes received
								// -1 = SCOKET_ERROR

								if (result == SOCKET_ERROR) {
									if (WSAGetLastError() == WSAEWOULDBLOCK) {
										tryAgain = true;
										std::cout << ". ";
									}
									else {
										printf("failed to receive message from the server with error %d\n", WSAGetLastError());
										closesocket(connectSocket);
										WSACleanup();
										return 1;
									}
								}
								else {
									std::string recvString = std::string(receivedBuffer, 0, result);
									std::cout << "SERVER> " << recvString << std::endl;
									printf("Received %d bytes from the Server.\n", result);
									if (recvString == "validated") {
										tryAgain = false;
										
										//LOGIN SUCESSFULL
										std::cout << "Login Sucessfull!";

										//Send the same message to Client
										int sendAccountResult = send(currentClient.socket, recvString.c_str(), recvString.size() + 1, 0);
										if (sendAccountResult == SOCKET_ERROR) {
											printf("failed to send message back to the client with error %d\n", WSAGetLastError());
										}
										else {
											printf("Sent %d bytes to the Client.\n", sendAccountResult);
										}
									}
									else {
										//LOGIN FAIL
										std::cout << "Login Fail!";
										std::string loginFail = "Invalid credentials";
										int sendFailResult = send(currentClient.socket, loginFail.c_str(), loginFail.size() + 1, 0);
										if (sendFailResult == SOCKET_ERROR) {
											printf("failed to send message back to the client with error %d\n", WSAGetLastError());
										}
										else {
											printf("Sent %d bytes to the Client.\n", sendFailResult);
										}
									}
								}
							}
						}
					}
				}

				std::string serverMessage = "Mensagem recebida.\n";
				int sendResult = send(currentClient.socket, serverMessage.c_str(), serverMessage.size() + 1, 0);
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


//#include "MyServer.h"

//int main(int argc, char** argv) {
//	MyServer server;
//
//	int result = server.Initialize();
//	if (result != 0) {
//		return result;
//	}
//
//	struct timeval tv;
//	tv.tv_sec = 0;
//	tv.tv_usec = 500 * 1000;
//
//	int selectResult;
//
//	// Communication
//	selectResult = server.OpenServer(tv);
//	if (selectResult != 0) {
//		return selectResult;
//	}
//
//	server.Shutdown();
//	return 0;
//}