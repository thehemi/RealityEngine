//--------------------------------------------------------------------------------------
// File: PRTSimulator.h
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once


//--------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------
class CPRTSimulator
{
public:
	// TIM: Errors
	CHAR	strError[MAX_PATH];

	// We have to clone this structure, can't use one in xcustom.h because it lacks some key members
	// KEEP THIS IN SYNC!!!
	struct SIMULATOR_OPTIONS
	{
		void CopyFrom(::SIMULATOR_OPTIONS o)
		{
			dwNumRays			= o.dwNumRays;
			dwOrder				= o.dwOrder;
			dwNumChannels		= o.dwNumChannels;
			dwNumBounces		= o.dwNumBounces;
			bSubsurfaceScattering = o.bSubsurfaceScattering;
			fLengthScale		= o.fLengthScale;

			dwPredefinedMatIndex	= o.dwPredefinedMatIndex;
			Diffuse					= o.Diffuse;
			Absoption				= o.Absoption;
			ReducedScattering		= o.ReducedScattering;
			fRelativeIndexOfRefraction = o.fRelativeIndexOfRefraction;

			bAdaptive						= o.bAdaptive;
			bRobustMeshRefine				= o.bRobustMeshRefine;
			fRobustMeshRefineMinEdgeLength	= o.fRobustMeshRefineMinEdgeLength;
			dwRobustMeshRefineMaxSubdiv		= o.dwRobustMeshRefineMaxSubdiv;
			bAdaptiveDL						= o.bAdaptiveDL;
			fAdaptiveDLMinEdgeLength		= o.fAdaptiveDLMinEdgeLength;
			fAdaptiveDLThreshold			= o.fAdaptiveDLThreshold;
			dwAdaptiveDLMaxSubdiv			= o.dwAdaptiveDLMaxSubdiv;
			bAdaptiveBounce					= o.bAdaptiveBounce;
			fAdaptiveBounceMinEdgeLength	= o.fAdaptiveBounceMinEdgeLength;
			fAdaptiveBounceThreshold		= o.fAdaptiveBounceThreshold;
			dwAdaptiveBounceMaxSubdiv		 = o.dwAdaptiveBounceMaxSubdiv;

			dwNumClusters	= o.dwNumClusters;
			dwNumPCA		= o.dwNumPCA;
			bPerPixel		= o.bPerPixel;
			dwTextureSize	= o.dwTextureSize;
		}

		// General settings
		WCHAR     strInitialDir[MAX_PATH];
		WCHAR     strInputMesh[MAX_PATH];
		WCHAR     strResultsFile[MAX_PATH];
		DWORD     dwNumRays;
		DWORD     dwOrder;
		DWORD     dwNumChannels;
		DWORD     dwNumBounces;
		bool      bSubsurfaceScattering;
		float     fLengthScale;
		bool      bShowTooltips;

		// Material options
		DWORD     dwPredefinedMatIndex;
		D3DXCOLOR Diffuse;
		D3DXCOLOR Absoption;
		D3DXCOLOR ReducedScattering;
		float     fRelativeIndexOfRefraction;

		// Adaptive options
		bool      bAdaptive;
		bool      bRobustMeshRefine;
		float     fRobustMeshRefineMinEdgeLength;
		DWORD     dwRobustMeshRefineMaxSubdiv;
		bool      bAdaptiveDL;
		float     fAdaptiveDLMinEdgeLength;
		float     fAdaptiveDLThreshold;
		DWORD     dwAdaptiveDLMaxSubdiv;
		bool      bAdaptiveBounce;
		float     fAdaptiveBounceMinEdgeLength;
		float     fAdaptiveBounceThreshold;
		DWORD     dwAdaptiveBounceMaxSubdiv;
		WCHAR     strOutputMesh[MAX_PATH];
		bool      bBinaryOutputXFile;
	    
		// Compression options
		bool      bSaveCompressedResults;
		D3DXSHCOMPRESSQUALITYTYPE Quality;
		DWORD     dwNumClusters;
		DWORD     dwNumPCA;
		bool	  bPerPixel;	
		DWORD     dwTextureSize;
	};

    CPRTSimulator(void);
    ~CPRTSimulator(void);

	// TIM: Blocker
    HRESULT Run( IDirect3DDevice9* pd3dDevice, struct SIMULATOR_OPTIONS* pOptions, CPRTMesh* pPRTMesh, ID3DXMesh* blocker );
    bool    IsRunning();
    HRESULT Stop();

    float   GetPercentComplete() { return m_fPercentDone * 100.0f; }
    int     GetCurrentPass() { return m_nCurPass; }
    WCHAR*  GetCurrentPassName() { return m_strCurPass; }
    int     GetNumPasses() { return m_nNumPasses; }

    static DWORD WINAPI StaticPRTSimulationThreadProc( LPVOID lpParameter );
    DWORD PRTSimulationThreadProc();

	HRESULT CPRTSimulator::CreateBuffer(UINT NumSamples,UINT NumCoeffs,UINT NumChannels,LPD3DXPRTBUFFER *ppBuffer);

public:
    static HRESULT WINAPI StaticPRTSimulatorCB( float fPercentDone, LPVOID pParam );
    HRESULT PRTSimulatorCB( float fPercentDone );

    CRITICAL_SECTION m_cs;

    SIMULATOR_OPTIONS m_Options;
    CPRTMesh* m_pPRTMesh;
    int m_nCurPass;
    int m_nNumPasses;
    WCHAR m_strCurPass[256];

	// TIM: BLOCKER
	ID3DXMesh*		m_pBlockerMesh;
    ID3DXPRTEngine* m_pPRTEngine;
    IDirect3DDevice9* m_pd3dDevice;

    bool m_bStopSimulator;
    bool m_bRunning;
    bool m_bFailed;

    HANDLE m_hThreadId;
    DWORD  m_dwThreadId;
    float  m_fPercentDone;
};
