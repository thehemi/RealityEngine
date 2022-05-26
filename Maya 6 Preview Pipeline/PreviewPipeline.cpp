#include "dxstdafx.h"

#pragma warning( push )
#pragma warning( disable: 4995 ) // strsafe triggers warnings for STL
#pragma warning( disable: 4005 ) // strsafe triggers warnings for tchar.h
#include "PreviewPipeline.h"
#pragma warning( pop )


HRESULT CPipeline::Create() 
{ 
	LPDXCCRESOURCE pResource= NULL;
	pPreviewEngine= NULL;
	fTime= 0;
	DXCCCreateManager(&pDXCCManager);
	DXCCCreateFrame(&pDXCCRoot);
	pDXCCManager->CreateResource(pDXCCRoot, IID_IDXCCFrame, TRUE, &pResource);
	pResource->SetName("DXCC_ROOT");
	DXCC_RELEASE(pResource);

	ReadWriteMutex= CreateMutexA(NULL, FALSE, "DXCCPipeline_SceneReadWriteLock");
	InitializeCriticalSection(&ZeroSceneReadersSection);
	SceneReadersCount= 0;
	ZeroSceneReadersEvent= CreateEventA(NULL, TRUE, TRUE, "DXCCPipeline_ZeroReaders");

	return S_OK; 
}

HRESULT CPipeline::Destroy() 
{ 

	DXCC_RELEASE(pDXCCRoot);
	DXCC_RELEASE(pDXCCManager);
	CloseHandle(ReadWriteMutex);
	ReadWriteMutex= NULL;
	CloseHandle(ZeroSceneReadersEvent);
	ZeroSceneReadersEvent= NULL;
	DeleteCriticalSection(&ZeroSceneReadersSection);
		
	return S_OK; 
}

HRESULT CPipeline::SetEngine( CPipelineEngine* pPreviewPipelineEngine )  
{ 
	TriggerDeviceEvents.OnD3DDeviceDestroy();
	pPreviewEngine= pPreviewPipelineEngine; 
	TriggerDeviceEvents.OnD3DDeviceCreate();
	return S_OK;
}



void CPipeline::SetTime(float time) 
{
	fTime= time;
}

float CPipeline::GetTime() 
{
	return fTime;
}

CPipelineEngine* 
CPipeline::AccessEngine()
{
	return pPreviewEngine;
}

LPDXCCMANAGER
CPipeline::AccessManager()
{
	return pDXCCManager;
}

LPDXCCFRAME 
CPipeline::AccessRoot()
{
	return pDXCCRoot;
}


bool CPipeline::SceneWriteLock(BOOL WaitForLock, CPipelineLock& Lock) 
{
	if(Lock.isLocked()==false)
	{
		if(GetSingleObject(WaitForLock, ReadWriteMutex))
		{
			if(GetSingleObject(WaitForLock, ZeroSceneReadersEvent))
			{
				Lock.Locked=true;
				Lock.Type=CPipelineLock::TYPE_SCENE_WRITE;
				Lock.pPreviewPipeline=this;
				return true;
			}
			else
			{
				ReleaseMutex(ReadWriteMutex);
				return false;
			}
		}
		else
			return false;
	}
	else
		return false;
}
	
void CPipeline::SceneWriteUnlock(CPipelineLock& Lock) 
{
	if(Lock.isLocked()==true && Lock.ofType()==CPipelineLock::TYPE_SCENE_WRITE)
	{
		Lock.Locked=false;
		Lock.pPreviewPipeline=NULL;

		ReleaseMutex(ReadWriteMutex);
	}
}
CPipelineLock::CPipelineLock(bool AutoUnlock)
{
	Type=TYPE_NONE; 
	Locked=false; 
	UnlockOnDestuction= AutoUnlock;
	pPreviewPipeline=NULL; 
};

CPipelineLock::~CPipelineLock()
{
	if(Locked && UnlockOnDestuction && pPreviewPipeline)
	{
		switch(Type)
		{

		case TYPE_SCENE_WRITE:
			pPreviewPipeline->SceneWriteUnlock(*this);
			break;
		case TYPE_SCENE_READ:
			pPreviewPipeline->SceneReadUnlock(*this);
			break;
		};
	}
}

