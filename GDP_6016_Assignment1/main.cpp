#include <vector>

#include "MyBuffer.h"

void MyBufferExample() {
	// Buffer Class Workshop
	MyBuffer myBuffer(8);

	myBuffer.WriteInt32LE(50);
	myBuffer.WriteInt32LE(4, 500);

	int myReadValue50 = myBuffer.ReadUInt32LE();
	int myReadValue500 = myBuffer.ReadUInt32LE(4);

	printf("%d MyBuffer should be 50\n", myReadValue50);
	printf("%d MyBuffer should be 500\n", myReadValue500);
}

int main(int argc, char** argv) {

	//MyBufferExample();

	int breakpoint = 1;

	return 0;
}
