/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_gui
File:  invguidata.h

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

  This is the interface to the control panel gui items.
  You implement this interface and pass it to the panel, which draws the gui items
  requested.
  The class nvNVGUI_Item is a container for a GUI item, and is synchronised by the panel
  when the gui changes.
  The strategy for this method is very similar to the nv_connections implementation:
  an interface single-inheritance hierarchy is implemented by an inherited version of a
  base class.  To get single-inheritance, we use a template when creating each concrete clas.
  (the concrete classes are in nvguidata.h

******************************************************************************/

#ifndef __INVGUIDATA_H
#define __INVGUIDATA_H

#include "nv_math\nv_math.h"

namespace nv_gui
{

typedef enum GUITYPE
{
	GUITYPE_BRANCH = 0,
	GUITYPE_SLIDER = 1,
	GUITYPE_COLOR = 2,
	GUITYPE_BOOL = 3,
	GUITYPE_EDITBOX = 4,
	GUITYPE_LISTBOX = 5,
	GUITYPE_FLOAT = 6,
	GUITYPE_VECTOR2 = 7,
	GUITYPE_VECTOR3 = 8,
	GUITYPE_VECTOR4 = 9,
	GUITYPE_MATRIX = 10,
	GUITYPE_RESERVED = 11,
	GUITYPE_INT = 12,
	GUITYPE_FILEPATH = 13,
	GUITYPE_TEXT = 14,
	GUITYPE_DWORD = 15,
	GUITYPE_UNKNOWN = 0x7FFFFFFF
} GUITYPE;

typedef std::list<std::string> tListItems;

class INVGUIItem : public nv_sys::INVObject
{
public:
    typedef enum {
        kUnset          = 0x0000001,
        kReadOnly       = 0x0000002
    } ItemState;

	// Info on the gui item.
	virtual const char* INTCALLTYPE     GetName() const = 0;
	virtual const char* INTCALLTYPE     GetInfoText() const = 0;
	virtual bool INTCALLTYPE            SetInfoText(const char* pszText) = 0;
	virtual GUITYPE INTCALLTYPE         GetType() const = 0;

	// For storing client-specific data.
	virtual void* INTCALLTYPE           GetClientData() const = 0;
	virtual void INTCALLTYPE            SetClientData(void* pClientData) = 0;

	// Private interfaces used by the gui control
	// !! DO NOT CALL DIRECTLY !!
	virtual unsigned int INTCALLTYPE    GetNumChildren() const = 0;
	virtual INVGUIItem* INTCALLTYPE     GetChild(unsigned int ChildNum) const = 0;
	virtual bool INTCALLTYPE            AddChild(INVGUIItem* pChild) = 0;
	virtual bool INTCALLTYPE            SetParent(INVGUIItem* pParent) = 0;
	virtual INVGUIItem* INTCALLTYPE     GetParent() = 0;
	virtual bool INTCALLTYPE            RemoveChild(INVGUIItem* pChild) = 0;
	virtual bool INTCALLTYPE            RemoveChildren() = 0;

	virtual void INTCALLTYPE            HasImage(bool bImage, unsigned int idx ) = 0;
    virtual unsigned int INTCALLTYPE    GetImageIdx() = 0;
    virtual bool INTCALLTYPE            IsImage() = 0;

	virtual void INTCALLTYPE            SetState(ItemState state, bool on) = 0;
    virtual void INTCALLTYPE            SetState(DWORD state) = 0;
    virtual DWORD INTCALLTYPE           GetState() = 0;
    virtual bool INTCALLTYPE            IsState(ItemState state) = 0;
};

class INVGUIItem_Text : public INVGUIItem
{
public:
	virtual const char* INTCALLTYPE     GetString() const = 0;
	virtual void INTCALLTYPE            SetString(const char* pszText) = 0;
};

class INVGUIItem_Branch : public INVGUIItem
{
public:
};

class INVGUIItem_Slider : public INVGUIItem
{
public:
	virtual unsigned int INTCALLTYPE GetPos() const = 0;
	virtual void INTCALLTYPE SetPos(unsigned int Value) = 0;
	virtual void INTCALLTYPE GetMinMaxStep(unsigned int& Min, unsigned int& Max, unsigned int& Step) const = 0;
	virtual void INTCALLTYPE SetMinMaxStep(unsigned int Min, unsigned int Max, unsigned int Step) = 0;
};

class INVGUIItem_ListBox : public INVGUIItem
{
public:
	virtual int INTCALLTYPE GetIndex() const = 0; // -1 is none selected
	virtual void INTCALLTYPE SetIndex(int Index) = 0;
	virtual unsigned int INTCALLTYPE GetNumStrings() const = 0;
	virtual const char* INTCALLTYPE GetString(unsigned int Num) const = 0;
	virtual void INTCALLTYPE SetPrivateData(unsigned int Index, void* pData) = 0;
	virtual void* INTCALLTYPE GetPrivateData(const unsigned int Index) const = 0;
	virtual void INTCALLTYPE Clear() = 0;
	virtual void INTCALLTYPE AddString(const char* pszString) = 0;
};

class INVGUIItem_Color : public INVGUIItem
{
public:
	virtual const vec4& INTCALLTYPE GetColor() const = 0;
	virtual void INTCALLTYPE SetColor(const vec4& color) = 0;
	virtual bool INTCALLTYPE HasAlpha() const = 0;
	virtual void INTCALLTYPE SetAlpha(bool bAlpha) = 0;
};

class INVGUIItem_EditBox : public INVGUIItem
{
public:
	virtual const char* INTCALLTYPE GetString() const = 0;
	virtual void INTCALLTYPE SetString(const char* pszString) = 0;
};

class INVGUIItem_FilePath : public INVGUIItem
{
public:
	virtual const char* INTCALLTYPE GetPath() const = 0;
	virtual void INTCALLTYPE SetPath(const char* pszString) = 0;
	virtual bool INTCALLTYPE IsTruncatedDisplay() const = 0;
	virtual void INTCALLTYPE SetTruncatedDisplay(bool bTruncated) = 0;
	virtual const char* INTCALLTYPE GetExtensionString() const = 0;
	virtual void INTCALLTYPE SetExtensionString(const char* pszExtension) = 0;
    virtual bool INTCALLTYPE IsPreview() const = 0;
    virtual void INTCALLTYPE SetPreview(bool bPreview) = 0;
};

class INVGUIItem_Float : public INVGUIItem
{
public:
	virtual const float INTCALLTYPE GetFloat() const = 0;
	virtual void INTCALLTYPE SetFloat(const float fVal) = 0;
	virtual void INTCALLTYPE GetMinMaxStep(float& Min, float& Max, float&Step) const = 0;
	virtual void INTCALLTYPE SetMinMaxStep(float Min, float Max,float Step) = 0;
};

class INVGUIItem_Dword : public INVGUIItem
{
public:
	virtual DWORD INTCALLTYPE GetDword() const = 0;
	virtual void INTCALLTYPE SetDword(DWORD dwValue) = 0;
	virtual void INTCALLTYPE GetMinMax(DWORD& Min, DWORD& Max) const = 0;
	virtual void INTCALLTYPE SetMinMax(DWORD Min, DWORD Max) = 0;
};

class INVGUIItem_Int : public INVGUIItem
{
public:
	virtual int INTCALLTYPE GetInt() const = 0;
	virtual void INTCALLTYPE SetInt(int Value) = 0;
	virtual void INTCALLTYPE GetMinMax(int& Min, int& Max) const = 0;
	virtual void INTCALLTYPE SetMinMax(int Min, int Max) = 0;
};

class INVGUIItem_Bool : public INVGUIItem
{
public:
	virtual bool INTCALLTYPE GetBool() const = 0;
	virtual void INTCALLTYPE SetBool(const bool bVal) = 0;
};


class INVGUIItem_Matrix : public INVGUIItem
{
public:
	virtual const float* INTCALLTYPE GetArray() const = 0;
	virtual void INTCALLTYPE SetArray(const float* pArray) = 0;
	virtual unsigned int INTCALLTYPE GetRows() const = 0;	
	virtual unsigned int INTCALLTYPE GetColumns() const = 0;
};

class INVGUIItem_Vector2 : public INVGUIItem
{
public:
	virtual const vec2& INTCALLTYPE GetVector() const = 0;
	virtual void INTCALLTYPE SetVector(const vec2& mat) = 0;
};

class INVGUIItem_Vector3 : public INVGUIItem
{
public:
	virtual const vec3& INTCALLTYPE GetVector() const = 0;
	virtual void INTCALLTYPE SetVector(const vec3& mat) = 0;
};

class INVGUIItem_Vector4 : public INVGUIItem
{
public:
	virtual const vec4& INTCALLTYPE GetVector() const = 0;
	virtual void INTCALLTYPE SetVector(const vec4& mat) = 0;
};

class INVGUIData : public nv_sys::INVObject
{
public:
	virtual unsigned int INTCALLTYPE GetNumItems() = 0;
	virtual INVGUIItem* INTCALLTYPE GetItem(unsigned int Item) = 0;
	virtual bool INTCALLTYPE AddItem(INVGUIItem* pItem) = 0;
	virtual bool INTCALLTYPE RemoveItem(INVGUIItem* pItem) = 0;
};

}; // namespace nv_gui

#endif // __INVGUIData_H
