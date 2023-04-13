#include <iostream>
#include <cstdint>
#include <sstream>

using namespace std;

uint8_t S[] = { 0xC, 0x5, 0x6, 0xB, 0x9, 0x0, 0xA, 0xD, 0x3, 0xE, 0xF, 0x8, 0x4,
		0x7, 0x1, 0x2 };

uint8_t P[] = { 0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4,
		20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54, 7, 23, 39, 55, 8, 24, 40, 56,
		9, 25, 41, 57, 10, 26, 42, 58, 11, 27, 43, 59, 12, 28, 44, 60, 13, 29,
		45, 61, 14, 30, 46, 62, 15, 31, 47, 63 };

struct byte {
	uint8_t bytesNibbleA :4;
	uint8_t bytesNibbleB :4;
} __attribute__((packed));
// packed to avoid automatic padding by compiler

//converting HexaDecimalString To Bytes
byte* hexaDecimalStringToBytes(char *input) {
	byte *bytes = new byte[8];
	int i;
//If between'0' & '9' the we  subtract '0' from it for decimal
//If between 'a' & 'f' then we 'a' from it and add 10 for decimal
	for (i = 0; i < 8; i++) {
		bytes[i].bytesNibbleA =
				(input[2 * i] >= '0' && input[2 * i] <= '9') ?
						(input[2 * i] - '0') : (input[2 * i] - 'a' + 10);
		bytes[i].bytesNibbleB =
				(input[2 * i + 1] >= '0' && input[2 * i + 1] <= '9') ?
						(input[2 * i + 1] - '0') :
						(input[2 * i + 1] - 'a' + 10);
	}
	return bytes;
}

uint64_t hexaDecimalStringToLong(char *input) {
//	converts hexadecimal str to uint64_t
//	left-shifts result current value by 4 bits
	//If  between '0' & '9' the we  subtract '0' from it for decimal
	//If between 'a' & 'f' then we subtract 'a' from it and add 10 for decimal
	uint64_t result = 0;
	for (int i = 0; i < 16; i++) {
		result = (result << 4)
				| ((input[i] >= '0' && input[i] <= '9') ?
						(input[i] - '0') : (input[i] - 'a' + 10));
	}
	return result;
}

//converting a uint64_t into HexaDecimalString
char* longToHexaDecimalString(uint64_t longinput) {
	char *hexaDecimalString = new char[17];
	//using sprintf to format longinput into hexadecimal
	sprintf(hexaDecimalString, "%016llx", longinput);
	return hexaDecimalString;
}

//converting byte array into uint64_t
uint64_t bytesToLong(byte *bytes) {
	uint64_t result = 0;
	// left-shifts result current value by 4 bits
	// performs a bitwise OR operation with the 4 least significant bits
	for (int i = 0; i < 8; i++) {
		result = (result << 4) | (bytes[i].bytesNibbleA & 0xF);
		result = (result << 4) | (bytes[i].bytesNibbleB & 0xF);
	}
	return result;
}

// converting uint64_t into bytes
byte* longToBytes(uint64_t input) {
	byte *bytes = (byte*) malloc(8 * sizeof(byte));
	int i;
	for (i = 7; i >= 0; i--) {
// shifting bits
		bytes[i].bytesNibbleB = (input >> 2 * (7 - i) * 4) & 0xF;
		bytes[i].bytesNibbleA = (input >> (2 * (7 - i) + 1) * 4) & 0xF;
	}
	return bytes;
}

uint8_t substitutionWithSBox(uint8_t input) {
//	using to substitue S box values, this just returns S box value
	return S[input];
}

uint64_t doPermutationOnInput(uint64_t input) {
	uint64_t permutateIntVariable = 0;

	for (int i = 0; i < 64; i++) {
		int distance = 63 - i;	//getting dist for each bit
	 permutateIntVariable = permutateIntVariable | ((input >> distance & 0x1) << 63 - P[i]); // bitwise operations
	}
	return permutateIntVariable;
}

