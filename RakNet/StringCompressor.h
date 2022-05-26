// Used to encode text.  Generated from a big text file.

#ifndef __STRING_COMPRESSOR_H
#define __STRING_COMPRESSOR_H

#include "BitStream.h"
class HuffmanEncodingTree;

class StringCompressor
{
public:

	~StringCompressor();
	// static function because only static functions can access static members
	static inline StringCompressor* Instance() {return &instance;}

	// Writes input to output, compressed.  Takes care of the null terminator for you
	void EncodeString(char *input, int maxCharsToWrite, RakNet::BitStream *output);

	// Writes input to output, uncompressed.  Takes care of the null terminator for you.
	// maxCharsToWrite should be the allocated size of output
	void DecodeString(char *output, int maxCharsToWrite, RakNet::BitStream *input);
private:
	void GenerateHuffmanEncodingTree(void);
	StringCompressor();
	static StringCompressor instance;
	HuffmanEncodingTree *huffmanEncodingTree;
};

#define stringCompressor StringCompressor::Instance()

#endif
