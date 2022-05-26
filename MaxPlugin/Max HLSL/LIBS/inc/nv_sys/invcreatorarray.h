#ifndef __INVCREATORARRAY_H
#define __INVCREATORARRAY_H

#include "invobject.h"

namespace nv_sys
{

//! An collection of creators, returned by INVSystem.
/* Use this interface to enumerate object creators, such as when creation creators in certain categories.
For example, creating a list of rendering device plugin options.
\sa INVCreator, INVSystem
*/
class INVCreatorArray : public INVObject
{
public:
	//!Add a creator to the array.
	/* 
	\return TRUE if succesfull.
	\param pCreator A pointer to the creator class.
	*/
	virtual bool INTCALLTYPE AddCreator(INVCreator* pCreator) = 0;

	//!Get a creator from the array.
	/* 
	\return A pointer to the creator if succesfull, or NULL if out of range.
	\param Index The index of the creator
	*/
	virtual INVCreator* INTCALLTYPE GetCreator(unsigned int Index) = 0;

	//!Get the number of creators in the array
	/* 
	\return The number of creators in the array
	*/
	virtual unsigned int INTCALLTYPE GetNumCreators() = 0;
};

}; // namespace nv_sys

//! The unique interface ID for INVCreatorArray
// {0059FC11-34D4-4349-94A5-8722D6E11DF0}
static const nv_sys::NVGUID IID_INVCreatorArray = 
{ 0x59fc11, 0x34d4, 0x4349, { 0x94, 0xa5, 0x87, 0x22, 0xd6, 0xe1, 0x1d, 0xf0 } };

#endif __INVCREATORARRAY_H