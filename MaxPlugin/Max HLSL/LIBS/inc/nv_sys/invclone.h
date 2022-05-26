#ifndef __INVCLONE_H
#define __INVCLONE_H

namespace nv_sys
{

//! A mixin interface which an object in the system might implement to allow itself to be cloned.
/*! Objects may want to allow themselves to be copied, and they do this by inheriting from INVClone.
*/
class INVClone : public INVObject
{
public:
	virtual unsigned long INTCALLTYPE AddRef() = 0;
	virtual unsigned long INTCALLTYPE Release() = 0;
	virtual bool INTCALLTYPE GetInterface(const NVGUID& name, void** ppObj) = 0;

	//! Call to create a clone of this object and return an INVObject pointer to the clone
	/*! If you wish to clone an object in the system, first try calling GetInterface to see if it
	supports INVClone.  If so, get the interface and call the clone method to make a copy.
	\return The newly created object
	\param Interface The requested interface from the clone
	\param ppObj Pointer to the returned object's pointer
	*/
	virtual bool INTCALLTYPE Clone(const NVGUID& Interface, void** ppObj) const = 0;
};

}; // namespace nv_sys

//! The unique interface ID for INVClone
// {7A595838-7082-4674-9AB7-5D224A28A767}
static const nv_sys::NVGUID IID_INVClone = 
{ 0x7a595838, 0x7082, 0x4674, { 0x9a, 0xb7, 0x5d, 0x22, 0x4a, 0x28, 0xa7, 0x67 } };

#endif __INVCLONE