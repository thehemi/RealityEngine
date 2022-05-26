#include "DataBlockEncryptor.h"
#include "CheckSum.h"
#include "GetTime.h"
#include "Rand.h"
#include <assert.h>
#include <string.h>

DataBlockEncryptor::DataBlockEncryptor()
{
	keySet=false;
}
DataBlockEncryptor::~DataBlockEncryptor()
{
}
bool DataBlockEncryptor::IsKeySet(void) const
{
	return keySet;
}

void DataBlockEncryptor::SetKey(const unsigned char key[16])
{
	keySet=true;
	secretKeyAES128.set_key(key);
}

void DataBlockEncryptor::UnsetKey(void)
{
	keySet=false;
}

void DataBlockEncryptor::Encrypt(unsigned char *input, int inputLength, unsigned char *output, int *outputLength)
{
	unsigned index, byteIndex, lastBlock;
	unsigned long checkSum;
	unsigned char paddingBytes;
	unsigned char encodedPad;
	unsigned char randomChar;
	CheckSum checkSumCalculator;

#ifdef _DEBUG
	assert(keySet);
#endif

	assert(input && inputLength);

	// randomChar will randomize the data so the same data sent twice will not look the same
	randomChar=(unsigned char)randomMT();

	// 16-(((x-1) % 16)+1) 

	// # of padding bytes is 16 -(((input_length + extra_data -1) % 16)+1)
	paddingBytes = (unsigned char)(16 -(((inputLength + sizeof(randomChar)+sizeof(checkSum)+sizeof(encodedPad) -1) % 16)+1));
	*outputLength=inputLength + sizeof(randomChar)+sizeof(checkSum)+sizeof(encodedPad)+paddingBytes;

	// Randomize the pad size variable
	encodedPad=(unsigned char)randomMT();
	encodedPad<<=4;
	encodedPad|=paddingBytes;

	// Write the data first, in case we are overwriting ourselves
	if (input==output)
		memmove(output+sizeof(checkSum)+sizeof(randomChar)+sizeof(encodedPad)+paddingBytes, input, inputLength);
	else
		memcpy(output+sizeof(checkSum)+sizeof(randomChar)+sizeof(encodedPad)+paddingBytes, input, inputLength);

	// Write the random char
	memcpy(output+sizeof(checkSum), (char*)&randomChar, sizeof(randomChar));

	// Write the pad size variable
	memcpy(output+sizeof(checkSum)+sizeof(randomChar), (char*)&encodedPad, sizeof(encodedPad));

	// Write the padding
	for (index=0; index < paddingBytes; index++)
		*(output+sizeof(checkSum)+sizeof(randomChar)+sizeof(encodedPad)+index)=(unsigned char)randomMT();

	// Calculate the checksum on the data
	checkSumCalculator.add(output+sizeof(checkSum), inputLength+sizeof(randomChar)+sizeof(encodedPad)+paddingBytes);
	checkSum=checkSumCalculator.get();

	// Write checksum
	memcpy(output, (char*)&checkSum, sizeof(checkSum));

	// AES on the first block
	secretKeyAES128.encrypt16(output);
	lastBlock=0;

	// Now do AES on every other block from back to front
	for (index=*outputLength-16; index >= 16; index-=16)
	{
		for (byteIndex=0; byteIndex<16; byteIndex++)
			output[index+byteIndex]^=output[lastBlock+byteIndex];

		secretKeyAES128.encrypt16(output+index);

		lastBlock=index;
	}
}

bool DataBlockEncryptor::Decrypt(unsigned char *input, int inputLength, unsigned char *output, int *outputLength)
{
	unsigned index, byteIndex, lastBlock;
	unsigned long checkSum;
	unsigned char paddingBytes;
	unsigned char encodedPad;
	unsigned char randomChar;
	CheckSum checkSumCalculator;
#ifdef _DEBUG
	assert(keySet);
#endif
	
	if (input==0 || inputLength<16 || (inputLength % 16) != 0)
	{
		return false;
	}

	// Unchain in reverse order
	for (index=16; (int)index <= inputLength-16;index+=16)
	{
		secretKeyAES128.decrypt16(input+index);

		for (byteIndex=0; byteIndex<16; byteIndex++)
		{
			if (index+16==(unsigned)inputLength)
				input[index+byteIndex]^=input[byteIndex];
			else
				input[index+byteIndex]^=input[index+16+byteIndex];
		}

		lastBlock=index;
	};

	// Decrypt the first block
	secretKeyAES128.decrypt16(input);

	// Read checksum
	memcpy((char*)&checkSum, input, sizeof(checkSum));

	// Read the pad size variable
	memcpy((char*)&encodedPad, input+sizeof(randomChar)+sizeof(checkSum), sizeof(encodedPad));

	// Ignore the high 4 bytes
	paddingBytes=encodedPad&0x0F;

	// Get the data length
	*outputLength=inputLength - sizeof(randomChar)-sizeof(checkSum)-sizeof(encodedPad)-paddingBytes;

	// Calculate the checksum on the data
	checkSumCalculator.add(input+sizeof(checkSum), *outputLength+sizeof(randomChar)+sizeof(encodedPad)+paddingBytes);
	if (checkSum != checkSumCalculator.get())
		return false;

	// Read the data
	if (input==output)
		memmove(output, input+sizeof(randomChar)+sizeof(checkSum)+sizeof(encodedPad)+paddingBytes, *outputLength);
	else
		memcpy(output, input+sizeof(randomChar)+sizeof(checkSum)+sizeof(encodedPad)+paddingBytes, *outputLength);

	return true;
}
