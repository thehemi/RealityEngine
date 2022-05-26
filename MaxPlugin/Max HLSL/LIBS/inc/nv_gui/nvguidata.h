/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_gui
File:  nvguidata.h

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
  base class.  To get single-inheritance, we use a template parameter when creating 
  each concrete clas.

******************************************************************************/

#ifndef __NVGUIDATA_H
#define __NVGUIDATA_H

#include "invguidata.h"
#include <algorithm>
#include <vector>
#include <map>

namespace nv_gui
{

typedef std::vector<INVGUIItem*> tvecGUIItems;
typedef std::vector<float> tvecFloatItems;
class NVGUIData : public INVGUIData
{
public:
	NVGUIData()
		: m_dwRefCount(1)
	{}
	virtual ~NVGUIData() 
	{
		tvecGUIItems::iterator itrChildren = m_Children.begin();
		while (itrChildren != m_Children.end())
		{
			SAFE_RELEASE(*itrChildren);
			itrChildren++;
		}
	};
	virtual unsigned long INTCALLTYPE AddRef() { m_dwRefCount++; return m_dwRefCount; }	\
	virtual unsigned long INTCALLTYPE Release() { DWORD dwRefNew = --m_dwRefCount; if (dwRefNew == 0) delete this; return dwRefNew; }	\
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& Interface, void** ppvObj) { return false; }	\
	virtual unsigned int INTCALLTYPE GetNumItems() { return (unsigned int)m_Children.size(); }
	virtual INVGUIItem* INTCALLTYPE GetItem(unsigned int Item) { if (m_Children.size() > Item) return m_Children[Item]; else { assert(0); return NULL; }}
	virtual bool INTCALLTYPE AddItem(INVGUIItem* pItem) 
	{
		if (!pItem)
			return false;

		SAFE_ADDREF(pItem);
		m_Children.push_back(pItem); 
		return true; 
	}
	virtual bool INTCALLTYPE RemoveItem(INVGUIItem* pItem)
	{
		if (!pItem)
			return false;
		m_Children.erase(std::remove(m_Children.begin(), m_Children.end(), pItem), m_Children.end()); 
		SAFE_RELEASE(pItem);
		return true;
	}
private:
	DWORD m_dwRefCount;
	tvecGUIItems m_Children;
};

template <class I>
class NVGUIItem : virtual public I
{
public:
	enum TreeItemStates
	{
		TreeItemSelected =		0x00000001,
		TreeItemExpanded =		0x00000002,
		TreeItemCheckbox =		0x00000004,
		TreeItemChecked =		0x00000008,
		TreeItemActivated =		0x00000010,
        TreeItemImage =         0x00000020,
	};

    NVGUIItem(const char* pszName, GUITYPE Type)
		: m_Type(Type),			
		m_strName(pszName),		
		m_dwRefCount(1),		
		m_pClientData(NULL),	
		m_pParent(NULL),
        m_dwState(0),
        m_ImageIdx(-1),
        m_ItemState(0)
	{}

	virtual ~NVGUIItem()
	{																								
		RemoveChildren();
	}																								
	virtual unsigned long INTCALLTYPE AddRef() { m_dwRefCount++; return m_dwRefCount; }				
	virtual unsigned long INTCALLTYPE Release() { DWORD dwRefNew = --m_dwRefCount; if (dwRefNew == 0) delete this; return dwRefNew; }	
	virtual bool INTCALLTYPE GetInterface(const nv_sys::NVGUID& Interface, void** ppvObj) { return false; }

