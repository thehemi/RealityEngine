#ifndef __DATA_BLOCK_ENCRYPTOR_H
#define __DATA_BLOCK_ENCRYPTOR_H

#include "AES128.h"

class DataBlockEncryptor
{
public:
	DataBlockEncryptor();
	~DataBlockEncryptor();
	bool IsKeySet(void) const;

	void SetKey(const unsigned char key[16]);

	void UnsetKey(void);

	// Encrypt adds up to 15 bytes.  Output should be large enough to hold this.
	// Output can be the same memory block as input
	void Encrypt(unsigned char *input, int inputLength, unsigned char *output, int *outputLength);

	// Decrypt removes bytes, as few as 6.  Output should be large enough to hold this.
	// Returns false on bad checksum or input, true on success
	// Output can be the same memory block as input
	bool Decrypt(unsigned char *input, int inputLength, unsigned char *output, int *outputLength);
protected:
	AES128 secretKeyAES128;
	bool keySet;
};

#endif
