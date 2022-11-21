#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include <SHA256.h>

#pragma comment(lib, "Ws2_32.lib")

#include "MyBuffer.h"
#include "cCreateAccountPacket.h"
#include "gen/addressbook.pb.h"

MyBuffer* buffer;

WSADATA wsaData;
int result;

// Create socket
SOCKET connectSocket;

const int recvBufLen = 1024;
char receivedBuffer[recvBufLen];

bool tryAgain = true;
bool loggedIn = false;

std::string userInput;
std::string userName;
cCreateAccountPacket pCreateAccountPacket; 
std::string serializedString;

struct Packet {
	int packetLength;
	int messageId;
};

struct LoginCommandPacket : public Packet {
	std::string name;
	std::string email;
	std::string username;
	std::string password;
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

void CheckUserCommand(std::string userImput) {
	JoinRoomCommandPacket joinPkt;
	LeaveRoomCommandPacket leavePkt;
	SendCommandPacket sendPkt;

	std::string roomName;
	std::string message;

	int messageTotalLenght;
	int messageId;
	int messageLength;

	switch (userImput[0]) {
	case '1':
		joinPkt.messageId = 1;

		std::cout << "Room Name you wanted to join: \n";
		std::cout << "(If room Name doesn't exist, it will be created)\n>";
		getline(std::cin, roomName);

		joinPkt.roomName = roomName;
		joinPkt.packetLength = sizeof(Packet) +
			sizeof(roomName.size()) + roomName.size();

		buffer->WriteInt32LE(joinPkt.packetLength);
		buffer->WriteInt32LE(joinPkt.messageId);
		buffer->WriteInt32LE(joinPkt.roomName.size());
		buffer->WriteString(joinPkt.roomName);

		messageTotalLenght = buffer->ReadUInt32LE();
		messageId = buffer->ReadUInt32LE();
		messageLength = buffer->ReadUInt32LE();
		message = buffer->ReadString(messageLength);

		std::cout << "messageLenght: " << messageTotalLenght << std::endl;
		std::cout << "messageId: " << messageId << std::endl;
		std::cout << "readMessage: " << message << std::endl;
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

void CheckUserImput(std::string userImput) {
	std::string passwordImput;
	std::string userIdImput;
	std::string emailImput;

	Authentication::CreateAccountPacket createAccountPacket;
	Authentication::LoginPacket loginPacket;
	buffer = new MyBuffer(128);

	switch (userImput[0]) {
	case '1':
		// LOGIN - Send message type 1 to Server TCP
		std::cout << "E-mail: \n>";
		getline(std::cin, emailImput);

		std::cout << "Password:\n>";
		getline(std::cin, passwordImput);

		if (emailImput != "" && passwordImput != "") {
			loginPacket.set_email(emailImput);
			loginPacket.set_hashed_password(passwordImput);

			loginPacket.SerializeToString(&serializedString);

			buffer->WriteInt32LE(sizeof(int) +
				sizeof(serializedString.length()) +
				sizeof(serializedString));
			buffer->WriteInt32LE(1);
			buffer->WriteInt32LE(serializedString.length());
			buffer->WriteString(serializedString);
		}
		break;
	case '2':
		// CREATE AN ACCOUNT - Send message type 1 to Server TCP
		std::cout << "User ID: \n>";
		getline(std::cin, userIdImput);

		std::cout << "E-Mail: \n>";
		getline(std::cin, emailImput);

		std::cout << "Password:\n>";
		getline(std::cin, passwordImput);

		if (userIdImput != "" &&
			passwordImput != "" &&
			emailImput != "") {
			
			createAccountPacket.set_userid(stoi(userIdImput));
			createAccountPacket.set_email(emailImput);

			//Defining random salt
			std::string salt;
			salt = std::to_string(rand() * 100000);			
			createAccountPacket.set_salt(salt);

			// Hashing salt + password
			std::string hashedPassword;
			SHA256 sha;
			sha.update(salt + passwordImput);
			uint8_t* digest = sha.digest();
			createAccountPacket.set_hashed_password(SHA256::toString(digest));

			delete[] digest; // Don't forget to free the digest!

			// Serializing and writing buffer
			createAccountPacket.SerializeToString(&serializedString);
			buffer->WriteInt32LE(sizeof(int) + 
				sizeof(serializedString.length()) + 
				sizeof(serializedString));
			buffer->WriteInt32LE(2);
			buffer->WriteInt32LE(serializedString.length());
			buffer->WriteString(serializedString);
		}
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

	// LOGIN OR SIGN IN
	do {
		system("CLS");
		std::cout << "CHAT SERVER\n ";
		std::cout << "Chose one of the options to continue . . .\n ";
		std::cout << "1 - Login\n ";
		std::cout << "2 - Create an account\n> ";
		getline(std::cin, userInput);
		if (userInput.size() > 0) {
			CheckUserImput(userInput);

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
							Sleep(1000);
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
							loggedIn = true;
							//LOGIN SUCESSFULL
							std::cout << "Login Sucessfull!" << std::endl;
						}
						else {
							//LOGIN FAIL
							tryAgain = false;
							loggedIn = false;
							std::cout << "Login Fail!" << std::endl;;
							Sleep(2000);
						}
					}
				}
			}
		}
	} while (!loggedIn);


	// CHAT ROOM
	do
	{
		std::cout << "Command: \n>";
		getline(std::cin, userInput);

		if (userInput.size() > 0)
		{
			CheckUserCommand(userInput);

			// Send the text
			int sendResult = send(connectSocket, (const char*)&(buffer->m_Buffer[0]), buffer->m_Buffer.size(), 0);
			if (sendResult == SOCKET_ERROR)
			{
				printf("failed to send message to the server with error %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return 1;
			} else {
				ZeroMemory(receivedBuffer, recvBufLen);
				tryAgain = true;
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