	virtual const char* INTCALLTYPE GetName() const { return m_strName.c_str(); }				
	virtual const char* INTCALLTYPE GetInfoText() const { return m_strInfoText.c_str(); }		
	virtual bool INTCALLTYPE SetInfoText(const char* pszInfo) { m_strInfoText = pszInfo; return true;}
	virtual GUITYPE INTCALLTYPE GetType() const { return m_Type; }										
	virtual void* INTCALLTYPE GetClientData() const { return m_pClientData; }							
	virtual void INTCALLTYPE SetClientData(void* pClientData) { m_pClientData = pClientData; }			
	virtual unsigned int INTCALLTYPE GetNumChildren() const { return (unsigned int)m_Children.size(); }	
	virtual INVGUIItem* INTCALLTYPE GetChild(unsigned int ChildNum) const								
	{																									
		assert(m_Children.size() > ChildNum);															
		if (m_Children.size() <= ChildNum)																
		{ assert(!"Invalid child index!");	return NULL; }												
		else return m_Children[ChildNum];																
	}																									
	virtual bool INTCALLTYPE AddChild(nv_gui::INVGUIItem* pChild)										
	{																									
		if (!pChild) return false;																		
		m_Children.push_back(pChild);																	
		pChild->SetParent(this);																		
		SAFE_ADDREF(pChild);																			
		return true;																					
	}																									
	virtual bool INTCALLTYPE RemoveChild(nv_gui::INVGUIItem* pItem)										
	{																									
		tvecGUIItems::iterator itrItem = m_Children.begin();											
		while (itrItem != m_Children.end())																
		{																								
			if ((*itrItem) == pItem)																	
			{																							
				pItem->RemoveChildren();
				m_Children.erase(std::remove(m_Children.begin(), m_Children.end(), pItem), m_Children.end()); 
				SAFE_RELEASE(pItem);
				return true;
			}																							
			itrItem++;
		}																								
		return true;																					
	}																									
	virtual bool INTCALLTYPE RemoveChildren()																	
	{																									
		tvecGUIItems::iterator itrItem = m_Children.begin();											
		while (itrItem != m_Children.end())																
		{																								
			(*itrItem)->RemoveChildren();
			SAFE_RELEASE(*itrItem);
			itrItem++;																					
		}																								
		m_Children.clear();																				
		return true;																					
	}							
    virtual void INTCALLTYPE HasImage(bool bImage, unsigned int idx )
    {
        if (bImage)
		    m_dwState |= TreeItemImage;
	    else
		    m_dwState &= ~TreeItemImage;
        m_ImageIdx = idx;
    }

    virtual unsigned int INTCALLTYPE GetImageIdx()
    {
        return m_ImageIdx;
    }

    virtual bool INTCALLTYPE IsImage()
    {
        return (m_dwState & TreeItemImage) ? true : false;
    }

    virtual void INTCALLTYPE            SetState(DWORD state)
    {
        m_ItemState = state;
    }

    virtual void INTCALLTYPE            SetState(INVGUIItem::ItemState state, bool on)
    {
        if (on)
		    m_ItemState  |= state;
	    else
		    m_ItemState  &= ~state;
    }

    virtual bool INTCALLTYPE            IsState(INVGUIItem::ItemState state)
    {
        return (m_ItemState & state) ? true : false;
    }
    
    virtual DWORD INTCALLTYPE           GetState()
    {
        return m_ItemState;
    }

    virtual bool INTCALLTYPE SetParent(nv_gui::INVGUIItem* pParent) { m_pParent = pParent; return true; }		
	virtual INVGUIItem* INTCALLTYPE GetParent() { return m_pParent; }											
public:						
	typedef I InterfaceType;
	GUITYPE             m_Type;																						
	std::string         m_strName;																				
	std::string         m_strInfoText;																			
	void*               m_pClientData;																				
	tvecGUIItems        m_Children;																			
	DWORD               m_dwRefCount;																					
	nv_gui::INVGUIItem* m_pParent;
    DWORD				m_dwState;
    unsigned int        m_ImageIdx;

    DWORD				m_ItemState;
};

#define DECLARE_ITEM_CONSTRUCT(ItemClass, Type)														\
public:																								\
	ItemClass(const char* pszName)																	\
		: NVGUIItem<InterfaceType>(pszName, Type)													\
		{};

#define DECLARE_ITEM(ItemClass, Type)	\
DECLARE_ITEM_CONSTRUCT(ItemClass, Type)



class NVGUIItem_Slider : public NVGUIItem<INVGUIItem_Slider>
{
	DECLARE_ITEM(NVGUIItem_Slider, GUITYPE_SLIDER)
	virtual unsigned int INTCALLTYPE GetPos() const
	{
		return m_Pos;
	}

	virtual void INTCALLTYPE SetPos(unsigned int Pos)
	{
		m_Pos = Pos;
	}

	virtual void INTCALLTYPE GetMinMaxStep(unsigned int& Min, unsigned int& Max, unsigned int& Step) const
	{
		Min = m_Min;
		Max = m_Max;
		Step = m_Step;
	}
	
