/*********************************************************************NVMH4****
Path:  plugins\nvmax
File:  TextureMgr.cpp

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

Texture manager from the metalbump sample sources

  cmaughan - I improved this along the way and fixed a couple of key bugs, the
  main one being that the reference counting never actually worked because copies
  of the cache entries were being made and the copy's ref count was being updated, 
  not the original.  Also calls to SAFE_RELEASE were nulling ref-counted pointers.



******************************************************************************/
#include "pch.h"

using namespace nv_renderdevice;
using namespace nv_sys;

TextureMgr	TheTextureMgr;
TextureMgr::TextureMgr()
{
	m_List.clear();
}


//______________________________________
//
//	Default destructor 
//
//______________________________________

TextureMgr::~TextureMgr()
{
	int	i,Count;

	Count = m_List.size();

	for(i=0; i < Count; i++)
	{
		SAFE_RELEASE(m_List[i].m_Texture);
	}

	m_List.clear();

}

//______________________________________
//
//	Find 
//
//______________________________________

int TextureMgr::Find(const std::string& Name)
{
	int	i,Count;

	if(Name.length())
	{
		Count = m_List.size();

		for(i=0; i < Count; i++)
		{
			if(Name == m_List[i].m_Name)
			{
				return(i);
			}
		}

	}

	return(-1);
}

int TextureMgr::Find(INVTexture* pTexture)
{
	int	i,Count;

	if(pTexture)
	{
		Count = m_List.size();

		for(i=0; i < Count; i++)
		{
			if(pTexture == m_List[i].m_Texture)
			{
				return(i);
			}
		}

	}

	return(-1);
}


//______________________________________
//
//	Add 
//
//______________________________________

void TextureMgr::Add(const std::string& Name, NVTEXTURETARGETTYPE Type, INVTexture *Texture)
{
	TCache	T;

	assert(Find(Texture) == -1);
	assert(Find(Name) == -1);

	T.m_Name	= Name;
	T.m_Type    = Type;
	T.m_Texture = Texture;
	T.m_Ref		= 1;
	T.m_Texture->AddRef();

	DISPDBG(3, "TextureMgr::Add: " << Name.data());

	m_List.push_back(T);

}

//______________________________________
//
//	Get 
//
//______________________________________

INVTexture*	TextureMgr::Get(int Index)
{
	if(Index >=0 && Index < m_List.size())
	{
		TCache&	T = m_List[Index];

		assert(T.m_Texture);
		T.m_Texture->AddRef();
		T.m_Ref++;

		return(T.m_Texture);
	}

	return(NULL);

}

const char* TextureMgr::GetTextureName(INVTexture* pTexture)
{
	int Index = Find(pTexture);
	if (Index >=0 && Index < m_List.size())
	{
		return m_List[Index].m_Name.c_str();
	}
	return "";
}

//______________________________________
//
//	AddRef 
//
//______________________________________

void TextureMgr::AddRef(const std::string& Name, NVTEXTURETARGETTYPE Type)
{
	int		Index;

	Index = Find(Name);

	if(Index >=0 && Index < m_List.size())
	{
		DISPDBG(3, "TextureMgr::AddRef: " << Name.data());
		TCache&	T = m_List[Index];

		T.m_Texture->AddRef();
		T.m_Ref++;
	}

}

void TextureMgr::AddRef(INVTexture* pTexture)
{
	int		Index;

	Index = Find(pTexture);

	if(Index >=0 && Index < m_List.size())
	{
		TCache&	T = m_List[Index];
		DISPDBG(3, "TextureMgr::AddRef: " << T.m_Name);

		T.m_Texture->AddRef();
		T.m_Ref++;
	}

}

//______________________________________
//
//	Release 
//
//______________________________________

int TextureMgr::Release(const std::string& Name, NVTEXTURETARGETTYPE Type)
{
	int		Index;

	Index = Find(Name);

	DISPDBG(4, "TextureMgr::Release: " << Name.data());

	if(Index >=0 && Index < m_List.size())
	{
		TCache&	T = m_List[Index];
		T.m_Ref--;
		if (T.m_Texture)
			T.m_Texture->Release();

		if(T.m_Ref == 0)
		{
			int Ref = T.m_Ref;
			DISPDBG(3, "Texture completely freed");
			m_List.erase(m_List.begin() + Index); 
			return Ref;
		}
		return T.m_Ref;
	}
	return 0;	

}

int TextureMgr::Release(INVTexture* pTexture)
{
	int		Index;

	Index = Find(pTexture);

	DISPDBG(4, "TextureMgr::Release (Ptr version): " << pTexture);

	if(Index >=0 && Index < m_List.size())
	{
		TCache&	T = m_List[Index];
		T.m_Ref--;

		DISPDBG(3, "Name: " << T.m_Name);
		if (T.m_Texture)
			T.m_Texture->Release();

		if(T.m_Ref == 0)
		{
			int Ref = T.m_Ref;
			DISPDBG(3, "Texture completely freed");
			m_List.erase(m_List.begin() + Index);
			return Ref;
			
		}
		return T.m_Ref;
	}
	return 0;
}