bool CPipeline::SceneReadLock(BOOL WaitForLock, CPipelineLock& Lock)
{
	if(Lock.isLocked()==false)
	{
		if(GetSingleObject(WaitForLock, ReadWriteMutex))
		{
			//EnterCriticalSection(&ZeroSceneReadersSection); //using ReadWriteMutex instead
			if(0==SceneReadersCount)
				ResetEvent(ZeroSceneReadersEvent);
			++SceneReadersCount; //InterlockedIncrement(&SceneReadersCount);//using ReadWriteMutex instead
			//LeaveCriticalSection(&ZeroSceneReadersSection); //using ReadWriteMutex instead

			ReleaseMutex(ReadWriteMutex);

			Lock.Locked=true;
			Lock.Type=CPipelineLock::TYPE_SCENE_READ;
			Lock.pPreviewPipeline=this;

			return true;
		}
		else
			return false;
	}
	else
		return false;

}


void CPipeline::SceneReadUnlock(CPipelineLock& Lock)
{
	if(Lock.isLocked()==true && Lock.ofType()==CPipelineLock::TYPE_SCENE_READ)
	{
		EnterCriticalSection(&ZeroSceneReadersSection); 

		Lock.Locked=false;
		Lock.pPreviewPipeline=NULL;
		
		if(SceneReadersCount > 0
			&& 0== --SceneReadersCount)
		{
			SetEvent(ZeroSceneReadersEvent);
		}
		
		LeaveCriticalSection(&ZeroSceneReadersSection);
	}
}

bool CPipeline::IsValid()
{
	return (pPreviewEngine && pDXCCManager && pDXCCRoot);
}

HRESULT CPipelineEngineEventsTrigger::RegisterEvents(CPipelineEngineEvents* pCall)
{
	UnregisterEvents(pCall);
	Events.push_back(pCall);
	return S_OK;
}

HRESULT CPipelineEngineEventsTrigger::UnregisterEvents(CPipelineEngineEvents* pCall) 
{
	for(UINT i= 0; 
		i < Events.size(); 
		i++)
	{
		const CPipelineEngineEvents* event= Events.at(i);
		if(event==pCall)
		{
			Events.erase( Events.begin( ) + i);
			return S_OK;
		}
	}
	
	return E_INVALIDARG;
}

HRESULT CPipelineManagerEventsTrigger::RegisterEvents(CPipelineManagerEvents* pCall)
{
	UnregisterEvents(pCall);
	Events.push_back(pCall);
	return S_OK;
}

HRESULT CPipelineManagerEventsTrigger::UnregisterEvents(CPipelineManagerEvents* pCall)
{
	for(UINT i= 0; 
		i < Events.size(); 
		i++)
	{
		const CPipelineManagerEvents* event= Events.at(i);
		if(event==pCall)
		{
			Events.erase( Events.begin( ) + i);
			return S_OK;
		}
	}
	
	return E_INVALIDARG;
}

HRESULT CPipelineViewerEventsTrigger::RegisterEvents(CPipelineViewerEvents* pCall)
{
	UnregisterEvents(pCall);
	Events.push_back(pCall);
	return S_OK;
}

HRESULT CPipelineViewerEventsTrigger::UnregisterEvents(CPipelineViewerEvents* pCall)
{
	for(UINT i= 0; 
		i < Events.size(); 
		i++)
	{
		const CPipelineViewerEvents* event= Events.at(i);
		if(event==pCall)
		{
			Events.erase( Events.begin( ) + i);
			return S_OK;
		}
	}
	
	return E_INVALIDARG;
}



HRESULT CPipelineEngineEventsTrigger::OnD3DDeviceCreate()
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineEngineEvents* &event= Events.at(i);
		hr= event->OnD3DDeviceCreate();
	}
	return hr;
};

HRESULT CPipelineEngineEventsTrigger::OnD3DDeviceReset()
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineEngineEvents* &event= Events.at(i);
		hr= event->OnD3DDeviceReset();
	}
	return hr;
};

HRESULT CPipelineEngineEventsTrigger::OnD3DDeviceLost()
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineEngineEvents* &event= Events.at(i);
		hr= event->OnD3DDeviceLost();
	}
	return hr;
};

