#ifndef __CHECKSUM_H
#define __CHECKSUM_H

// From http://www.flounder.com/checksum.htm

class CheckSum {
public:
	CheckSum() { clear(); }
	void clear() { sum = 0; r = 55665; c1 = 52845; c2 = 22719;}
	void add(unsigned long w);
	void add(unsigned int w) { add((unsigned long)w); }
	void add(unsigned short w);
	void add(unsigned char* b, unsigned int length);
	void add(unsigned char b);
	unsigned long get() { return sum; }
protected:
	unsigned short r;
	unsigned short c1;
	unsigned short c2;
	unsigned long sum;
};

#endif
