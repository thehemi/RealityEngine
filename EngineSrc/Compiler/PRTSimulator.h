//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Runs Precomputed Radiance Transfer Calculations, Per-Pixel or Per-Vertex
///
///
/// Author: Tim Johnson (Based on Microsoft's PRTSimulator class)
//====================================================================================
#pragma once

//--------------------------------------------------------------------------------------
/// Precomputed radiance transfer engine
//--------------------------------------------------------------------------------------
class CPRTSimulator
{
public:
	/// TIM: Errors
	CHAR	strError[MAX_PATH];

    CPRTSimulator(void);
    ~CPRTSimulator(void);

	/// TIM: Blocker
    HRESULT Run( IDirect3DDevice9* pd3dDevice, struct PRTSettings* pOptions, CPRTMesh* pPRTMesh, ID3DXMesh* blocker );
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

    PRTSettings m_Options;
    CPRTMesh* m_pPRTMesh;
    int m_nCurPass;
    int m_nNumPasses;
    WCHAR m_strCurPass[256];

	/// TIM: BLOCKER
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
