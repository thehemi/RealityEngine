// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

// Uncomment this to check that read and writes match with the same type and in the case of streams, size.  Useful during debugging
//#define TYPE_CHECKING

#ifdef TYPE_CHECKING
#ifndef _DEBUG
#ifdef _WIN32
#pragma message("Warning: TYPE_CHECKING is defined in BitStream.cpp when not in _DEBUG mode" )
#endif
#endif
#endif

#include "BitStream.h"
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>

using namespace RakNet;

BitStream::BitStream()
{
	numberOfBitsUsed=0;
	numberOfBitsAllocated=32 * 8;
	readOffset=0;
	data=(unsigned char*)malloc(32);
#ifdef _DEBUG
	assert(data);
#endif
	memset(data, 0, 32);
	copyData=true;
}

BitStream::BitStream(int initialBytesToAllocate)
{
	numberOfBitsUsed=0;
	numberOfBitsAllocated=initialBytesToAllocate<<3;
	readOffset=0;
	data=(unsigned char*)malloc(initialBytesToAllocate);
	#ifdef _DEBUG
	assert(data);
	#endif
	memset(data, 0, initialBytesToAllocate);
	copyData=true;
}

BitStream::BitStream(const char* _data, unsigned int lengthInBytes, bool _copyData)
{
	numberOfBitsUsed=lengthInBytes<<3;
	numberOfBitsAllocated=lengthInBytes<<3;
	readOffset=0;
	copyData=_copyData;
	if (copyData)
	{
		if (lengthInBytes>0)
		{
			data=(unsigned char*)malloc(lengthInBytes);
			#ifdef _DEBUG
			assert(data);
			#endif
			memcpy(data, _data, lengthInBytes);
		}
		else
			data=0;
	}
	else
		data=(unsigned char*)_data;
}

// Use this if you pass a pointer copy to the constructor (_copyData==false) and want to overallocate to prevent reallocation
void BitStream::SetNumberOfBitsAllocated(unsigned int lengthInBits)
{
	#ifdef _DEBUG
	assert(lengthInBits>=(unsigned int)numberOfBitsAllocated);
	#endif
	numberOfBitsAllocated=lengthInBits;
}

BitStream::~BitStream()
{
	if (copyData)
		free(data);  // Use realloc and free so we are more efficient than delete and new for resizing
}

void BitStream::Reset(void)
{
	if (numberOfBitsUsed>0)
		memset(data, 0, BITS_TO_BYTES(numberOfBitsUsed));
	// Don't free memory here for speed efficiency
	//free(data);  // Use realloc and free so we are more efficient than delete and new for resizing
	numberOfBitsUsed=0;
	//numberOfBitsAllocated=8;
	readOffset=0;
	//data=(unsigned char*)malloc(1);
//	if (numberOfBitsAllocated>0)
//		memset(data, 0, BITS_TO_BYTES(numberOfBitsAllocated));
}

// Write the native types to the end of the buffer
void BitStream::Write(bool input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 0;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif

	if (input)
		Write1();
	else
		Write0();
}

void BitStream::Write(unsigned char  input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 1;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif

	WriteBits((unsigned char*)&input, sizeof(input)*8, true);
}

void BitStream::Write(char input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 2;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteBits((unsigned char*)&input, sizeof(input)*8, true);
}

void BitStream::Write(unsigned short input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 3;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteBits((unsigned char*)&input, sizeof(input)*8, true);
}

void BitStream::Write(short input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 4;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteBits((unsigned char*)&input, sizeof(input)*8, true);
}

void BitStream::Write(unsigned int input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 5;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteBits((unsigned char*)&input, sizeof(input)*8, true);
}

void BitStream::Write(int input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 6;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteBits((unsigned char*)&input, sizeof(input)*8, true);
}

void BitStream::Write(unsigned long input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 7;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteBits((unsigned char*)&input, sizeof(input)*8, true);
}

void BitStream::Write(long input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 8;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteBits((unsigned char*)&input, sizeof(input)*8, true);
}

void BitStream::Write(float input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 9;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteBits((unsigned char*)&input, sizeof(input)*8, true);
}

void BitStream::Write(double input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 10;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteBits((unsigned char*)&input, sizeof(input)*8, true);
}

