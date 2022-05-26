/*********************************************************************NVMH4****
Path:  NVSDK\Common\src\nv_fx
File:  nvrenderdevice_fx.cpp

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:

******************************************************************************/
#ifndef __NVOBJECTSEMANTICS_H
#define __NVOBJECTSEMANTICS_H

namespace nv_sys
{

typedef std::map<std::string, tAnnotationInfo> tmapNameToAnnotationInfo;

// A basic generic type holder.
// Assigning
class NVObjectSemantics : public INVObjectSemantics
{
public:
	NVObjectSemantics();

	static bool INTCALLTYPE CreateNVObject(const nv_sys::INVCreator* pCreator, const NVGUID& InterfaceID, void** ppObj);

	// INVObject
	virtual unsigned long INTCALLTYPE AddRef();
	virtual unsigned long INTCALLTYPE Release();
	virtual bool INTCALLTYPE GetInterface(const NVGUID& guid, void** ppObj);

	// INVObjectSemantics
	virtual bool INTCALLTYPE SetSemanticID(eSEMANTICID SemanticID);
	virtual bool INTCALLTYPE SetName(const char* pszName);
	virtual eSEMANTICID INTCALLTYPE GetSemanticID() const;
	virtual const char* INTCALLTYPE GetName() const;
	virtual const tAnnotationInfo* INTCALLTYPE FindAnnotationInfo(eANNOTATIONNAMEID NameID) const;
	virtual eANNOTATIONVALUEID INTCALLTYPE FindAnnotationValue(eANNOTATIONNAMEID NameID) const;
	virtual bool INTCALLTYPE AddAnnotation(eANNOTATIONNAMEID NameID, eANNOTATIONVALUEID ValueID);
	virtual bool INTCALLTYPE AddAnnotation(eANNOTATIONNAMEID NameID, const nv_sys::NVType& Value);
	virtual bool INTCALLTYPE AddAnnotation(const char* pszName, const nv_sys::NVType& Value);
	virtual unsigned int INTCALLTYPE GetNumAnnotations() const;
	virtual const tAnnotationInfo* INTCALLTYPE GetAnnotation(unsigned int Num) const;

	// INVClone
	virtual bool INTCALLTYPE Clone(const NVGUID& Interface, void** ppObj) const;

	// INVProperties
	virtual INVObject* INTCALLTYPE GetProperties() const { return NULL; }

private:
	std::string m_strName;
	eSEMANTICID m_SemanticID;
	tmapNameToAnnotationInfo m_mapNameToAnnotationInfo;
	unsigned long m_dwRefCount;
};

}; // namespace nv_sys

#endif