HRESULT CPipelineEngineEventsTrigger::OnD3DDeviceDestroy()
{
	HRESULT hr= S_OK;
	for(UINT i= 0;
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineEngineEvents* &event= Events.at(i);
		hr= event->OnD3DDeviceDestroy();
	}
	return hr;
};


HRESULT CPipelineManagerEventsTrigger::OnReload()
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineManagerEvents* &event= Events.at(i);
		hr= event->OnReload();
	}
	return hr;
};

HRESULT CPipelineManagerEventsTrigger::OnResourceAdd(IDXCCResource* pRes)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineManagerEvents* &event= Events.at(i);
		hr= event->OnResourceAdd(pRes);
	}
	return hr;
};


CPipelineManagerEvents::CPipelineManagerEvents()
{
	ExclusiveModeMutex= CreateMutex(NULL, false, NULL);
}

CPipelineManagerEvents::~CPipelineManagerEvents()
{
	CloseHandle(ExclusiveModeMutex);

}


HRESULT CPipelineManagerEventsTrigger::OnResourceRecycle(IDXCCResource* pRes)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineManagerEvents* &event= Events.at(i);
		hr= event->OnResourceRecycle(pRes);
	}
	return hr;
};

HRESULT CPipelineManagerEventsTrigger::OnResourceRemove(IDXCCResource* pRes)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineManagerEvents* &event= Events.at(i);
		hr=event->OnResourceRemove(pRes);
	}
	return hr;
};

HRESULT CPipelineViewerEventsTrigger::OnFrameChildAdded(IDXCCFrame* pParent, IDXCCFrame* pChild)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineViewerEvents* &event= Events.at(i);
		hr= event->OnFrameChildAdded(pParent, pChild);
	}
	return hr;
};

HRESULT CPipelineViewerEventsTrigger::OnFrameChildRemoved(IDXCCFrame* pParent, IDXCCFrame* pChild) 
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineViewerEvents* &event= Events.at(i);
		hr= event->OnFrameChildRemoved(pParent, pChild);
	}
	return hr;
};

HRESULT CPipelineViewerEventsTrigger::OnFrameMemberAdded(IDXCCFrame* pParent, IUnknown* pChild)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineViewerEvents* &event= Events.at(i);
		hr= event->OnFrameMemberAdded(pParent, pChild);
	}
	return hr;
};

HRESULT CPipelineViewerEventsTrigger::OnFrameMemberRemoved(IDXCCFrame* pParent, IUnknown* pChild)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineViewerEvents* &event= Events.at(i);
		hr= event->OnFrameMemberRemoved(pParent, pChild);
	}
	return hr;
};

HRESULT CPipelineViewerEventsTrigger::OnMeshChange(IDXCCMesh* pMesh)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineViewerEvents* &event= Events.at(i);
		hr= event->OnMeshChange(pMesh);
	}
	return hr;
};

HRESULT CPipelineViewerEventsTrigger::OnMeshDeclarationChange(IDXCCMesh* pMesh, IDXCCMesh* pOldMesh)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineViewerEvents* &event= Events.at(i);
		hr= event->OnMeshDeclarationChange(pMesh, pOldMesh);
	}
	return hr;
};

HRESULT CPipelineViewerEventsTrigger::OnMeshTopologyChange(IDXCCMesh* pMesh, IDXCCMesh* pOldMesh)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineViewerEvents* &event= Events.at(i);
		hr= event->OnMeshTopologyChange(pMesh, pOldMesh);
	}
	return hr;
};

HRESULT CPipelineViewerEventsTrigger::OnMeshVertexChange(IDXCCMesh* pMesh, UINT vertMin, UINT vertMax)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineViewerEvents* &event= Events.at(i);
		hr= event->OnMeshVertexChange(pMesh, vertMin, vertMax);
	}
	return hr;
};

HRESULT CPipelineViewerEventsTrigger::OnMeshSubVertexChange(IDXCCMesh* pMesh, D3DDECLUSAGE Usage, UINT UsageIndex, UINT vertMin, UINT vertMax)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineViewerEvents* &event= Events.at(i);
		hr= event->OnMeshSubVertexChange(pMesh, Usage, UsageIndex, vertMin, vertMax);
	}
	return hr;
};
HRESULT CPipelineViewerEventsTrigger::OnMeshAttributeChange(IDXCCMesh* pMesh, UINT faceMin, UINT faceMax)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineViewerEvents* &event= Events.at(i);
		hr= event->OnMeshAttributeChange(pMesh, faceMin, faceMax);
	}
	return hr;
};