// Write an array or casted stream
void BitStream::Write(char* input, int numberOfBytes)
{
#ifdef TYPE_CHECKING
	unsigned char ID =11;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
	WriteBits((unsigned char*)&numberOfBytes, sizeof(int)*8, true);
#endif

	WriteBits((unsigned char*)input, numberOfBytes*8, true);
}

// Write the native types with simple compression.
// Best used with  negatives and positives close to 0
void BitStream::WriteCompressed(unsigned char  input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 12;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteCompressed((unsigned char*)&input, sizeof(input)*8, true);
}

void BitStream::WriteCompressed(char input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 13;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteCompressed((unsigned char*)&input, sizeof(input)*8, false);
}

void BitStream::WriteCompressed(unsigned short input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 14;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteCompressed((unsigned char*)&input, sizeof(input)*8, true);
}

void BitStream::WriteCompressed(short input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 15;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteCompressed((unsigned char*)&input, sizeof(input)*8, false);
}

void BitStream::WriteCompressed(unsigned int input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 16;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteCompressed((unsigned char*)&input, sizeof(input)*8, true);
}

void BitStream::WriteCompressed(int input)
{
#ifdef TYPE_CHECKING
	unsigned char ID =17;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteCompressed((unsigned char*)&input, sizeof(input)*8, false);
}

void BitStream::WriteCompressed(unsigned long input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 18;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteCompressed((unsigned char*)&input, sizeof(input)*8, true);
}

void BitStream::WriteCompressed(long input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 19;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteCompressed((unsigned char*)&input, sizeof(input)*8, false);
}

void BitStream::WriteCompressed(float input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 20;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	// Not yet implemented
	WriteBits((unsigned char*)&input,sizeof(input)*8, true);
}

void BitStream::WriteCompressed(double input)
{
#ifdef TYPE_CHECKING
	unsigned char ID = 21;
	WriteBits((unsigned char*)&ID, sizeof(unsigned char)*8, true);
#endif
	WriteBits((unsigned char*)&input,sizeof(input)*8, true);
}

// Read the native types from the front of the buffer
// Write the native types to the end of the buffer
bool BitStream::Read(bool& output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	#ifdef _DEBUG
	assert(ID==0);
	#endif
#endif

	//assert(readOffset+1 <=numberOfBitsUsed); // If this assert is hit the stream wasn't long enough to read from
	if (readOffset+1 >numberOfBitsUsed)
		return false;

	//if (ReadBit()) // Check that bit	
	if (data[readOffset>>3] & (0x80 >> (readOffset++ %8))) // Is it faster to just write it out here?
		output=true;
	else
		output=false;

	return true;
}

bool BitStream::Read(unsigned char  &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==1);
#endif

	return ReadBits((unsigned char*)&output, sizeof(output)*8);
}

bool BitStream::Read(char &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==2);
#endif

	return ReadBits((unsigned char*)&output, sizeof(output)*8);
}

bool BitStream::Read(unsigned short &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==3);
#endif

	return ReadBits((unsigned char*)&output, sizeof(output)*8);
}

bool BitStream::Read(short &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==4);
#endif

	return ReadBits((unsigned char*)&output, sizeof(output)*8);
}

bool BitStream::Read(unsigned int &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==5);
#endif

	return ReadBits((unsigned char*)&output, sizeof(output)*8);
}

bool BitStream::Read(int &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==6);
#endif

	return ReadBits((unsigned char*)&output, sizeof(output)*8);
}

bool BitStream::Read(unsigned long &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==7);
#endif

	return ReadBits((unsigned char*)&output, sizeof(output)*8);
}

bool BitStream::Read(long &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==8);
#endif

	return ReadBits((unsigned char*)&output, sizeof(output)*8);
}

bool BitStream::Read(float &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==9);
#endif

	return ReadBits((unsigned char*)&output, sizeof(output)*8);
}

bool BitStream::Read(double &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==10);
#endif

	return ReadBits((unsigned char*)&output, sizeof(output)*8);
}

// Read an array or casted stream
bool BitStream::Read(char* output, int numberOfBytes)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==11);
	int NOB;
	ReadBits((unsigned char*)&NOB, sizeof(int)*8);
	assert(NOB==numberOfBytes);
