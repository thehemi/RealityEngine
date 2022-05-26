// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#ifndef __DISTRIBUTED_CLASS_STUB_H
#define __DISTRIBUTED_CLASS_STUB_H

#include "EncodeClassName.h"

class DistributedNetworkObject;
class DistributedNetworkObjectBaseStub
{
public:
	char *GetEncodedClassName(void) const;
	virtual DistributedNetworkObject *GetObject()=0;
protected:
	void RegisterStub(char *className);
	char encodedClassName[MAXIMUM_CLASS_IDENTIFIER_LENGTH];
};

template <class T>
class DistributedNetworkObjectStub : public DistributedNetworkObjectBaseStub
{
public:
	DistributedNetworkObjectStub(char* className) {RegisterStub(className);};
	DistributedNetworkObject *GetObject()
	{
		return new T;
	};
protected:
};


#endif

