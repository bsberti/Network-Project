#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

#include "MyBuffer.h"

MyBuffer buffer(8);

WSADATA wsaData;
int result;

// Create socket
SOCKET connectSocket;

const int recvBufLen = 1024;
char receivedBuffer[recvBufLen];
bool tryAgain = true;
std::string userInput;
std::string userName;

struct Packet {
	int packetLength;
	int messageId;
};

struct JoinRoomCommandPacket : public Packet {
	std::string roomName;
};

struct LeaveRoomCommandPacket : public Packet {
	std::string roomName;
};

struct SendCommandPacket : public Packet {
	std::string roomName;
	std::string message;
};


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
		ZeroMemory(receivedBuffer, recvBufLen);
		while (tryAgain) {
			result = recv(connectSocket, receivedBuffer, recvBufLen, 0);
			// 0 = closed connection, disconnection
			// > 0 = number of bytes received
			// -1 = SCOKET_ERROR

			if (result == SOCKET_ERROR) {
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
				std::cout << "\n Server Message >>>>>>\n\n " << std::string(receivedBuffer, 0, result) << std::endl;
				//printf("Received %d bytes from the Server.\n", result);
				result = 0;
			}
		}
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

void CheckUserImput(std::string userImput) {
	JoinRoomCommandPacket joinPkt;
	LeaveRoomCommandPacket leavePkt;
	SendCommandPacket sendPkt;

	std::string roomName;
	std::string message;

	switch (userImput[0]) {
	case '1':

		joinPkt.messageId = 1;

		std::cout << "Room Name you wanted to join: \n";
		std::cout << "(If room Name doesn't exist, it will be created)\n>";
		getline(std::cin, roomName);

		joinPkt.roomName = roomName;
		joinPkt.packetLength = sizeof(Packet) +
			sizeof(roomName.size()) + roomName.size();

		buffer.WriteInt32LE(joinPkt.packetLength);
		buffer.WriteInt32LE(joinPkt.messageId);
		buffer.WriteInt32LE(joinPkt.roomName.size());
		//buffer.WriteString(joinPkt.roomName);
		break;
	case '2':
		leavePkt.messageId = 1;

		std::cout << "Room Name you wanted to leave: \n>";
		getline(std::cin, roomName);

		leavePkt.roomName = roomName;
		leavePkt.packetLength = sizeof(Packet) +
			sizeof(roomName.size()) + roomName.size();

		break;
	case '3':
		sendPkt.messageId = 1;

		std::cout << "Room Name you wanted to send Message: \n>";
		getline(std::cin, roomName);
		sendPkt.roomName = roomName;

		std::cout << "Message: \n>";
		getline(std::cin, message);
		sendPkt.message = message;

		sendPkt.packetLength = sizeof(Packet) +
			sizeof(roomName.size()) + roomName.size() +
			sizeof(message.size()) + message.size();

		break;
	default:
		break;
	}
}

int main(int argc, char** argv) {
	// Initialization
	result = Initialize("127.0.0.1", "5555");
	if (result != 0) {
		Shutingdown();
		return result;
	}

	// Defining userName
	std::cout << "Insert Username\n> ";
	getline(std::cin, userName);

	do
	{
		std::cout << "Command: \n>";
		getline(std::cin, userInput);

		if (userInput.size() > 0)
		{
			CheckUserImput(userInput);

			// Send the text
			int sendResult = send(connectSocket, (const char*)&(buffer.m_Buffer[0]), userInput.size() + 1, 0);
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
						std::cout << "SERVER> " << std::string(receivedBuffer, 0, result) << std::endl;
						printf("Received %d bytes from the Server.\n", result);
					}
				}
			}
		}

	} while (userInput.size() > 0);	

	Shutingdown();
	return 0;
}