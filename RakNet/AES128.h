/*
	128-bit Advanced Encryption Standard (Rjindael)

	catid(cat02e@fsu.edu)

	7/18/2004	Moved from old code
	9/5/2004	Created

	Tabs: 4 spaces
	Dist: public
*/

#ifndef AES_H
#define AES_H

//////// AES128 ////////

class AES128
{
	unsigned char key_schedule[11][16];

protected:
	unsigned char GF2M(unsigned char k, unsigned char b);
	void AddRoundKey(unsigned char *m, unsigned char *rk);
	void ShiftRows(unsigned char *m);
	void iShiftRows(unsigned char *m);
	void Substitution(unsigned char *m);
	void iSubstitution(unsigned char *m);
	void MixColumns(unsigned char *m);
	void iMixColumns(unsigned char *m);
	unsigned int RolSubByte(unsigned int n);
public:
	void set_key(const unsigned char in_key[16]);
	void encrypt16(unsigned char buffer[16]);
	void decrypt16(unsigned char buffer[16]);
};

#endif // AES_H