	virtual void INTCALLTYPE SetMinMaxStep(unsigned int Min, unsigned int Max, unsigned int Step)
	{
		m_Min = Min;
		m_Max = Max;
		m_Step = Step;
	}
private:
	unsigned int m_Min;
	unsigned int m_Max;
	unsigned int m_Step;
	unsigned int m_Pos;
};

class NVGUIItem_Text : public NVGUIItem<INVGUIItem_Text>
{	
	DECLARE_ITEM(NVGUIItem_Text, GUITYPE_TEXT)
    
	virtual const char* INTCALLTYPE GetString() const { return m_strText.c_str(); }
	virtual void INTCALLTYPE SetString(const char* pszText) { m_strText = pszText; }
private:
	std::string m_strText;
};

class NVGUIItem_Bool : public NVGUIItem<INVGUIItem_Bool>
{
public:
	NVGUIItem_Bool(const char* pszName)
		: NVGUIItem<InterfaceType>(pszName, GUITYPE_BOOL),
		m_Bool(false)
	{}
	virtual bool INTCALLTYPE GetBool() const { return m_Bool; }
	virtual void INTCALLTYPE SetBool(const bool Bool) { m_Bool = Bool;  }
private:
	bool m_Bool;
};

class NVGUIItem_ListBox : public NVGUIItem<INVGUIItem_ListBox>
{
public:
    NVGUIItem_ListBox(const char* pszName)
        : NVGUIItem<InterfaceType>(pszName, GUITYPE_LISTBOX),
        m_Index(0)
    {}

    virtual int INTCALLTYPE GetIndex() const { return m_Index; }
    virtual void INTCALLTYPE SetIndex(int Index) { m_Index = Index; }
    virtual unsigned int INTCALLTYPE GetNumStrings() const { return m_vecStrings.size(); }
    virtual const char* INTCALLTYPE GetString(unsigned int Num) const { return m_vecStrings[Num].c_str(); }
    virtual void INTCALLTYPE AddString(const char* pszString) { m_vecStrings.push_back(pszString); }
	virtual void INTCALLTYPE SetPrivateData(unsigned int Index, void* pData) { m_mapIndexData[Index] = pData; }
	virtual void* INTCALLTYPE GetPrivateData(const unsigned int Index) const 
	{
		tmapIndexData::const_iterator itrData = m_mapIndexData.find(Index);
		if (itrData == m_mapIndexData.end())
			return NULL;
		return itrData->second;
	}

    virtual void INTCALLTYPE Clear() { m_vecStrings.clear(); m_mapIndexData.clear();}
private:
    typedef std::vector<std::string> tvecStrings;
	typedef std::map<unsigned int, void*> tmapIndexData;
	tmapIndexData m_mapIndexData;
    tvecStrings m_vecStrings;
    int m_Index;
};

class NVGUIItem_EditBox : public NVGUIItem<INVGUIItem_EditBox>
{	
	DECLARE_ITEM(NVGUIItem_EditBox, GUITYPE_EDITBOX)
    
	virtual const char* INTCALLTYPE GetString() const { return m_strText.c_str(); }
	virtual void INTCALLTYPE SetString(const char* pszText) { m_strText = pszText; }
private:
	std::string m_strText;
};

class NVGUIItem_FilePath : public NVGUIItem<INVGUIItem_FilePath>
{	
public:
	NVGUIItem_FilePath(const char* pszName)
		: NVGUIItem<InterfaceType>(pszName, GUITYPE_FILEPATH),
		m_bTruncated(true),
        m_bPreview(false),
		m_pszExtension("All Files (*.*)\0;*.*\0")
	{}

	virtual const char* INTCALLTYPE GetPath() const                         { return m_strPath.c_str(); }
	virtual void INTCALLTYPE SetPath(const char* pszPath)                   { m_strPath = pszPath; }
	virtual bool INTCALLTYPE IsTruncatedDisplay() const                     { return m_bTruncated; }
	virtual void INTCALLTYPE SetTruncatedDisplay(bool bTruncated)           { m_bTruncated = bTruncated; }
    virtual bool INTCALLTYPE IsPreview() const                              { return m_bPreview; }
    virtual void INTCALLTYPE SetPreview(bool bPreview)                      { m_bPreview = bPreview; };
	virtual const char* INTCALLTYPE GetExtensionString() const              { return m_pszExtension.c_str(); }
	virtual void INTCALLTYPE SetExtensionString(const char* pszExtension)   { m_pszExtension = pszExtension; }

private:
	std::string     m_strPath;
	std::string     m_pszExtension;
	bool            m_bTruncated;
    bool            m_bPreview;
};