#endif

	return ReadBits((unsigned char*)output, numberOfBytes*8);
}

// Read the types you wrote with WriteCompressed
bool BitStream::ReadCompressed(unsigned char & output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==12);
#endif
	return ReadCompressed((unsigned char*)&output, sizeof(output)*8, true);
}

bool BitStream::ReadCompressed(char &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==13);
#endif
	return ReadCompressed((unsigned char*)&output, sizeof(output)*8, false);
}

bool BitStream::ReadCompressed(unsigned short &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==14);
#endif
	return ReadCompressed((unsigned char*)&output, sizeof(output)*8, true);
}

bool BitStream::ReadCompressed(short &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==15);
#endif
	return ReadCompressed((unsigned char*)&output, sizeof(output)*8, false);
}

bool BitStream::ReadCompressed(unsigned int &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==16);
#endif
	return ReadCompressed((unsigned char*)&output, sizeof(output)*8, true);
}

bool BitStream::ReadCompressed(int &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==17);
#endif
	return ReadCompressed((unsigned char*)&output, sizeof(output)*8, false);
}

bool BitStream::ReadCompressed(unsigned long &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==18);
#endif
	return ReadCompressed((unsigned char*)&output, sizeof(output)*8, true);
}

bool BitStream::ReadCompressed(long& output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==19);
#endif
	return ReadCompressed((unsigned char*)&output, sizeof(output)*8, false);
}

bool BitStream::ReadCompressed(float &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==20);
#endif
	// Not yet implemented
	return ReadBits((unsigned char*)&output, sizeof(output)*8);
}

bool BitStream::ReadCompressed(double &output)
{
#ifdef TYPE_CHECKING
	unsigned char ID;
	if (ReadBits((unsigned char*)&ID, sizeof(unsigned char)*8)==false)
		return false;
	assert(ID==21);
#endif
	// Not yet implemented
	return ReadBits((unsigned char*)&output, sizeof(output)*8);
}

// Sets the read pointer back to the beginning of your data.
void BitStream::ResetReadPointer(void)
{
	readOffset=0;
}

// Write a 0
void  BitStream::Write0(void)
{
	AddBitsAndReallocate(1);

	// New bits are set to 0 as default
	numberOfBitsUsed++;
}

 // Write a 1
void  BitStream::Write1(void)
{
	AddBitsAndReallocate(1);

	data[numberOfBitsUsed>>3] |= 0x80 >> (numberOfBitsUsed%8); // Set the bit to 1
	numberOfBitsUsed++; // This ++ was in the line above - but boundschecker didn't like that for some reason.
}

// Returns true if the next data read is a 1, false if it is a 0
bool BitStream::ReadBit(void)
{
#pragma warning( disable : 4800 )
	return (bool)(data[readOffset>>3] & (0x80 >> (readOffset++ %8)));
#pragma warning( default : 4800 )
}

// Align the bitstream to the byte boundary and then write the specified number of bits.
// This is faster than WriteBits but wastes the bits to do the alignment and requires you to call
// SetReadToByteAlignment at the corresponding read position 
void BitStream::WriteAlignedBytes(unsigned char* input, int numberOfBytesToWrite)
{
	#ifdef _DEBUG
	assert(numberOfBytesToWrite>0);
	#endif

	AlignWriteToByteBoundary();
	// Allocate enough memory to hold everything
	AddBitsAndReallocate(numberOfBytesToWrite<<3);

	// Write the data
	memcpy(data + (numberOfBitsUsed>>3), input, numberOfBytesToWrite);

	numberOfBitsUsed+=numberOfBytesToWrite<<3;
}

// Read bits, starting at the next aligned bits. Note that the modulus 8 starting offset of the
// sequence must be the same as was used with WriteBits. This will be a problem with packet coalescence
// unless you byte align the coalesced packets.
bool BitStream::ReadAlignedBytes(unsigned char* output, int numberOfBytesToRead)
{
	#ifdef _DEBUG
	assert(numberOfBytesToRead>0);
	#endif

	if (numberOfBytesToRead<=0)
		return false;

	// Byte align
	AlignReadToByteBoundary();

	if (readOffset+(numberOfBytesToRead<<3) >numberOfBitsUsed)
		return false;	

	// Write the data
	memcpy(output, data + (readOffset>>3), numberOfBytesToRead);

	readOffset+=numberOfBytesToRead<<3;

	return true;
}

// Align the next write and/or read to a byte boundary.  This can be used to 'waste' bits to byte align for efficiency reasons
void BitStream::AlignWriteToByteBoundary(void)
{
	if (numberOfBitsUsed)
		numberOfBitsUsed+= 8 - ((numberOfBitsUsed -1) %8 + 1);
}

// Align the next write and/or read to a byte boundary.  This can be used to 'waste' bits to byte align for efficiency reasons
void BitStream::AlignReadToByteBoundary(void)
{
	if (readOffset)
		readOffset+= 8 - ((readOffset -1) %8 + 1);
}

// Write numberToWrite bits from the input source
void BitStream::WriteBits(unsigned char *input, int numberOfBitsToWrite, bool rightAlignedBits)
{
//	if (numberOfBitsToWrite<=0)
//		return;

	AddBitsAndReallocate(numberOfBitsToWrite);
	int offset=0;
	unsigned char dataByte;
	int numberOfBitsUsedMod8;

	numberOfBitsUsedMod8=numberOfBitsUsed%8;

	// Faster to put the while at the top surprisingly enough
	while(numberOfBitsToWrite>0)
	//do
	{
		dataByte = *(input+offset);
		if (numberOfBitsToWrite<8 && rightAlignedBits) // rightAlignedBits means in the case of a partial byte, the bits are aligned from the right (bit 0) rather than the left (as in the normal internal representation)
			dataByte<<=8-numberOfBitsToWrite;	 // shift left to get the bits on the left, as in our internal representation

		// Copy over the new data.
		*(data+(numberOfBitsUsed>>3)) |= dataByte >> (numberOfBitsUsedMod8); // First half
		if (8 - (numberOfBitsUsedMod8) < 8 && 8 - (numberOfBitsUsedMod8) < numberOfBitsToWrite) // If we didn't write it all out in the first half (8 - (numberOfBitsUsed%8) is the number we wrote in the first half)
			*(data+(numberOfBitsUsed>>3) + 1) |= (unsigned char)(dataByte << (8 - (numberOfBitsUsedMod8))); // Second half (overlaps byte boundary)
	
		if (numberOfBitsToWrite>=8)
			numberOfBitsUsed+= 8;
		else
		{
			numberOfBitsUsed+= numberOfBitsToWrite;
		}
		numberOfBitsToWrite-= 8;
		
		offset++;
	}
//	} while(numberOfBitsToWrite>0);

}

// Set the stream to some initial data.  For internal use
void BitStream::SetData(const unsigned char* input, int numberOfBits)
{
	#ifdef _DEBUG
	assert(numberOfBitsUsed==0); // Make sure the stream is clear
	#endif
	if (numberOfBits<=0)
		return;

	AddBitsAndReallocate(numberOfBits);
	memcpy(data, input, BITS_TO_BYTES(numberOfBits));
	numberOfBitsUsed=numberOfBits;
}

// Assume the input source points to a native type, compress and write it
void  BitStream::WriteCompressed(unsigned char* input, int size, bool unsignedData)
{
	int currentByte=(size>>3)-1; // PCs
	
	unsigned char byteMatch;
	if (unsignedData)
	{
		byteMatch=0;
	}
	else
	{
		byteMatch=0xFF;
	}

	// Write upper bytes with a single 1
	// From high byte to low byte, if high byte is a byteMatch then write a 1 bit.  Otherwise write a 0 bit and then write the remaining bytes
	while (currentByte>0)
	{
		if (input[currentByte]==byteMatch) // If high byte is byteMatch (0 of 0xff) then it would have the same value shifted
		{
			bool b = true;
			Write(b);
		}
		else
		{
			// Write the remainder of the data after writing 0
			bool b = false;
			Write(b);

			WriteBits(input,(currentByte+1)<<3, true);
			currentByte--;

	
			return;
		}

		currentByte--;
	}

	// If the upper half of the last byte is a 0 (positive) or 16 (negative) then write a 1 and the remaining 4 bits.  Otherwise write a 0 and the 8 bites.
	if ((unsignedData && ((*(input+currentByte))&0xF0) ==0x00) ||
		(unsignedData==false && ((*(input+currentByte))&0xF0) ==0xF0))
	{
		bool b = true;
		Write(b);
		WriteBits(input+currentByte, 4, true);
	}
	else
	{
		bool b = false;
		Write(b);
		WriteBits(input+currentByte, 8, true);
	}
}

// Read numberOfBitsToRead bits to the output source
// alignBitsToRight should be set to true to convert internal bitstream data to userdata
// It should be false if you used WriteBits with rightAlignedBits false
bool BitStream::ReadBits(unsigned char* output, int numberOfBitsToRead, bool alignBitsToRight)
{
	#ifdef _DEBUG
	assert(numberOfBitsToRead>0);
	#endif
//	if (numberOfBitsToRead<=0)
//		return false;

	if (readOffset+numberOfBitsToRead >numberOfBitsUsed)
		return false;

	int readOffsetMod8;

	int offset=0;
	memset(output, 0, BITS_TO_BYTES(numberOfBitsToRead));

	readOffsetMod8=readOffset%8;

//	do
	// Faster to put the while at the top surprisingly enough
	while(numberOfBitsToRead>0)
	{
			*(output + offset) |= *(data+(readOffset>>3)) << (readOffsetMod8); // First half

			if (readOffsetMod8 > 0 && numberOfBitsToRead > 8-(readOffsetMod8)) // If we have a second half, we didn't read enough bytes in the first half
				*(output + offset) |= *(data+(readOffset>>3) + 1) >> (8 - (readOffsetMod8)); // Second half (overlaps byte boundary)

			numberOfBitsToRead-= 8;
			if (numberOfBitsToRead<0) // Reading a partial byte for the last byte, shift right so the data is aligned on the right
			{
				if (alignBitsToRight)
					*(output + offset)>>=-numberOfBitsToRead;
				readOffset+=8+numberOfBitsToRead;
			}
			else
				readOffset+= 8;

			offset++;

	}
	//} while(numberOfBitsToRead>0);

	return true;
}

// Assume the input source points to a compressed native type.  Decompress and read it
bool BitStream::ReadCompressed(unsigned char* output, int size, bool unsignedData)
{
	int currentByte=(size>>3)-1;


	unsigned char byteMatch, halfByteMatch;
	if (unsignedData)
	{
	byteMatch=0;
	halfByteMatch=0;
	}
	else
	{
	byteMatch=0xFF;
	halfByteMatch=0xF0;
	}
	
	// Upper bytes are specified with a single 1 if they match byteMatch
	// From high byte to low byte, if high byte is a byteMatch then write a 1 bit.  Otherwise write a 0 bit and then write the remaining bytes
	while (currentByte>0)
	{
		// If we read a 1 then the data is byteMatch.

		bool b;
		if (Read(b)==false)
			return false;

		if (b) // Check that bit
		{
			output[currentByte]=byteMatch;
			currentByte--;
		}
		else
		{
			// Read the rest of the bytes
			if (ReadBits(output,(currentByte+1)<<3)==false)
				return false;

			return true;
		}
	}

	// All but the first bytes are byteMatch.  If the upper half of the last byte is a 0 (positive) or 16 (negative) then what we read will be a 1 and the remaining 4 bits.
	// Otherwise we read a 0 and the 8 bytes
	//assert(readOffset+1 <=numberOfBitsUsed); // If this assert is hit the stream wasn't long enough to read from
	if (readOffset+1 >numberOfBitsUsed)
		return false;
	bool b;
	if (Read(b)==false)
		return false;
	if (b) // Check that bit
	{
		if (ReadBits(output+currentByte,4)==false)
			return false;
		
		output[currentByte]|=halfByteMatch; // We have to set the high 4 bits since these are set to 0 by ReadBits
	}
	else
	{
		if (ReadBits(output+currentByte,8)==false)
			return false;
	}

	return true;
}

