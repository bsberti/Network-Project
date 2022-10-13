#include "MyBuffer.h";


MyBuffer::MyBuffer(size_t size) {
	m_Buffer.resize(size);

	m_WriteBuffer = 0;
	m_ReadBuffer = 0;
}

void MyBuffer::WriteInt32LE(std::size_t index, int32_t value) {
	m_Buffer[index] = value;			// x x x O
	m_Buffer[index + 1] = value >> 8;	// x x O x
	m_Buffer[index + 2] = value >> 16;	// x O x x
	m_Buffer[index + 3] = value >> 24;	// O x x x
}

void MyBuffer::WriteInt32LE(int32_t value) {
	m_Buffer[m_WriteBuffer] = value;
	m_Buffer[m_WriteBuffer + 1] = value >> 8;
	m_Buffer[m_WriteBuffer + 2] = value >> 16;
	m_Buffer[m_WriteBuffer + 3] = value >> 24;
}

void MyBuffer::WriteString(std::size_t index, uint8_t value) {
	m_Buffer[index] = value;
}

void MyBuffer::WriteString(uint8_t value) {
	m_Buffer[m_WriteBuffer] = value;
}

uint32_t MyBuffer::ReadUInt32LE(std::size_t index) {
	uint32_t value = m_Buffer[index];	// [244] 1 0 0
	value |= m_Buffer[index + 1] << 8;	// 244 [1] 0 0
	value |= m_Buffer[index + 2] << 16;	// 244 1 [0] 0
	value |= m_Buffer[index + 3] << 24;	// 244 1 0 [0]

	return value;
}

uint32_t MyBuffer::ReadUInt32LE() {
	uint32_t value = m_Buffer[m_ReadBuffer];
	value |= m_Buffer[m_ReadBuffer + 1] << 8;
	value |= m_Buffer[m_ReadBuffer + 2] << 16;
	value |= m_Buffer[m_ReadBuffer + 3] << 24;

	return value;
}
