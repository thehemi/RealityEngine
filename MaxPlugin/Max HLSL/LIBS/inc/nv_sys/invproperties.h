#ifndef __INVPROPERTIES_H
#define __INVPROPERTIES_H

namespace nv_sys
{

class INVProperties : public INVObject
{
public:
	virtual unsigned long INTCALLTYPE AddRef() = 0;
	virtual unsigned long INTCALLTYPE Release() = 0;
	virtual bool INTCALLTYPE GetInterface(const NVGUID& name, void** ppObj) = 0;

	virtual INVObject* INTCALLTYPE GetProperties() const = 0;
};

}; // namespace nv_sys

// {4F5DF0C9-1D13-44f1-B98A-6265EFB670C3}
static const nv_sys::NVGUID IID_INVProperties = 
{ 0x4f5df0c9, 0x1d13, 0x44f1, { 0xb9, 0x8a, 0x62, 0x65, 0xef, 0xb6, 0x70, 0xc3 } };

#endif __INVPROPERTIES