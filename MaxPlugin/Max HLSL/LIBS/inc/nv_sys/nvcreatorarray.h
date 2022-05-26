#ifndef __NVCREATORARRAY_H
#define __NVCREATORARRAY_H

namespace nv_sys
{

class NVCreatorArray : public INVCreatorArray
{
	typedef std::vector<INVCreator*> tvecCreator;
	NVCreatorArray();
public:
	
	static bool INTCALLTYPE CreateNVObject(const INVCreator* pCreator, const NVGUID& InterfaceID, void** ppObj);

	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const NVGUID& guid, void** ppObj);

	virtual bool INTCALLTYPE AddCreator(INVCreator* pCreator);
	virtual INVCreator* INTCALLTYPE GetCreator(unsigned int Index);
	virtual unsigned int INTCALLTYPE GetNumCreators();
private:
	tvecCreator m_vecCreator;
	unsigned long m_dwRefCount;
};

}; // namespace nv_sys

#endif __NVCREATORARRAY_H