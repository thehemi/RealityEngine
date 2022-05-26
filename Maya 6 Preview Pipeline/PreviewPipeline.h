//--------------------------------------------------------------------------------------
// File: PreviewPipeline.h
//
// Defines the interfaces for the preview pipeline
//
// Copyright (c) Microsoft Corporation. All rights reserved
//--------------------------------------------------------------------------------------
#pragma once
#ifndef PREVIEWPIPELINE_H
#define PREVIEWPIPELINE_H

#include "dxcc.h"

#include <vector>
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <atlcoll.h>



// Forward declaration
class CPipelineLock;
class CPipelineEngineEvents;
class CPipelineManagerEvents;
class CPipelineViewerEvents;
class CPipelineEngineEventsTrigger;
class CPipelineManagerEventsTrigger;
class CPipelineViewerEventsTrigger;
class CPipeline;
class CPipelineEngine;
class CPipelineViewer;

#define GetSingleObject(WaitForLock, obj) (WAIT_OBJECT_0 == ((WaitForLock) ? WaitForSingleObject(obj, INFINITE) :   WaitForSingleObject(obj, 0)))


class CPipelineEngineEvents
{
public:

	virtual HRESULT OnD3DDeviceCreate(){ return S_OK; };//on Adapter Change recovery
	virtual HRESULT OnD3DDeviceReset(){ return S_OK; };//on Device Lost recovery
	virtual HRESULT OnD3DDeviceLost(){ return S_OK; };//on Device Lost
	virtual HRESULT OnD3DDeviceDestroy(){ return S_OK; };//on Adapter Change
};

class CPipelineManagerEvents
{
public:
	CPipelineManagerEvents();
	~CPipelineManagerEvents();

	virtual HRESULT OnReload(){ return S_OK; };//expect that everything is new
	virtual HRESULT OnResourceAdd(IDXCCResource* pRes){ return S_OK; };
	virtual HRESULT OnResourceRecycle(IDXCCResource* pRes){ return S_OK; };
	virtual HRESULT OnResourceUpdate(IDXCCResource* pRes){ return S_OK; };
	virtual HRESULT OnResourceRemove(IDXCCResource* pRes){ return S_OK; };

private:
	HANDLE ExclusiveModeMutex;
};

class CPipelineViewerEvents
{
public:

	virtual HRESULT OnFrameChildAdded(IDXCCFrame* pParent, IDXCCFrame* pChild) { return S_OK; };
	virtual HRESULT OnFrameChildRemoved(IDXCCFrame* pParent, IDXCCFrame* pChild) { return S_OK; };

	virtual HRESULT OnFrameMemberAdded(IDXCCFrame* pParent, IUnknown* pChild) { return S_OK; };
	virtual HRESULT OnFrameMemberRemoved(IDXCCFrame* pParent, IUnknown* pChild) { return S_OK; };

	virtual HRESULT OnMeshChange(IDXCCMesh* pMesh){ return S_OK; };

	virtual HRESULT OnMeshDeclarationChange(IDXCCMesh* pMesh, IDXCCMesh* pOldMesh){ return S_OK; };
	//topology has change, regenerate mesh
	virtual HRESULT OnMeshTopologyChange(IDXCCMesh* pMesh, IDXCCMesh* pOldMesh){ return S_OK; };
	//vertices has changed so updates that range with the given Usage info
	virtual HRESULT OnMeshVertexChange(IDXCCMesh* pMesh, UINT vertMin, UINT vertMax){ return S_OK; };
	virtual HRESULT OnMeshSubVertexChange(IDXCCMesh* pMesh, D3DDECLUSAGE Usage, UINT UsageIndex, UINT vertMin, UINT vertMax){ return S_OK; };
	//the attributes used to associate a material to the mesh have changed
	virtual HRESULT OnMeshAttributeChange(IDXCCMesh* pMesh, UINT faceMin, UINT faceMax){ return S_OK; };
	//the materials associated to the attributes have changed
	virtual HRESULT OnMeshMaterialChange(IDXCCMesh* pMesh, DXCCATTRIBUTE Attrib, ID3DXEffect* pOldMaterial){ return S_OK; };
	//The material's Effect interface has changed (commonly used to unbind from Standard Semantics detabase)
	virtual HRESULT OnMaterialEffectChange(LPD3DXEFFECT pMaterial, LPD3DXEFFECT pOldEffect){ return S_OK; };
	//The material's Effect interface has changed (commonly used to unbind from Standard Semantics detabase)
	virtual HRESULT OnMaterialParameterChange(LPD3DXEFFECT pMaterial, D3DXHANDLE hParameter){ return S_OK; };
};