class NVGUIItem_Branch : public NVGUIItem<INVGUIItem_Branch>
{
	DECLARE_ITEM(NVGUIItem_Branch, GUITYPE_BRANCH)
	virtual unsigned int INTCALLTYPE GetInfo() const
	{
		return m_Pos;
	}

	virtual void INTCALLTYPE SetPos(unsigned int Pos)
	{
		m_Pos = Pos;
	}

	virtual void INTCALLTYPE GetMinMaxStep(unsigned int& Min, unsigned int& Max, unsigned int& Step) const
	{
		Min = m_Min;
		Max = m_Max;
		Step = m_Step;
	}
	
	virtual void INTCALLTYPE SetMinMaxStep(unsigned int Min, unsigned int Max, unsigned int Step)
	{
		m_Min = Min;
		m_Max = Max;
		m_Step = Step;
	}
private:
	unsigned int m_Min;
	unsigned int m_Max;
	unsigned int m_Step;
	unsigned int m_Pos;
};

class NVGUIItem_Color : public NVGUIItem<INVGUIItem_Color>
{
public:
	NVGUIItem_Color(const char* pszName)
		: NVGUIItem<InterfaceType>(pszName, GUITYPE_COLOR),
		m_bAlpha(false),
		m_Color(0.0f, 0.0f, 0.0f, 1.0f)
	{}

	virtual const vec4& INTCALLTYPE GetColor() const { return m_Color; }
	virtual void INTCALLTYPE SetColor(const vec4& color) { m_Color = color; if (!m_bAlpha) m_Color.w = 1.0f; }
	virtual bool INTCALLTYPE HasAlpha() const { return m_bAlpha; }
	virtual void INTCALLTYPE SetAlpha(bool bAlpha) { m_bAlpha = bAlpha;  }

private:
	bool m_bAlpha;
	vec4 m_Color;
};

class NVGUIItem_Float : public NVGUIItem<INVGUIItem_Float>
{
public:
	NVGUIItem_Float(const char* pszName)
		: NVGUIItem<InterfaceType>(pszName, GUITYPE_FLOAT),
		m_Float(0.0f),
		m_fMin(0.0f),
		m_fMax(1.0f),
        m_fStep(0.0f)
	{}

	virtual const float INTCALLTYPE GetFloat() const { return m_Float; }
	virtual void INTCALLTYPE SetFloat(const float fVal) { m_Float = fVal; }
	virtual void INTCALLTYPE GetMinMaxStep(float& Min, float& Max, float& Step) const { Min = m_fMin; Max = m_fMax; Step = m_fStep; }
    virtual void INTCALLTYPE SetMinMaxStep(float Min, float Max, float Step) { m_fMin = Min; m_fMax = Max; m_fStep = Step; }

private:
	float m_Float;
	float m_fMin;
	float m_fMax;
    float m_fStep;
};

class NVGUIItem_Dword : public NVGUIItem<INVGUIItem_Dword>
{
public:
	NVGUIItem_Dword(const char* pszName)
		: NVGUIItem<InterfaceType>(pszName, GUITYPE_DWORD),
		m_dwValue(0),
		m_dwMin(0),
		m_dwMax(1)
	{}

	virtual DWORD INTCALLTYPE GetDword() const { return m_dwValue; }
	virtual void INTCALLTYPE SetDword(DWORD dwValue) { m_dwValue = dwValue; }
	virtual void INTCALLTYPE GetMinMax(DWORD& dwMin, DWORD& dwMax) const { dwMin = m_dwMin; dwMax = m_dwMax; }
	virtual void INTCALLTYPE SetMinMax(DWORD dwMin, DWORD dwMax) { m_dwMin = dwMin; m_dwMax = dwMax; }

private:
	DWORD m_dwValue;
	DWORD m_dwMin;
	DWORD m_dwMax;
};

