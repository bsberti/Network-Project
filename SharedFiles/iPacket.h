//#pragma once
#ifndef _iPacket_11_13_
#define _iPacket_11_13_

//#include "MyBuffer.h"

class iPacket {
public:
	virtual ~iPacket() {}

	virtual void setPacketLength(int value) = 0;
	virtual void setMessageId(int value) = 0;

	//virtual void serializePacket(MyBuffer &buffer) = 0;
};
#endif