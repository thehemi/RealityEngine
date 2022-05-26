#ifndef __INVOBJECT_H
#define __INVOBJECT_H

//! The nv_sys namespace is the core namespace used by system components.
namespace nv_sys
{

//! A unique ID for an interface/object in the system. 
typedef struct NVGUID 
{   
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
	unsigned char Data4[8];
} NVGUID;

//! A less than operator, which enables NVGUID's to be stored in std::maps, etc.
/*! The stl containers use the less than operator to sort items */
static bool operator < (const NVGUID& lhs, const NVGUID& rhs)
{
	if (lhs.Data1 < rhs.Data1)
		return true;
	else if (lhs.Data1 > rhs.Data1)
		return false;

	if (lhs.Data2 < rhs.Data2)
		return true;
	else if (lhs.Data2 > rhs.Data2)
		return false;

	if (lhs.Data3 < rhs.Data3)
		return true;
	else if (lhs.Data3 > rhs.Data3)
		return false;

	if (memcmp(&lhs.Data4[0], &rhs.Data4[0], sizeof(char) * 8) < 0)
		return true;

	return false;
}

//! A comparison function to check that 2 NVGUID ID's match 
/*!
\sa GetInterface
*/
static bool EqualNVGUID(const NVGUID& lhs, const NVGUID& rhs)
{
	if (lhs.Data1 != rhs.Data1)
		return false;
	if (lhs.Data2 != rhs.Data2)
		return false;
	if (lhs.Data3 != rhs.Data3)
		return false;

	if (memcmp(&lhs.Data4[0], &rhs.Data4[0], sizeof(char) * 8) != 0)
		return false;

	return true;
}

//! This is base interface which most objects derive from.
/*! The INVObject base class supports simply increasing/decreasing the reference count of an object, and GetInterface which
enables you to request the object cast itself to another interface it supports.
*/
class INVObject
{
public:
	//! Increase the reference on the object
	/*!
	\return The reference count after the increase
	\sa Release
	*/
	virtual unsigned long INTCALLTYPE AddRef() = 0;

	//! Decrease the reference on the object
	/*! When the last reference is gone (i.e. the reference count returned is 0, the object will call delete on itself)
	\return The reference count after the increase
	\sa AddRef
	*/
	virtual unsigned long INTCALLTYPE Release() = 0;

	//! Get another interface from this object
	/*! Will return an interface if this object supports it.
	\param guid A unique ID for the interface requested
	\param ppObj A pointer to an interface pointer.
	\return TRUE if the interface is succesfully retrieved.
	*/
	virtual bool INTCALLTYPE GetInterface(const NVGUID& guid, void** ppObj) = 0;

};

}; // namespace nv_sys

//! A unique ID for the INVObject interface.
// {B61F5586-2095-4364-9AB9-581EBFEE130B}
static const nv_sys::NVGUID IID_INVObject = 
{ 0xb61f5586, 0x2095, 0x4364, { 0x9a, 0xb9, 0x58, 0x1e, 0xbf, 0xee, 0x13, 0xb } };

#endif // __INVOBJECT
