#ifndef __RAW_PACKET_STRUCT_H
#define __RAW_PACKET_STRUCT_H

struct rawPacketStruct
{
	char buffer[512];
	char ip[22];
	unsigned short port;
	int length;

#ifdef _INTERNET_SIMULATOR
	unsigned long sendTime;
#endif
};

#endif