class NVGUIItem_Int : public NVGUIItem<INVGUIItem_Int>
{
public:
	NVGUIItem_Int(const char* pszName)
		: NVGUIItem<InterfaceType>(pszName, GUITYPE_INT),
		m_Value(0),
		m_Min(0),
		m_Max(1)
	{}

	virtual int INTCALLTYPE GetInt() const { return m_Value; }
	virtual void INTCALLTYPE SetInt(int Value) { m_Value = Value; }
	virtual void INTCALLTYPE GetMinMax(int& Min, int& Max) const { Min = m_Min; Max = m_Max; }
	virtual void INTCALLTYPE SetMinMax(int Min, int Max) { m_Min = Min; m_Max = Max; }

private:
	int m_Value;
	int m_Min;
	int m_Max;
};

class NVGUIItem_Vector2 : public NVGUIItem<INVGUIItem_Vector2>
{
public:
	NVGUIItem_Vector2(const char* pszName)
		: NVGUIItem<InterfaceType>(pszName, GUITYPE_VECTOR2)
	{}

	virtual const vec2& INTCALLTYPE GetVector() const { return m_Vec; }
	virtual void INTCALLTYPE SetVector(const vec2& theVec) { m_Vec = theVec; }

private:
	vec2 m_Vec;
};

class NVGUIItem_Vector3 : public NVGUIItem<INVGUIItem_Vector3>
{
public:
	NVGUIItem_Vector3(const char* pszName)
		: NVGUIItem<InterfaceType>(pszName, GUITYPE_VECTOR3)
	{}

	virtual const vec3& INTCALLTYPE GetVector() const { return m_Vec; }
	virtual void INTCALLTYPE SetVector(const vec3& theVec) { m_Vec = theVec; }

private:
	vec3 m_Vec;
};

class NVGUIItem_Vector4 : public NVGUIItem<INVGUIItem_Vector4>
{
public:
	NVGUIItem_Vector4(const char* pszName)
		: NVGUIItem<InterfaceType>(pszName, GUITYPE_VECTOR4)
	{}

	virtual const vec4& INTCALLTYPE GetVector() const { return m_Vec; }
	virtual void INTCALLTYPE SetVector(const vec4& theVec) { m_Vec = theVec; }

private:
	vec4 m_Vec;
};

class NVGUIItem_Matrix : public NVGUIItem<INVGUIItem_Matrix>
{
public:
	NVGUIItem_Matrix(const char* pszName, unsigned int Rows, unsigned int Columns)
		: NVGUIItem<InterfaceType>(pszName, GUITYPE_MATRIX),
		m_pArray(NULL),
		m_Rows(Rows),
		m_Columns(Columns)
	{
		m_pArray = (float*)malloc(sizeof(float) * m_Rows * m_Columns);
	}

	~NVGUIItem_Matrix()
	{
		if (m_pArray)
			free(m_pArray);
	}

	virtual const float* INTCALLTYPE GetArray() const { return m_pArray; }
	virtual void INTCALLTYPE SetArray(const float* pArray) { assert(m_pArray); memcpy(m_pArray, pArray, m_Rows * m_Columns * sizeof(float)); }
	virtual unsigned int INTCALLTYPE GetRows() const { return m_Rows; }
	virtual unsigned int INTCALLTYPE GetColumns() const { return m_Columns; }

private:
	float* m_pArray;
	unsigned int m_Rows;
	unsigned int m_Columns;
};

/*

interface INVGUIItem_EditBox : public INVGUIItem
{
	virtual const char* GetString() const = 0;
	virtual void SetString(const char* pszString) = 0;
};

interface INVGUIItem_Int : public INVGUIItem
{
	virtual int GetInt() const = 0;
	virtual void SetInt(const int iVal) = 0;
};

interface INVGUIItem_Matrix4x4 : public INVGUIItem
{
	virtual const mat4& GetMatrix() const = 0;
	virtual void SetMatrix(const mat4& mat) = 0;
};

interface INVGUIItem_Matrix3x3 : public INVGUIItem
{
	virtual const mat3& GetMatrix() const = 0;
	virtual void SetMatrix(const mat3& mat) = 0;
};

*/


}; // namespace nv_gui

#endif // __NVGUIData_H
