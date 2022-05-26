#include "StringCompressor.h"
#include "HuffmanEncodingTree.h"
#include "BitStream.h"
#include <assert.h>
#include <string.h>

StringCompressor StringCompressor::instance;

unsigned long englishCharacterFrequencies[] =
{
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
5655,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
64995,
2,
515,
3,
3,
5,
13,
125,
289,
294,
0,
0,
1968,
558,
4849,
91,
244,
795,
459,
323,
379,
210,
155,
138,
280,
590,
150,
46,
4,
3,
4,
69,
28,
595,
161,
950,
364,
430,
326,
221,
308,
712,
189,
56,
181,
318,
426,
256,
676,
14,
458,
1247,
853,
253,
105,
297,
53,
81,
2,
235,
0,
235,
0,
2,
0,
11400,
2156,
6665,
5543,
19692,
3186,
2759,
5614,
12742,
57,
1191,
6009,
4271,
11288,
13376,
3619,
106,
10492,
12069,
14885,
5462,
1664,
1850,
265,
3591,
169,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0
};

StringCompressor::StringCompressor()
{
}

void StringCompressor::GenerateHuffmanEncodingTree(void)
{
	huffmanEncodingTree = new HuffmanEncodingTree;
	huffmanEncodingTree->GenerateFromFrequencyTable(englishCharacterFrequencies);
}
StringCompressor::~StringCompressor()
{
	if (huffmanEncodingTree)
		delete huffmanEncodingTree;
}

void StringCompressor::EncodeString(char *input, int maxCharsToWrite, RakNet::BitStream *output)
{
	if (input==0)
		return;

	RakNet::BitStream encodedBitStream;
	unsigned short stringBitLength;
	int charsToWrite;

	if (huffmanEncodingTree==0)
		GenerateHuffmanEncodingTree();

	if ((int)strlen(input) < maxCharsToWrite)
		charsToWrite=(int)strlen(input);
	else
		charsToWrite=maxCharsToWrite-1;

	huffmanEncodingTree->EncodeArray((unsigned char*) input, charsToWrite, &encodedBitStream);

	stringBitLength = (unsigned short)encodedBitStream.GetNumberOfBitsUsed();
	output->WriteCompressed(stringBitLength);
	output->WriteBits(encodedBitStream.GetData(), stringBitLength);
}

void StringCompressor::DecodeString(char *output, int maxCharsToWrite, RakNet::BitStream *input)
{
	unsigned short stringBitLength;
	int bytesInStream;

	if (huffmanEncodingTree==0)
		GenerateHuffmanEncodingTree(); 

	output[0]=0;

	
	if (input->ReadCompressed(stringBitLength)==false)
		return;

	bytesInStream=huffmanEncodingTree->DecodeArray(input, stringBitLength, maxCharsToWrite, (unsigned char*) output);
	if (bytesInStream < maxCharsToWrite)
		output[bytesInStream]=0;
	else
		output[maxCharsToWrite-1]=0;
}

