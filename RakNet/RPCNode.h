#ifndef __RPC_NODE
#define __RPC_NODE

#include "NetworkTypes.h"

// RPC stuff.  Ignore this
struct RPCNode
{
	char *uniqueIdentifier;
	void (*functionName)(char *input, int numberOfBitsOfData, PlayerID sender);
	RPCNode(char* uniqueID, void (*_functionName)(char *input, int numberOfBitsOfData, PlayerID sender));
	RPCNode& operator = (const RPCNode& input);
	RPCNode(const RPCNode& input);

	RPCNode();
	~RPCNode();

	friend int operator==(const RPCNode& left, const RPCNode& right);
	friend int operator > (const RPCNode& left, const RPCNode& right);
	friend int operator < (const RPCNode& left, const RPCNode& right);
};

#endif

