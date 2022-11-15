#include "MyBuffer.h";
#include <iostream>

MyBuffer::MyBuffer(size_t size) {
	m_Buffer.resize(size);

	m_WriteBuffer = 0;
	m_ReadBuffer = 0;
	m_BufferSize = size;
}

void MyBuffer::WriteInt32LE(int32_t value) {
	if ((m_BufferSize - m_WriteBuffer) < sizeof(int32_t)) {
		m_BufferSize += sizeof(int32_t);
		m_Buffer.resize(m_BufferSize);
	}

	m_Buffer[m_WriteBuffer] = value;
	m_Buffer[++m_WriteBuffer] = value >> 8;
	m_Buffer[++m_WriteBuffer] = value >> 16;
	m_Buffer[++m_WriteBuffer] = value >> 24;
	m_WriteBuffer++;
}

void MyBuffer::WriteString(std::string value) {
	// Checks if the buffer array has space for the string
	if ((m_BufferSize - m_WriteBuffer) < value.size()) {
		m_BufferSize += value.size();
		m_Buffer.resize(m_BufferSize);
	}

	for (int i = 0; i < value.size(); i++) {
		m_Buffer[m_WriteBuffer] = value.at(i);
		m_WriteBuffer++;
	}
}

uint32_t MyBuffer::ReadUInt32LE() {
	uint32_t value = m_Buffer[m_ReadBuffer];
	value |= m_Buffer[++m_ReadBuffer] << 8;
	value |= m_Buffer[++m_ReadBuffer] << 16;
	value |= m_Buffer[++m_ReadBuffer] << 24;
	m_ReadBuffer++;
	return value;
}

std::string MyBuffer::ReadString(std::size_t length) {
	std::string value;
	for (int i = 0; i < length; i++) {
		int currentIndex = m_ReadBuffer + i;

		if (currentIndex < m_BufferSize) 
			value += m_Buffer[currentIndex];
	}
	return value;
}