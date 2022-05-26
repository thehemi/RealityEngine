/*********************************************************************NVMH4****
Path:  plugins\nvmax
File:  Utility.cpp

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

Utilities used by the plugin.


******************************************************************************/

#include "pch.h"

//_____________________________________________________________________________
//
//	Globals	
//
//_____________________________________________________________________________

TCHAR	gFilePath[MAX_PATH];
typedef std::map<std::string, std::string> tMapFileToFound;
tMapFileToFound gMapFileToFound;


//_____________________________________________________________________________
//
//	Functions	
//
//_____________________________________________________________________________

//_____________________________________
//
//	GetFileResource
//
//_____________________________________

bool GetFileResource(const char *Name, const char *Type, void **Data, unsigned long &Size)
{
	HRSRC			Rec;
	HGLOBAL			Mem;

	Rec = FindResource(g_hInstance,
					   Name,Type);

	if(Rec)
	{
		Mem  = LoadResource(g_hInstance,Rec);
		Size = SizeofResource(g_hInstance,Rec);

		if(Mem && Size)
		{
			*Data = LockResource(Mem);

			return(true);
		}

	}
	
	return(false);
}

//_____________________________________
//
//	FindFile
//
//_____________________________________

std::string FindFile(const std::string& strFile)
{
	int			i;
	BOOL		Found;
	TCHAR		*Part;
	Interface	*I;
	Found = 0;


	if (strFile.empty())
		return "";

	TSTR p,f,e;
	TSTR name(strFile.c_str());
	SplitFilename(name, &p, &f, &e);

	Found = SearchPath(p.data(),f.data(),e.data(),MAX_PATH,gFilePath,&Part);
	if(!Found)
	{

		I = GetCOREInterface();

		// Search the from where the file was loaded....

		TSTR CurFileP = I->GetCurFilePath();
		TSTR tP;
		
		SplitFilename(CurFileP,&tP,NULL,NULL);
		Found = SearchPath(tP.data(),f.data(),e.data(),MAX_PATH,gFilePath,&Part);
		
		//
		//	Search maps
		//
		if(!Found)
		{
			for (i=0; i<TheManager->GetMapDirCount(); i++) {
				TCHAR* dir = TheManager->GetMapDir(i);
				if((Found = SearchPath(dir,f.data(),e.data(),MAX_PATH,gFilePath,&Part)))
				{
					break;
				}
			}
		}

		// not sure why we search here....
		if(!Found)
		{
			//
			//	Search plugins
			//
			for(i=0; i < I->GetPlugInEntryCount(); i++)
			{
				TCHAR* dir = I->GetPlugInDir(i);
				if((Found = SearchPath(dir,f.data(),e.data(),MAX_PATH,gFilePath,&Part)))
				{
					break;
				}
			}

		}
	}

	if(Found)
	{
		return(gFilePath);
	}
	else
	{
		return("");
	}

}

//_____________________________________
//
//	GetTriObjectFromNode
//
//_____________________________________

TriObject* GetTriObjectFromNode(INode *Node, TimeValue T, bool &Delete) 
{
	Delete		= false;
	Object *Obj = Node->EvalWorldState(T).obj;

	if(Obj && Obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID,0))) 
	{ 
		TriObject *Tri = (TriObject *)Obj->ConvertToType(T,Class_ID(TRIOBJ_CLASS_ID,0));

		if(Obj != Tri) 
		{
			Delete = true;
		}

		return(Tri);
	}
	else 
	{
		return(NULL);
	}

}


//_____________________________________
//
//	FindNodeRef
//
//_____________________________________

INode* FindNodeRef(ReferenceTarget *Rt) 
{
	DependentIterator Di(Rt);
	ReferenceMaker	*Rm;
	INode	*Nd = NULL;

	while (Rm = Di.Next()) 
	{	
		if(Rm->SuperClassID() == BASENODE_CLASS_ID) 
		{
			return (INode *)Rm;
		}

		Nd = FindNodeRef((ReferenceTarget *)Rm);

		if(Nd)
		{
			 return(Nd);
		}
	}

	return(NULL);
}


//_____________________________________
//
//	AddNormal
//
//_____________________________________

void VNormal::AddNormal(Point3 &N, unsigned long Smooth, Point3 &S, Point3 &T) 
{
	if((!(Smooth & m_Smooth)) && m_Init) 
	{
		if(m_Next)
		{	
			m_Next->AddNormal(N,Smooth,S,T);
		}
		else 
		{
			m_Next = new VNormal(N,Smooth,S,T);
		}
	} 
	else
	{
		m_Normal += N;
		m_S		 += S;
		m_T		 += T;
		m_Smooth |= Smooth;
		m_Init    = true;
	}
}

//_____________________________________
//
//	GetNormal
//
//_____________________________________

Point3& VNormal::GetNormal(unsigned long Smooth, Point3 &S, Point3 &T, Point3 &SxT)
{
	if((m_Smooth & Smooth) || !m_Next)
	{
		 S	 = m_S;
		 T	 = m_T;
		 SxT = m_SxT;

		 return(m_Normal);
	}
	else
	{
		 return(m_Next->GetNormal(Smooth,S,T,SxT));	
	}
}

//_____________________________________
//
//	Normalize
//
//_____________________________________

void VNormal::Normalize() 
{
	VNormal	*Ptr,*Prev;
	Matrix3	Mat;

	Ptr  = m_Next;
	Prev = this;

	while(Ptr) 
	{
		if(Ptr->m_Smooth & m_Smooth) 
		{
			m_Normal += Ptr->m_Normal;
			m_S		 += Ptr->m_S;
			m_T      += Ptr->m_T;

			Prev->m_Next = Ptr->m_Next;
			Ptr->m_Next = NULL;

			delete Ptr;

			Ptr = Prev->m_Next;
		} 
		else 
		{
			Prev = Ptr;
			Ptr  = Ptr->m_Next;
		}
	}


	Point3Normalize(m_Normal);
	Point3Normalize(m_S);

	m_T	= CrossProd(m_S,m_Normal);
	Point3Normalize(m_T);

	m_S	= CrossProd(m_Normal,m_T);
	Point3Normalize(m_S);

	m_SxT = m_Normal;

	if(m_Next)
	{
		 m_Next->Normalize();
	}

}

//_____________________________________
//
//	GetString 
//
//_____________________________________

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (g_hInstance)
	{
		return(LoadString(g_hInstance, id, buf, sizeof(buf)) ? buf : NULL);
	}

	return(NULL);
}

std::string GetFileNameAndExtension(const char* pszPath)
{
	TSTR p,f,e;
	TSTR strPath(pszPath);
	SplitFilename(strPath, &p, &f, &e);

	if (!f)
	{
		assert(0);
		return std::string("");
	}

	if (!e)
	{
		return f;
	}

	return std::string(f + e);
}

