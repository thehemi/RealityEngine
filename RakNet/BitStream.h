// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#ifndef  __BITSTREAM_H
#define __BITSTREAM_H

namespace RakNet
{

#define BITS_TO_BYTES(x) (((x)+7)>>3)

class BitStream
{
public:
	BitStream();
	BitStream(int initialBytesToAllocate);
	// Set _copyData to true if you want to make an internal copy of the data you are passing.  You can then Write and do all other operations
	// Set it to false if you want to just use a pointer to the data you are passing, in order to save memory and speed.  You should only then do read operations
	BitStream(const char* _data, unsigned int lengthInBytes, bool _copyData);
	~BitStream();

	// Reset the bitstream for reuse
	void Reset(void);

	// Write the native types to the end of the buffer
	void Write(bool input);
	void Write(unsigned char  input);
	void Write(char input);
	void Write(unsigned short input);
	void Write(short input);
	void Write(unsigned int input);
	void Write(int input);
	void Write(unsigned long input);
	void Write(long input);
	void Write(float input);
	void Write(double input);
	void Write(char* input, int numberOfBytes); // Write an array or casted stream

	// Write the native types with simple compression.
	// Best used with  negatives and positives close to 0
	void WriteCompressed(unsigned char  input);
	void WriteCompressed(char input);
	void WriteCompressed(unsigned short input);
	void WriteCompressed(short input);
	void WriteCompressed(unsigned int input);
	void WriteCompressed(int input);
	void WriteCompressed(unsigned long input);
	void WriteCompressed(long input);
	void WriteCompressed(float input);
	void WriteCompressed(double input);

	// Read the native types from the front of the buffer
	bool Read(bool &output);
	bool Read(unsigned char  &output);
	bool Read(char &output);
	bool Read(unsigned short &output);
	bool Read(short &output);
	bool Read(unsigned int &output);
	bool Read(int &output);
	bool Read(unsigned long &output);
	bool Read(long &output);
	bool Read(float &output);
	bool Read(double &output);
	bool Read(char* output, int numberOfBytes);	// Read an array or casted stream

	// Read the types you wrote with WriteCompressed
	// Returns true on success, false on not enough data to read
	bool ReadCompressed(unsigned char & output);
	bool ReadCompressed(char &output);
	bool ReadCompressed(unsigned short &output);
	bool ReadCompressed(short &output);
	bool ReadCompressed(unsigned int &output);
	bool ReadCompressed(int &output);
	bool ReadCompressed(unsigned long &output);
	bool ReadCompressed(long& output);
	bool ReadCompressed(float &output);
	bool ReadCompressed(double &output);

	// Sets the read pointer back to the beginning of your data.
	void ResetReadPointer(void);

	// This is good to call when you are done with the stream to make sure you didn't leave any data left over
	void AssertStreamEmpty(void);
	void PrintBits(void) const;

	// Ignore data we don't intend to read
	void IgnoreBits(int numberOfBits);

	// Move the write pointer to a position on the array.  Dangerous if you don't know what you are doing!
	void SetWriteOffset(int offset);

	// Returns the length in bits of the stream
	int GetNumberOfBitsUsed(void) const;

	// Returns the length in bytes of the stream
	int GetNumberOfBytesUsed(void) const;

	// Returns the number of bits into the stream that we have read
	int GetReadOffset(void) const;

	// Returns the number of bits left in the stream that haven't been read
	int GetNumberOfUnreadBits(void) const;

	// Makes a copy of the internal data for you
	// Data will point to the stream.  Returns the length in bits of the stream
	// Partial bytes are left aligned
	int CopyData(unsigned char**  _data) const;

	// Set the stream to some initial data.  For internal use
	// Partial bytes are left aligned
	void SetData(const unsigned char* input, int numberOfBits);

	// Exposes the internal data
	// Partial bytes are left aligned
	unsigned char* GetData(void) const;

	// Write numberToWrite bits from the input source
	// Right aligned data means in the case of a partial byte, the bits are aligned from the right (bit 0) rather than the left (as in the normal internal representation)
	// You would set this to true when writing user data, and false when copying bitstream data, such as writing one bitstream to another
	void WriteBits(unsigned char* input, int numberOfBitsToWrite, bool rightAlignedBits=true);

	// Align the bitstream to the byte boundary and then write the specified number of bits.
	// This is faster than WriteBits but wastes the bits to do the alignment and requires you to call
	// ReadAlignedBits at the corresponding read position.  
	void WriteAlignedBytes(unsigned char* input, int numberOfBytesToWrite);

	// Read bits, starting at the next aligned bits. Note that the modulus 8 starting offset of the
	// sequence must be the same as was used with WriteBits. This will be a problem with packet coalescence
	// unless you byte align the coalesced packets.
	bool ReadAlignedBytes(unsigned char* output, int numberOfBytesToRead);

	// Align the next write and/or read to a byte boundary.  This can be used to 'waste' bits to byte align for efficiency reasons
	// It can also be used to force coalesced bitstreams to start on byte boundaries so 
	// so WriteAlignedBits and ReadAlignedBits both calculate the same offset when aligning.
	void AlignWriteToByteBoundary(void);
	void AlignReadToByteBoundary(void);

	// Read numberOfBitsToRead bits to the output source
	// alignBitsToRight should be set to true to convert internal bitstream data to userdata
	// It should be false if you used WriteBits with rightAlignedBits false
	bool ReadBits(unsigned char* output, int numberOfBitsToRead, bool alignBitsToRight=true); 

	// --- Low level functions ---
	// These are for when you want to deal with bits and don't care about type checking
	void Write0(void); // Write a 0
	void Write1(void);  // Write a 1
	bool ReadBit(void); // Reads 1 bit and returns true if that bit is 1 and false if it is 0

	// If we used the constructor version with copy data off, this makes sure it is set to on and the data pointed to is copied.
	void AssertCopyData(void);

	// Use this if you pass a pointer copy to the constructor (_copyData==false) and want to overallocate to prevent reallocation
	void SetNumberOfBitsAllocated(unsigned int lengthInBits);

private:
	void WriteCompressed(unsigned char* input, int size, bool unsignedData); // Assume the input source points to a native type, compress and write it
   
	
	bool ReadCompressed(unsigned char* output, int size, bool unsignedData); // Assume the input source points to a compressed native type.  Decompress and read it
	
	void AddBitsAndReallocate(int numberOfBitsToWrite);  // Reallocates (if necessary) in preparation of writing numberOfBitsToWrite

	int numberOfBitsUsed, numberOfBitsAllocated, readOffset;
	unsigned char *data;
	bool copyData;
};

}

#endif
