#include "cCreateAccountPacket.h"
#include <iostream>

cCreateAccountPacket::cCreateAccountPacket() {
	this->packetLength = 0;
	this->messageId = 0;
}

cCreateAccountPacket::~cCreateAccountPacket() {

}


// Function to serialized the array of string
std::string serializeString(std::string str[], int ln)
{
	std::string temp = "";
	int stringLen;
	for (int i = 0; i < ln; i++) {
		stringLen = str[i].length();
		temp.push_back('0' + stringLen);
		temp = temp + "|" + str[i];
	}
	return temp;
}

// Function to deserialize the string
std::vector<std::string> deserializedString(std::string str)
{
	std::vector<std::string> deserialized;
	
	int stringLen, pos = 0;
	std::string temp = "";
	//int i = 0;
	while (pos > -1) {
		pos = str.find("|", pos + 1);
		if (pos > 0) {
			stringLen = str[pos - 1] - 48;
			temp.append(str, pos + 1, stringLen);
			deserialized.push_back(temp);
			temp = "";
		}
	}
	return deserialized;
}

void cCreateAccountPacket::serializePacket(MyBuffer* buffer) {
	pBuffer = buffer;
	pBuffer->WriteInt32LE(this->packetLength);
	pBuffer->WriteInt32LE(this->messageId);
	pBuffer->WriteInt32LE(sizeof(this->name) +
		sizeof(this->email) + 
		sizeof(this->username) + 
		sizeof(this->password));

	std::string message[4];
	message[0] = this->name;
	message[1] = this->email;
	message[2] = this->username;
	message[3] = this->password;

	int ln = sizeof(message) / sizeof(message[0]);
	std::string serializedstr = serializeString(message, ln);

	pBuffer->WriteString(serializedstr);

	//pbuffer->WriteString(this->name + "|");
	//pbuffer->WriteString(this->email + "|");
	//pbuffer->WriteString(this->username + "|");
	//pbuffer->WriteString(this->password + "|");
}

void cCreateAccountPacket::deserializePacket(MyBuffer* buffer) {
	pBuffer = buffer;
	//int messageTotalLenght = pBuffer->ReadUInt32LE();
	//int messageId = pBuffer->ReadUInt32LE();
	int messageLength = pBuffer->ReadUInt32LE();
	std::string message = pBuffer->ReadString(messageLength);

	std::vector<std::string> deserializedStrings;
	deserializedStrings.resize(4);
	deserializedStrings = deserializedString(message);

	this->name = deserializedStrings[0];
	this->email = deserializedStrings[1];
	this->username = deserializedStrings[2];
	this->password = deserializedStrings[3];
}