/*
enum
{
	DXCCMSG_GLOBAL_START,
	DXCCMSG_GLOBAL_PAUSE,
	DXCCMSG_GLOBAL_STOP,
	DXCCMSG_DEVICE_CREATE,
	DXCCMSG_DEVICE_RESET,
	DXCCMSG_DEVICE_LOST,
	DXCCMSG_MANAGER_ADD,
	DXCCMSG_MANAGER_REMOVE,
	DXCCMSG_MANAGER_REFRESH,
}

class CPipelineCallbacks
{
	void RegisterMessageToHwnd(HWND , WPARAM );
	void UnregisterMessageToHwnd(HWND , WPARAM);

	void RegisterMessageToThread(DWORD, WPARAM);
	void UnregisterMessageToThread(DWORD, WPARAM);

	//get the count of all threads and hwnds which have registered to recieve this message
	UINT GetDeliveryCount(WPARAM Msg);

	//This will post to the Window Message Queue of all registered recipients of WPARAM 
	//the DXCCMSG with WPARAM Msg and LPARAM Package
	//this will Addref #recipients so that the buffer is delete once everyone has recieved it
	//TODO change IDXCCHeap to inherit from ID3DXBuffer
	void PostMessage(WPARAM Msg, LPD3DXBUFFER Package);
	void SendMessage(WPARAM Msg);//COMMANDS ONLY like PAUSE or STOP

	//RecievePackage transfers the reference count to the user.
	//When all recipients recieve and release the package, 
	//the package should delete itself if the sender has also released it
	HRESULT RecievePackage(LPARAM, LPD3DXBUFFER* Package);

private:
	CAtlMap< WPARAM ,CAtlArray<HWND> > AddressBookOfHwnds;
	CAtlMap< WPARAM ,CAtlArray<DWORD> > AddressBookOfThreads;
	CAtlMap< LPARAM, LPD3DXBUFFER > Packages;
	HANDLE	OwnershipMutex; //all functions will acquire and release this.

}
*/


class CPipelineEngineEventsTrigger : public CPipelineEngineEvents
{
	std::vector<CPipelineEngineEvents*> Events;
public:

	HRESULT RegisterEvents(CPipelineEngineEvents* pCall);
	HRESULT UnregisterEvents(CPipelineEngineEvents* pCall);


	HRESULT OnD3DDeviceCreate();
	HRESULT OnD3DDeviceReset();
	HRESULT OnD3DDeviceLost();
	HRESULT OnD3DDeviceDestroy();
};

class CPipelineManagerEventsTrigger : public CPipelineManagerEvents
{
	std::vector<CPipelineManagerEvents*> Events;
public:

	HRESULT RegisterEvents(CPipelineManagerEvents* pCall);
	HRESULT UnregisterEvents(CPipelineManagerEvents* pCall);


	HRESULT OnReload(); 
	HRESULT OnResourceAdd(IDXCCResource* pRes);//EMITTED
	HRESULT OnResourceRecycle(IDXCCResource* pRes);//EMITTED
	HRESULT OnResourceRemove(IDXCCResource* pRes);
};

class CPipelineViewerEventsTrigger : public CPipelineViewerEvents
{
	std::vector<CPipelineViewerEvents*> Events;
public:
	