HRESULT CPipelineViewerEventsTrigger::OnMeshMaterialChange(IDXCCMesh* pMesh, DXCCATTRIBUTE Attrib, ID3DXEffect* pOldMaterial)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineViewerEvents* &event= Events.at(i);
		hr= event->OnMeshMaterialChange(pMesh, Attrib, pOldMaterial);
	}
	return hr;
};

HRESULT CPipelineViewerEventsTrigger::OnMaterialEffectChange(ID3DXEffect* pMaterial, LPD3DXEFFECT pOldEffect)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineViewerEvents* &event= Events.at(i);
		hr= event->OnMaterialEffectChange(pMaterial, pOldEffect);
	}
	return hr;
};

HRESULT CPipelineViewerEventsTrigger::OnMaterialParameterChange(ID3DXEffect* pMaterial, D3DXHANDLE hParameter)
{
	HRESULT hr= S_OK;
	for(UINT i= 0; 
		i < Events.size() 
		&& hr == S_OK; 
		i++)
	{
		CPipelineViewerEvents* &event= Events.at(i);
		hr= event->OnMaterialParameterChange(pMaterial, hParameter);
	}
	return hr;
};



CBank::CBank()
{
	GroupGenerator= 0;
	ItemGenerator= 0;
};

CBank::GroupId
CBank::GenerateGroupId()
{
	return GroupGenerator++;
}


CBank::CItemBank::CPair* 
CBank::AddItemToGroup(GroupId Gid, void* pValue)
{
	POSITION result = ItemBank.SetAt(ItemGenerator, pValue);
	if(result)
	{
		GroupBank.Insert(Gid, ItemGenerator);
		ItemToGroup.SetAt(ItemGenerator, Gid);
	}
	return ( result ? ItemBank.GetAt(result) : NULL);
}


void 
CBank::RemoveByItem( ItemId Iid , bool DeleteValue)
{
	POSITION IidToGid= ItemToGroup.Lookup(Iid);
	if(IidToGid)
	{
		GroupId Gid= ItemToGroup.GetAt(IidToGid)->m_value;

		for( POSITION pos= GroupBank.FindFirstWithKey(Gid);
			pos != NULL;
			GroupBank.GetNextWithKey(pos, Gid))
		{
			if(Iid == GroupBank.GetValueAt(pos))
			{
				GroupBank.RemoveAt(pos);
				break;
			}
		}

		ItemToGroup.RemoveAt(IidToGid);
	}
	
	POSITION valuePosition= ItemBank.Lookup(Iid);
	if(valuePosition)
	{
		if(DeleteValue)
		{
			delete ItemBank.GetValueAt(valuePosition);
		}
		ItemBank.RemoveAt(valuePosition);
	}
}

void 
CBank::RemoveByGroup( GroupId Gid , bool DeleteValues)
{
	for( POSITION pos= GroupBank.FindFirstWithKey(Gid);
		pos != NULL;
		GroupBank.GetNextWithKey(pos, Gid))
	{
		ItemId Iid = GroupBank.GetValueAt(pos);

		ItemToGroup.RemoveKey(Iid);

		POSITION valuePosition= ItemBank.Lookup(Iid);
		if(valuePosition)
		{
			if(DeleteValues)
			{
				delete ItemBank.GetValueAt(valuePosition);
			}
			ItemBank.RemoveAt(valuePosition);
		}
	}

	GroupBank.RemoveKey(Gid);
}


void 
CBank::RemoveAll(bool DeleteValues)
{
	if(DeleteValues)
	{
		for( POSITION pos= ItemBank.GetHeadPosition();
			pos != NULL;
			GroupBank.GetNext(pos))
		{
			delete ItemBank.GetValueAt(pos);
		}
	}

	ItemBank.RemoveAll();
	GroupBank.RemoveAll();
	ItemToGroup.RemoveAll();
}