uint16_t getKeyRightLow(char *inputKey) {

	uint16_t keyRightLow = 0;
	for (int i = 16; i < 20; i++)
		keyRightLow = (keyRightLow << 4)
				| (((inputKey[i] >= '0' && inputKey[i] <= '9') ?
						(inputKey[i] - '0') : (inputKey[i] - 'a' + 10)) & 0xF);
	return keyRightLow;
}

uint64_t* generateSubkeys(char *inputKey) {
	//Generating 32 64-bit sub keys with the input key

	uint64_t keyLeftHigh = hexaDecimalStringToLong(inputKey);
	uint16_t keyRightLow = getKeyRightLow(inputKey);
	uint64_t *subKeys = new uint64_t[32]; // 32 (64-bit) sub keys will gets stored here
	cout << "Key/keyLeftHigh/keyRightLow " << inputKey << "  "
			<< longToHexaDecimalString(keyLeftHigh) << " "
			<< longToHexaDecimalString(keyRightLow) << endl;
	subKeys[0] = keyLeftHigh;

	for (int i = 1; i < 32; i++) {
		uint64_t temporaryVar1 = keyLeftHigh;//64 bit var
		uint64_t temporaryVar2 = keyRightLow;//64 bit var
		uint8_t temporaryVar; //80 bit
		keyLeftHigh = (keyLeftHigh << 61) | (temporaryVar2 << 45) | (temporaryVar1 >> 19);
		keyRightLow = ((temporaryVar1 >> 3) & 0xFFFF);

		temporaryVar = substitutionWithSBox(keyLeftHigh >> 60);
		keyLeftHigh = keyLeftHigh & 0x0FFFFFFFFFFFFFFF;

		keyLeftHigh = keyLeftHigh | (((uint64_t) temporaryVar) << 60);
		keyRightLow = keyRightLow ^ ((i & 0x01) << 15);
		keyLeftHigh = keyLeftHigh ^ (i >> 1);
		subKeys[i] = keyLeftHigh;// substuting key left at i place in  sub keys
	}
	return subKeys;
}

char* encrypt(char *plaintext, char *inputKey) {

	uint64_t *substitutionkeys = generateSubkeys(inputKey); //this fun gets us array of sub keys

	uint64_t state = hexaDecimalStringToLong(plaintext); //converting plain text hexString into 64-bit uint64_t

	for (int i = 0; i < 31; i++) {
		//performing XOR operation with the state & round sub key
		state = state ^ substitutionkeys[i];
		//converting state from uint64_t into bytes/nibbles
		byte *stateBytes = longToBytes(state);

		for (int j = 0; j < 8; j++) {
			//substituting state nibble bytes with S box bytes
			stateBytes[j].bytesNibbleA = substitutionWithSBox(stateBytes[j].bytesNibbleA);
			stateBytes[j].bytesNibbleB = substitutionWithSBox(stateBytes[j].bytesNibbleB);
		}

		//converting bytes into uint64_t format and passing to permutation function
		state = doPermutationOnInput(bytesToLong(stateBytes));
		//clearing the allocated memory of stateBytes pointer
		delete[] stateBytes;
	}
	//Finally we perform XOR operation with the state & round
	state = state ^ substitutionkeys[31];
	//clearing the allocated memory of sub keys pointer
	delete[] substitutionkeys;

	return longToHexaDecimalString(state);
}

int main() {
//	initialising variables
	char *ciphertext;
	char *plaintext = new char[17];
	char *inputKey = new char[21];

	strcpy(plaintext, "ffffffffffffffff"); 	// Assigning values
	strcpy(inputKey, "ffffffffffffffffffff");

//	strcpy(plaintext, "ffffffffffffffff");
//	strcpy(inputKey, "00000000000000000000");

//	strcpy(plaintext, "0000000000000000");
//	strcpy(inputKey, "ffffffffffffffffffff");

	ciphertext = encrypt(plaintext, inputKey); // calling encrypt by passing inputs

	cout << "cipher text: " << ciphertext << endl;

	delete[] inputKey;
	delete[] plaintext;
	delete[] ciphertext;

	return 0;
}