	HRESULT RegisterEvents(CPipelineViewerEvents* pCall);
	HRESULT UnregisterEvents(CPipelineViewerEvents* pCall);


	HRESULT OnFrameChildAdded(IDXCCFrame* pParent, IDXCCFrame* pChild);//EMITTED
	HRESULT OnFrameChildRemoved(IDXCCFrame* pParent, IDXCCFrame* pChild) ;//EMITTED
	HRESULT OnFrameMemberAdded(IDXCCFrame* pParent, IUnknown* pChild);//EMITTED
	HRESULT OnFrameMemberRemoved(IDXCCFrame* pParent, IUnknown* pChild);//EMITTED
	HRESULT OnMeshChange(IDXCCMesh* pMesh);//EMITTED
	HRESULT OnMeshDeclarationChange(IDXCCMesh* pMesh, IDXCCMesh* pOldMesh);
	HRESULT OnMeshTopologyChange(IDXCCMesh* pMesh, IDXCCMesh* pOldMesh);
	HRESULT OnMeshVertexChange(IDXCCMesh* pMesh, UINT vertMin, UINT vertMax);
	HRESULT OnMeshSubVertexChange(IDXCCMesh* pMesh, D3DDECLUSAGE Usage, UINT UsageIndex, UINT vertMin, UINT vertMax);
	HRESULT OnMeshAttributeChange(IDXCCMesh* pMesh, UINT faceMin, UINT faceMax);
	HRESULT OnMeshMaterialChange(IDXCCMesh* pMesh, DXCCATTRIBUTE Attrib, LPD3DXEFFECT pOldMaterial);
	HRESULT OnMaterialEffectChange(LPD3DXEFFECT pMaterial, LPD3DXEFFECT pOldEffect);
	HRESULT OnMaterialParameterChange(LPD3DXEFFECT pMaterial, D3DXHANDLE hParameter);
};


class CPipeline
{
	friend CPipelineLock;
public:

	virtual HRESULT Create();
	virtual HRESULT Destroy();

	virtual bool IsValid();

	virtual CPipelineEngine* AccessEngine();
	virtual LPDXCCMANAGER AccessManager();
	virtual LPDXCCFRAME AccessRoot();

    virtual float GetTime();
    virtual void SetTime(float time);


    virtual HRESULT SetEngine( CPipelineEngine* pPreviewEngine );


    virtual bool SceneWriteLock(BOOL WaitForLock, CPipelineLock& Lock); //returns true if you got the lock
	virtual void SceneWriteUnlock(CPipelineLock& Lock);

	virtual bool SceneReadLock(BOOL WaitForLock, CPipelineLock& Lock); //returns true if you got the lock
	virtual void SceneReadUnlock(CPipelineLock& Lock);

	CPipelineEngineEventsTrigger		TriggerDeviceEvents;
	CPipelineManagerEventsTrigger		TriggerManagerEvents;
	CPipelineViewerEventsTrigger		TriggerViewerEvents;

private:



//-------------------------------------------------------------------------------------
	
/* TODO Make device launcher part of PPCore
	void CreateDevice()
		{

			// Launch the thread which will create the device and check for device state changes
			m_hThread = _beginthreadex( NULL, 0, StaticRunThread, this, 0, &m_nThreadID );
			if( m_hThread == NULL )
				return E_FAIL;

			GetSingleObject(true, DeviceCreatedEvent);

			return S_OK;


		}

		static UINT __stdcall LaunchDeviceThread( void* pParam )
		{
			((CPipelineEngine*)pParam)->OnThreadInit();
			((CPipelineEngine*)pParam)->OnThreadRun();
		}


		UINT __stdcall CEngine::StaticRunThread( void* pParam )
		{
			CEngine* pDevice = (CEngine*) pParam;
			return pDevice->RunThread();
		}

*/




