#pragma once

#include <vector>
#include <string>

class MyBuffer {
public:
	MyBuffer(size_t size);

	void WriteInt32LE(int32_t value);
	void WriteString(std::string value);
	uint32_t ReadUInt32LE();
	std::string ReadString(std::size_t length);

	// call m_Buffer.resize in your constructor
	std::vector<uint8_t> m_Buffer;

	// initialize these at 0
	int m_WriteBuffer;
	int m_ReadBuffer;
	int m_BufferSize;

private:

};