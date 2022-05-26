// This file is part of RakNet Copyright 2003, 2004 Rakkarsoft LLC and Kevin Jenkins.

// Usage of Raknet is subject to the appropriate licence agreement.
// "Shareware" Licensees with Rakkarsoft LLC are subject to the shareware license found at http://www.rakkarsoft.com/shareWareLicense.html which you agreed to upon purchase of a "Shareware license"
// "Commercial" Licensees with Rakkarsoft LLC are subject to the commercial license found at http://www.rakkarsoft.com/sourceCodeLicense.html which you agreed to upon purchase of a "Commercial license"
// All other users are subject to the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

// Refer to the appropriate license agreement for distribution, modification, and warranty rights.

#if !defined(AFX_NETWORKOBJECT_H__29266376_3F9E_42CD_B208_B58957E1935B__INCLUDED_)
#define      AFX_NETWORKOBJECT_H__29266376_3F9E_42CD_B208_B58957E1935B__INCLUDED_

#include "BinarySearchTree.h"
#include "NetworkTypes.h"

class NetworkObject;

// Used internally to contain objects in the tree.  Ignore this
struct ObjectIDNode
{
	unsigned short objectID;
	NetworkObject *object;

	ObjectIDNode();
	ObjectIDNode(unsigned short ObjectID, NetworkObject *Object);

	friend int operator==(const ObjectIDNode& left, const ObjectIDNode& right);
	friend int operator > (const ObjectIDNode& left, const ObjectIDNode& right);
	friend int operator < (const ObjectIDNode& left, const ObjectIDNode& right);
};

class NetworkObject
{
public:
	NetworkObject();
	virtual ~NetworkObject();

	virtual unsigned short GetID(void) const;
	virtual void SetID(unsigned short id); // Only the server code should call this

	static BasicDataStructures::AVLBalancedBinarySearchTree<ObjectIDNode> IDTree;
	// These two functions are only meant to be used when saving and loading games as you should save the HIGHEST value staticItemID has achieved
	// upon save and reload it upon load.  Save AFTER you've created all the items derived from this class you are going to create.  Load
	// BEFORE you create any new objects that are not SetIDed based on the save data
	static unsigned short GetStaticItemID(void);
	static void SetStaticItemID(unsigned short i);

protected:
	unsigned short objectID;  // The network ID of this object
	bool serverAssignedID;

private:
	static unsigned short staticItemID;
};


NetworkObject* GET_OBJECT_FROM_ID(ObjectID x);

#endif // !defined(AFX_NETWORKOBJECT_H__29266376_3F9E_42CD_B208_B58957E1935B__INCLUDED_)