	//everyone must gain the Access to play fair
	//Write not release until unlock so that no reads come though
	//Read releases immediately to support multiple writes
	//Read incriments ReadCount on gain of AccessMutex and decriment on finishing
	//Read sets ZeroEvent when  ReadCount is zero and resets when non-zero
	//write waits on ZeroEvent after it has gained AccessMutex to ensure that reads are finished
	HANDLE	ReadWriteMutex;
	CRITICAL_SECTION ZeroSceneReadersSection; 
	UINT	SceneReadersCount;
	HANDLE	ZeroSceneReadersEvent;	
	
	CPipelineEngine* pPreviewEngine;
	LPDXCCMANAGER pDXCCManager;	
	LPDXCCFRAME pDXCCRoot;
	float fTime;



};


//we create a CPipelineEngine in it's own thread but you don't have to
//this prevents abnormal crashing caused by living in the same thread as OpenGL
class CPipelineEngine
{
public:
	virtual HRESULT Create(CPipeline* pPreviewPipeline) = 0;
    virtual HRESULT Destroy() = 0;

	virtual HRESULT GetD3DObject( IDirect3D9** ppObject ) = 0;
	virtual HRESULT GetD3DDevice( IDirect3DDevice9** ppDevice ) = 0;
};



//it is suggested that the viewers use StateBlocks to ensure that multiple viewers do not collide
class CPipelineViewer 
{
public:
	//get the device from the pPreviewPipeline and use CreateAdditionalSwapChain
	virtual HRESULT Create( CPipeline* pPreviewPipeline,	
							D3DPRESENT_PARAMETERS* pPresentationParameters,
							const WCHAR* strWindowTitle = L"Direct3D Preview Pipeline", 
							HINSTANCE hInstance = NULL, 
							HICON hIcon = NULL, 
							HMENU hMenu = NULL,
							int width = 640,
							int height = 480,
							int x = CW_USEDEFAULT, 
							int y = CW_USEDEFAULT) = 0;

    virtual HRESULT Destroy() = 0; //destroy thread

	virtual UINT Run() = 0; //allow core logic to start/continue
	virtual UINT GetPauseCount() = 0; //is the core logic running?
	virtual UINT Pause() = 0; //pause the core logic
	
    virtual HWND	GetRenderWindow() = 0; //get window this view will render to
    virtual HWND    GetShellWindow() = 0;//get the highest-level window you'd like docked inside a DCC
	virtual HRESULT	GetD3DSwapChain(IDirect3DSwapChain9** ppSwapChain) = 0;
};

class CPipelineLock
{
	friend CPipeline;
	friend CPipelineManagerEvents;
public:
	CPipelineLock(bool AutoUnlock= true);
	~CPipelineLock();

	long ofType(){ return Type; }
	bool isLocked(){ return Locked; }

private:
	enum
	{
		TYPE_NONE= 0,
		TYPE_SCENE_WRITE,
		TYPE_SCENE_READ,
		NUMBER_OF_TYPES
	}					Type;
	bool				Locked;
	bool				UnlockOnDestuction;
	CPipeline*	pPreviewPipeline;
};




class CBank
{
public:
	typedef DWORD GroupId;
	typedef DWORD ItemId;
	typedef CRBMap<ItemId , void* > CItemBank;
	typedef CRBMap<ItemId , GroupId > CItemToGroup;
	typedef CRBMultiMap<GroupId, ItemId> CGroupBank;

	CItemBank			ItemBank;
	CItemToGroup		ItemToGroup;
	CGroupBank			GroupBank;

	CBank();

	GroupId GenerateGroupId();
	CItemBank::CPair* AddItemToGroup(GroupId Gid, void* pValue);

	void RemoveByItem( ItemId Iid , bool DeleteValue);
	void RemoveByGroup( GroupId Gid, bool DeleteValues );

	void RemoveAll(bool DeleteValues);


protected:
	CBank(CBank& copyMe){};
	operator = (CBank& copyMe){};


	DWORD GroupGenerator;
	DWORD ItemGenerator;
	
};









#endif //PREVIEWPIPELINE_H