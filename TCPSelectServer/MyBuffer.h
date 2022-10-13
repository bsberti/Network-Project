#pragma once

#include <vector>
#include <string>

class MyBuffer {
public:
	MyBuffer(size_t size);

	void WriteInt32LE(std::size_t index, int32_t value);
	void WriteInt32LE(int32_t value);
	void WriteString(std::size_t index, uint8_t value);
	void WriteString(uint8_t value);
	uint32_t ReadUInt32LE(std::size_t index);
	uint32_t ReadUInt32LE();

	// call m_Buffer.resize in your constructor
	std::vector<uint8_t> m_Buffer;

private:

	// initialize these at 0
	int m_WriteBuffer;
	int m_ReadBuffer;
};