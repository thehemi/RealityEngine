/*********************************************************************NVMH4****
NVSDK not found!
Path:  
File:  TextureMgr.h

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

See texturemgr.cpp


******************************************************************************/

#ifndef TEXTUREMGR_H
#define TEXTUREMGR_H

#if _MSC_VER >= 1000
#pragma once
#endif 

typedef struct
{
	std::string				m_Name;
	nv_renderdevice::NVTEXTURETARGETTYPE	m_Type;
	nv_renderdevice::INVTexture*			m_Texture;
	int						m_Ref;
		
}TCache;

typedef std::vector<TCache> TCacheList;
class TextureMgr : public Singleton<TextureMgr>
{
	public:

		TCacheList	m_List;
		
		TextureMgr();
		~TextureMgr();

		int						Find(const std::string& Name);
		int						Find(nv_renderdevice::INVTexture* pTexture);
		void					Add(const std::string& Name, nv_renderdevice::NVTEXTURETARGETTYPE Type, nv_renderdevice::INVTexture *pTexture);
		nv_renderdevice::INVTexture* Get(int Index);
		const char*				GetTextureName(nv_renderdevice::INVTexture* pTexture);
		int						Release(const std::string& Name, nv_renderdevice::NVTEXTURETARGETTYPE Type);
		int						Release(nv_renderdevice::INVTexture* pTexture);
		void					AddRef(const std::string& Name, nv_renderdevice::NVTEXTURETARGETTYPE Type);
		void					AddRef(nv_renderdevice::INVTexture* pTexture);


};

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif


