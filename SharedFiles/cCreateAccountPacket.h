#pragma once
#include <string>
#include "MyBuffer.h"
#include "iPacket.h"

class cCreateAccountPacket :
	public iPacket
{
public:
	cCreateAccountPacket();
	~cCreateAccountPacket();

	std::string name;
	std::string email;
	std::string username;
	std::string password;

	MyBuffer* pBuffer;

	virtual void setPacketLength(int value) {
		this->packetLength = value;
	};

	virtual void setMessageId(int value) {
		this->messageId = value;
	};

	virtual int getMessageId() {
		return this->messageId;
	};

	void serializePacket(MyBuffer* buffer);
	void deserializePacket(MyBuffer* buffer);
private:
	int packetLength;
	int messageId;
};