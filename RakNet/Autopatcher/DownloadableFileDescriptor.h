#ifndef __DOWNLOADABLE_FILE_DESCRIPTOR
#define __DOWNLOADABLE_FILE_DESCRIPTOR

#include "BitStream.h"
#include "SHA1.h"
using namespace RakNet;

struct DownloadableFileDescriptor
{
	DownloadableFileDescriptor();
	~DownloadableFileDescriptor();
	void Clear(void);
	void SerializeHeader(BitStream *out);
	void SerializeSHA1(BitStream *out);
	void SerializeFileData(BitStream *out);
	bool DeserializeHeader(BitStream *in);
	bool DeserializeSHA1(BitStream *in);
	bool DeserializeFileData(BitStream *in);

	char *filename;
	unsigned fileLength;
	bool fileDataIsCompressed;
	unsigned compressedFileLength;
	char SHA1Code[SHA1_LENGTH];

	char *fileData;
	
};

#endif
