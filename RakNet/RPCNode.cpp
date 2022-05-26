#include "RPCNode.h"
#include <assert.h>
#include <string.h>

RPCNode::RPCNode()
{
	uniqueIdentifier=0;
	functionName=0;
}

RPCNode::RPCNode(char* uniqueID, void (*_functionName)(char *input, int numberOfBitsOfData, PlayerID sender))
{
#ifdef _DEBUG
	assert(uniqueID);
	assert(uniqueID[0]);
#endif
	uniqueIdentifier = new char [strlen(uniqueID)+1];
	strcpy(uniqueIdentifier, uniqueID);
	functionName=_functionName;
}

RPCNode::RPCNode(const RPCNode& input)
{
	if (input.uniqueIdentifier!=0)
	{
		uniqueIdentifier = new char [strlen(input.uniqueIdentifier)+1];
		strcpy(uniqueIdentifier, input.uniqueIdentifier);
	}
	else
	{
		uniqueIdentifier=0;
	}

	functionName=input.functionName;
}

RPCNode& RPCNode::operator = (const RPCNode& input)
{
	if (&input == this)
		return *this;

	if (input.uniqueIdentifier!=0)
	{
		if (uniqueIdentifier!=0)
			delete [] uniqueIdentifier;
		uniqueIdentifier = new char [strlen(input.uniqueIdentifier)+1];
		strcpy(uniqueIdentifier, input.uniqueIdentifier);
	}
	else
	{
		delete uniqueIdentifier;
		uniqueIdentifier=0;
	}

	functionName=input.functionName;

	return *this;
}


RPCNode::~RPCNode()
{
	delete [] uniqueIdentifier;
}

int operator==(const RPCNode& left, const RPCNode& right)
{
	if (strcmp(left.uniqueIdentifier, right.uniqueIdentifier)==0) return !0;
	return 0;
}

int operator>(const RPCNode& left, const RPCNode& right)
{
	if (strcmp(left.uniqueIdentifier, right.uniqueIdentifier)>0) return !0;
	return 0;
}
int operator<(const RPCNode& left, const RPCNode& right)
{
	if (strcmp(left.uniqueIdentifier, right.uniqueIdentifier)<0) return !0;
	return 0;
}

