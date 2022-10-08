#pragma once

#include <vector>

class MyBuffer {
public:
	MyBuffer(size_t size);

	void WriteInt32LE(std::size_t index, int32_t value);
	void WriteInt32LE(int32_t value);
	uint32_t ReadUInt32LE(std::size_t index);
	uint32_t ReadUInt32LE();

private:
	// call m_Buffer.resize in your constructor
	std::vector<uint8_t> m_Buffer;

	// initialize these at 0
	int m_WriteBuffer;
	int m_ReadBuffer;
};