// Reallocates (if necessary) in preparation of writing numberOfBitsToWrite
void  BitStream::AddBitsAndReallocate(int numberOfBitsToWrite)
{
	if (numberOfBitsToWrite<=0)
		return;

	int newNumberOfBitsAllocated = numberOfBitsToWrite + numberOfBitsUsed;
	if ( numberOfBitsToWrite + numberOfBitsUsed>0 && ((numberOfBitsAllocated-1)>>3) < ((newNumberOfBitsAllocated-1)>>3)) // If we need to allocate 1 or more new bytes
	{
		#ifdef _DEBUG
		// If this assert hits then we need to specify true for the third parameter in the constructor
		// It needs to reallocate to hold all the data and can't do it unless we allocated to begin with
		assert(copyData==true);
		#endif

		// Less memory efficient but saves on news and deletes
		newNumberOfBitsAllocated=(numberOfBitsToWrite + numberOfBitsUsed) * 2;
		int newByteOffset = BITS_TO_BYTES(numberOfBitsAllocated);
		 // Use realloc and free so we are more efficient than delete and new for resizing
		data = (unsigned char*) realloc(data, BITS_TO_BYTES(newNumberOfBitsAllocated));
		#ifdef _DEBUG
		assert(data); // Make sure realloc succeeded
		#endif
		memset(data+newByteOffset, 0,  ((newNumberOfBitsAllocated-1)>>3) - ((numberOfBitsAllocated-1)>>3)); // Set the new data block to 0
	}

	if (newNumberOfBitsAllocated > numberOfBitsAllocated)
		numberOfBitsAllocated=newNumberOfBitsAllocated;
}

// Should hit if reads didn't match writes
void BitStream::AssertStreamEmpty(void)
{
	assert(readOffset ==numberOfBitsUsed);
}

void BitStream::PrintBits(void) const
{
	if (numberOfBitsUsed<=0)
	{
		printf("No bits\n");
		return;
	}

	for (int counter=0; counter < BITS_TO_BYTES(numberOfBitsUsed); counter++)
	{
		int stop;
		if (counter==(numberOfBitsUsed-1)>>3)
			stop=8-(((numberOfBitsUsed-1)%8)+1);
		else
			stop=0;

		for (int counter2=7; counter2>=stop; counter2--)
		{
			if ((data[counter]>>counter2) & 1)
				putchar('1');
			else
				putchar('0');
		}
		putchar(' ');
	}

	putchar('\n');
}


// Exposes the data for you to look at, like PrintBits does.
// Data will point to the stream.  Returns the length in bits of the stream.
int BitStream::CopyData(unsigned char**  _data) const
{
	#ifdef _DEBUG
	assert(numberOfBitsUsed>0);
	#endif
	*_data = new unsigned char [BITS_TO_BYTES(numberOfBitsUsed)];
    memcpy(*_data,data, sizeof(unsigned char) * (BITS_TO_BYTES(numberOfBitsUsed)));
	return numberOfBitsUsed;
}

// Ignore data we don't intend to read
void BitStream::IgnoreBits(int numberOfBits)
{
	readOffset+=numberOfBits;
}

// Move the write pointer to a position on the array.  Dangerous if you don't know what you are doing!
void BitStream::SetWriteOffset(int offset)
{
	numberOfBitsUsed=offset;
}

// Returns the length in bits of the stream
int BitStream::GetNumberOfBitsUsed(void) const
{
	return numberOfBitsUsed;
}

// Returns the length in bytes of the stream
int BitStream::GetNumberOfBytesUsed(void) const
{
	return BITS_TO_BYTES(numberOfBitsUsed);
}

// Returns the number of bits into the stream that we have read
int BitStream::GetReadOffset(void) const
{
	return  readOffset;
}

// Returns the number of bits left in the stream that haven't been read
int BitStream::GetNumberOfUnreadBits(void) const
{
	return numberOfBitsUsed - readOffset;
}

// Exposes the internal data
unsigned char* BitStream::GetData(void) const
{
	return data;
}

// If we used the constructor version with copy data off, this makes sure it is set to on and the data pointed to is copied.
void BitStream::AssertCopyData(void)
{
	if (copyData==false)
	{
		copyData=true;
		if (numberOfBitsAllocated>0)
		{
			unsigned char *newdata=(unsigned char*)malloc(BITS_TO_BYTES(numberOfBitsAllocated));
			#ifdef _DEBUG
			assert(data);
			#endif
			memcpy(newdata, data, BITS_TO_BYTES(numberOfBitsAllocated));
			data=newdata;
		}
		else
			data=0;
	}
}
