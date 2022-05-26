//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Runs Precomputed Radiance Transfer Calculations, Per-Pixel or Per-Vertex
///
///
/// Author: Tim Johnson (Based on Microsoft's PRTSimulator class)
//====================================================================================
#include "dxstdafx.h"
#include "stdafx.h"
#include "prtmesh.h"

#include <string>
#include <vector>
using namespace std;
#include "sharedstructures.h"
//#include "..\MaxPlugin\ExportLib\xcustom.h"
#include "prtsimulator.h"
//#include "prtoptionsdlg.h"


//--------------------------------------------------------------------------------------
// Subsurface scattering parameters from:
// "A Practical Model for Subsurface Light Transport", 
// Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan.
// SIGGRAPH 2001
//--------------------------------------------------------------------------------------
/*const PREDEFINED_MATERIALS g_aPredefinedMaterials[] = 
{
    // name             scattering (R/G/B/A)            absorption (R/G/B/A)                    reflectance (R/G/B/A)           index of refraction
    TEXT("Default"),    {2.00f, 2.00f, 2.00f, 1.0f},    {0.0030f, 0.0030f, 0.0460f, 1.0f},      {1.00f, 1.00f, 1.00f, 1.0f},    1.3f,
    TEXT("Apple"),      {2.29f, 2.39f, 1.97f, 1.0f},    {0.0030f, 0.0030f, 0.0460f, 1.0f},      {0.85f, 0.84f, 0.53f, 1.0f},    1.3f,
    TEXT("Chicken1"),   {0.15f, 0.21f, 0.38f, 1.0f},    {0.0150f, 0.0770f, 0.1900f, 1.0f},      {0.31f, 0.15f, 0.10f, 1.0f},    1.3f,
    TEXT("Chicken2"),   {0.19f, 0.25f, 0.32f, 1.0f},    {0.0180f, 0.0880f, 0.2000f, 1.0f},      {0.32f, 0.16f, 0.10f, 1.0f},    1.3f,
    TEXT("Cream"),      {7.38f, 5.47f, 3.15f, 1.0f},    {0.0002f, 0.0028f, 0.0163f, 1.0f},      {0.98f, 0.90f, 0.73f, 1.0f},    1.3f,
    TEXT("Ketchup"),    {0.18f, 0.07f, 0.03f, 1.0f},    {0.0610f, 0.9700f, 1.4500f, 1.0f},      {0.16f, 0.01f, 0.00f, 1.0f},    1.3f,
    TEXT("Marble"),     {2.19f, 2.62f, 3.00f, 1.0f},    {0.0021f, 0.0041f, 0.0071f, 1.0f},      {0.83f, 0.79f, 0.75f, 1.0f},    1.5f,
    TEXT("Potato"),     {0.68f, 0.70f, 0.55f, 1.0f},    {0.0024f, 0.0090f, 0.1200f, 1.0f},      {0.77f, 0.62f, 0.21f, 1.0f},    1.3f,
    TEXT("Skimmilk"),   {0.70f, 1.22f, 1.90f, 1.0f},    {0.0014f, 0.0025f, 0.0142f, 1.0f},      {0.81f, 0.81f, 0.69f, 1.0f},    1.3f,
    TEXT("Skin1"),      {0.74f, 0.88f, 1.01f, 1.0f},    {0.0320f, 0.1700f, 0.4800f, 1.0f},      {0.44f, 0.22f, 0.13f, 1.0f},    1.3f,
    TEXT("Skin2"),      {1.09f, 1.59f, 1.79f, 1.0f},    {0.0130f, 0.0700f, 0.1450f, 1.0f},      {0.63f, 0.44f, 0.34f, 1.0f},    1.3f,
    TEXT("Spectralon"),{11.60f,20.40f,14.90f, 1.0f},    {0.0000f, 0.0000f, 0.0000f, 1.0f},      {1.00f, 1.00f, 1.00f, 1.0f},    1.3f,
    TEXT("Wholemilk"),  {2.55f, 3.21f, 3.77f, 1.0f},    {0.0011f, 0.0024f, 0.0140f, 1.0f},      {0.91f, 0.88f, 0.76f, 1.0f},    1.3f,
    TEXT("Custom"),     {0.00f, 0.00f, 0.00f, 1.0f},    {0.0000f, 0.0000f, 0.0000f, 1.0f},      {0.00f, 0.00f, 0.00f, 1.0f},    0.0f,
};
const int g_aPredefinedMaterialsSize = sizeof(g_aPredefinedMaterials) / sizeof(g_aPredefinedMaterials[0]);
*/

CPRTSimulator* g_pSimulator;

void LogPrintf(const char *fmt, ...);
#undef V
#define V(x)           { hr = x; if( FAILED(hr) ) { /*DXUTTrace( __FILE__, (DWORD)__LINE__, hr, L#x, true );*/ \
sprintf(strError, "Error: %s, in: "###x,DXGetErrorString9A(hr)); \
	goto LEarlyExit; \
} } 

//LogPrintf(" Error: %s, in: "###x,DXGetErrorString9(hr)); \
//--------------------------------------------------------------------------------------
CPRTSimulator::CPRTSimulator(void)
{
    m_nCurPass = 1;
    m_nNumPasses = 1;
    g_pSimulator = this;
    InitializeCriticalSection( &m_cs );   

    m_bStopSimulator = false;
    m_bRunning = false;
    m_bFailed = false;
    m_pPRTEngine = NULL;

    m_hThreadId = NULL; 
    m_dwThreadId = 0;
    m_fPercentDone = 0.0f;
    wcscpy( m_strCurPass, L"" );
}


//--------------------------------------------------------------------------------------
CPRTSimulator::~CPRTSimulator(void)
{
    DeleteCriticalSection( &m_cs );  
    SAFE_RELEASE( m_pPRTEngine );
}


//--------------------------------------------------------------------------------------
// TIM: BLOCKER MESH
HRESULT CPRTSimulator::Run( IDirect3DDevice9* pd3dDevice, PRTSettings* pOptions, CPRTMesh* pPRTMesh, ID3DXMesh* blockerMesh )
{
    if( IsRunning() ) 
        return E_FAIL;

	m_Options = *pOptions;
    m_pd3dDevice = pd3dDevice;
    m_pPRTMesh = pPRTMesh;
	// TIM: BLOCKER MESH
	m_pBlockerMesh = blockerMesh;

    m_bRunning = true;
    m_bFailed = false;
    m_bStopSimulator = false;
    m_fPercentDone = 0.0f;

    // Launch the PRT simulator on another thread cause it'll 
    // likely take a while and the UI would be unresponsive otherwise
    m_hThreadId = CreateThread( NULL, 0, StaticPRTSimulationThreadProc, 
                                this, 0, &m_dwThreadId );

    return S_OK;
}


//--------------------------------------------------------------------------------------
bool CPRTSimulator::IsRunning()
{
    if( m_hThreadId )
    {
        // Ask to stop the PRT simulator if it's running in the other thread
        DWORD dwResult = WaitForSingleObject( m_hThreadId, 0 );
        if( dwResult == WAIT_TIMEOUT )
            return true;
    }
    return false;
}


//--------------------------------------------------------------------------------------
HRESULT CPRTSimulator::Stop()
{
    if( IsRunning() )
    {
        EnterCriticalSection( &m_cs );
        m_bStopSimulator = true;
        LeaveCriticalSection( &m_cs );

        // Wait for it to close
        DWORD dwResult = WaitForSingleObject( m_hThreadId, 10000 );
        if( dwResult == WAIT_TIMEOUT )
            return E_FAIL;

        m_bStopSimulator = false;
        m_hThreadId = NULL;
        m_dwThreadId = 0;
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// static helper function
//-----------------------------------------------------------------------------
DWORD WINAPI CPRTSimulator::StaticPRTSimulationThreadProc( LPVOID lpParameter )
{   
    CPRTSimulator* pSim = (CPRTSimulator*)lpParameter;
    return pSim->PRTSimulationThreadProc();
}

//-----------------------------------------------------------------------------
// TIM: To switch between pixel and vertex buffers
//-----------------------------------------------------------------------------
HRESULT CPRTSimulator::CreateBuffer(UINT NumSamples,
    UINT NumCoeffs,
	UINT NumChannels,
	LPD3DXPRTBUFFER *ppBuffer
	)
{

	HRESULT hr;
	if(/*m_Options.bSubsurfaceScattering ||*/ !m_Options.bPerPixel)
		hr=D3DXCreatePRTBuffer( NumSamples, NumCoeffs, NumChannels, ppBuffer );
	else
		hr=D3DXCreatePRTBufferTex ( m_Options.dwTextureSize, m_Options.dwTextureSize, NumCoeffs, NumChannels, ppBuffer );

	return hr;
}

D3DXCOLOR AsD3DX(FloatColor& c)
{
	D3DXCOLOR x;
	x.a = c.a;
	x.b = c.b;
	x.g = c.g;
	x.r = c.r;
	return x;
}

//-----------------------------------------------------------------------------
// Load the mesh and start the simluator and save the results to a file
//-----------------------------------------------------------------------------
DWORD CPRTSimulator::PRTSimulationThreadProc()
{
    HRESULT hr;

    // Reset precent complete
    m_fPercentDone = 0.0f;

    if( !m_pPRTMesh->IsMeshLoaded() )    
        return 1;

    ID3DXPRTBuffer* pDataTotal = NULL;
    ID3DXPRTBuffer* pBufferA = NULL;
    ID3DXPRTBuffer* pBufferB = NULL;
    D3DXSHMATERIAL* pMatPtr = NULL;

    m_nNumPasses = m_Options.dwNumBounces;
    if( m_Options.bSubsurfaceScattering )
        m_nNumPasses *= 2;
    if( m_Options.bAdaptive && m_Options.bRobustMeshRefine )
        m_nNumPasses++;

    m_nNumPasses += 2;

    m_nCurPass = 1;
    m_fPercentDone = -1.0f;
    wcscpy( m_strCurPass, L"Initializing PRT engine" );

    ID3DXMesh* pMesh = m_pPRTMesh->GetMesh();

    bool bExtractUVs = false;
    if( m_Options.bAdaptive && m_pPRTMesh->GetAlbedoTexture() )
        bExtractUVs = true;

	// TIM: Added
	if( m_Options.bPerPixel /*&& !m_Options.bSubsurfaceScattering*/ )
		bExtractUVs = true;

    // Debuggign info
	int numVerts = pMesh->GetNumVertices();
   /// int numBlockerVerts = m_pBlockerMesh->GetNumVertices();
  //  bool s32 = pMesh->GetOptions() & D3DXMESH_32BIT;
  //  bool d32 = m_pBlockerMesh->GetOptions() & D3DXMESH_32BIT;

	LogPrintf("[Compile] Init PRT");
    V( D3DXCreatePRTEngine( pMesh,0, bExtractUVs, m_pBlockerMesh, &m_pPRTEngine ) );
	LogPrintf("[Compile] SetInfo()");
    V( m_pPRTEngine->SetCallBack( StaticPRTSimulatorCB, 0.001f, NULL ) );
    V( m_pPRTEngine->SetSamplingInfo( m_Options.dwNumRays, FALSE, TRUE, FALSE, 0.0f ) );
 
    if( m_Options.bAdaptive && m_pPRTMesh->GetAlbedoTexture() )
    {
        V( m_pPRTEngine->SetPerTexelAlbedo( m_pPRTMesh->GetAlbedoTexture(), 
                                            m_Options.dwNumChannels, NULL ) );
    }

	if( m_pPRTMesh->m_pNormalMap )
		V( m_pPRTEngine->SetPerTexelNormal(m_pPRTMesh->m_pNormalMap));
	LogPrintf("[Compile] SetMats");
    // Note that the alpha value is ignored for the Diffuse, Absorption, 
    // and ReducedScattering parameters of the material.
    D3DXSHMATERIAL shMat[1];
    ZeroMemory( &shMat[0], sizeof(D3DXSHMATERIAL) );
    shMat[0].Diffuse = AsD3DX(m_Options.Diffuse);
    shMat[0].bMirror = false;
    shMat[0].bSubSurf = m_Options.bSubsurfaceScattering;
    shMat[0].RelativeIndexOfRefraction  = m_Options.fRelativeIndexOfRefraction;
    shMat[0].Absorption = AsD3DX(m_Options.Absoption);
    shMat[0].ReducedScattering = AsD3DX(m_Options.ReducedScattering);

    DWORD dwNumMeshes = 0;
    V( pMesh->GetAttributeTable(NULL,&dwNumMeshes) );

    // This sample treats all subsets as having the same 
    // material properties but they don't have too
    D3DXMATERIAL* pd3dxMaterial = m_pPRTMesh->GetMaterials();
    pMatPtr = new D3DXSHMATERIAL[dwNumMeshes];
    for( DWORD i=0; i<dwNumMeshes; ++i )
    {
        ZeroMemory( &pMatPtr[i], sizeof(D3DXSHMATERIAL) );
        pMatPtr[i].Diffuse = AsD3DX(m_Options.Diffuse);
        pMatPtr[i].bMirror = false;
        pMatPtr[i].bSubSurf = m_Options.bSubsurfaceScattering;
        pMatPtr[i].RelativeIndexOfRefraction  = m_Options.fRelativeIndexOfRefraction;
        pMatPtr[i].Absorption = AsD3DX(m_Options.Absoption);
        pMatPtr[i].ReducedScattering = AsD3DX(m_Options.ReducedScattering);

		// TIM: NULL Material guard
		if(pd3dxMaterial)
			pMatPtr[i].Diffuse = pd3dxMaterial[i].MatD3D.Diffuse;
    }
 
    D3DXSHMATERIAL** pMatPtrArray = new D3DXSHMATERIAL*[dwNumMeshes];
    for( DWORD i=0; i<dwNumMeshes; ++i )
    {
        pMatPtrArray[i] = &pMatPtr[i];
    }

    bool bSetAlbedoFromMaterial = true;
    if( m_Options.bAdaptive && m_pPRTMesh->GetAlbedoTexture() )
        bSetAlbedoFromMaterial = false;

    V( m_pPRTEngine->SetMeshMaterials( (const D3DXSHMATERIAL**)pMatPtrArray, dwNumMeshes, 
                                       m_Options.dwNumChannels, 
                                       bSetAlbedoFromMaterial, m_Options.fLengthScale ) );

	LogPrintf("[Compile] Generating");
    if( !m_Options.bSubsurfaceScattering )
    {
        // Not doing subsurface scattering

        if( m_Options.bAdaptive && m_Options.bRobustMeshRefine ) 
        {
            m_nCurPass++;
            m_fPercentDone = -1.0f;
            wcscpy( m_strCurPass, L"Robust Mesh Refine" );
            V( m_pPRTEngine->RobustMeshRefine( m_Options.fRobustMeshRefineMinEdgeLength, m_Options.dwRobustMeshRefineMaxSubdiv ) );
        }

        DWORD dwNumSamples = m_pPRTEngine->GetNumVerts();
        V( CreateBuffer( dwNumSamples, m_Options.dwOrder*m_Options.dwOrder, 
                                m_Options.dwNumChannels, &pDataTotal ) );

        m_nCurPass++;
        wcscpy( m_strCurPass, L"Computing Direct Lighting" );
        m_fPercentDone = 0.0f;
        if( m_Options.bAdaptive && m_Options.bAdaptiveDL )
        {
            hr = m_pPRTEngine->ComputeDirectLightingSHAdaptive( m_Options.dwOrder, 
                                                                m_Options.fAdaptiveDLThreshold, m_Options.fAdaptiveDLMinEdgeLength, m_Options.dwAdaptiveDLMaxSubdiv, 
                                                                pDataTotal );
            if( FAILED(hr ) )
                goto LEarlyExit; // handle user aborting simulator via callback 
        }
        else
        {
            hr = m_pPRTEngine->ComputeDirectLightingSH( m_Options.dwOrder, pDataTotal );
            if( FAILED(hr ) )
                goto LEarlyExit; // handle user aborting simulator via callback 
        }

        if( m_Options.dwNumBounces > 1 )
        {
            dwNumSamples = m_pPRTEngine->GetNumVerts();
            V( CreateBuffer( dwNumSamples, m_Options.dwOrder*m_Options.dwOrder, 
                                    m_Options.dwNumChannels, &pBufferA ) );
            V( CreateBuffer( dwNumSamples, m_Options.dwOrder*m_Options.dwOrder, 
                                    m_Options.dwNumChannels, &pBufferB ) );
            V( pBufferA->AddBuffer( pDataTotal ) );
        }

        for( UINT iBounce=1; iBounce<m_Options.dwNumBounces; ++iBounce )
        {
            m_nCurPass++;
            swprintf( m_strCurPass, L"Computing Bounce %d Lighting", iBounce+1 );
            m_fPercentDone = 0.0f;
            if( m_Options.bAdaptive && m_Options.bAdaptiveBounce )
                hr = m_pPRTEngine->ComputeBounceAdaptive( pBufferA, m_Options.fAdaptiveBounceThreshold, m_Options.fAdaptiveBounceMinEdgeLength, m_Options.dwAdaptiveBounceMaxSubdiv, pBufferB, pDataTotal );
            else
                hr = m_pPRTEngine->ComputeBounce( pBufferA, pBufferB, pDataTotal );

            if( FAILED(hr ) )
                goto LEarlyExit; // handle user aborting simulator via callback 

            // Swap pBufferA and pBufferB
            ID3DXPRTBuffer* pPRTBufferTemp = NULL;
            pPRTBufferTemp = pBufferA;
            pBufferA = pBufferB;
            pBufferB = pPRTBufferTemp;
        }

        if( m_Options.bAdaptive )
        {
			UINT numVerts = m_pPRTEngine->GetNumVerts();
			UINT* vertRemap = new UINT[numVerts*3];
			FLOAT* vertWeights = new FLOAT[numVerts*3];
            V( m_pPRTEngine->GetAdaptedMesh( m_pd3dDevice, NULL, vertRemap, vertWeights, &pMesh ) );
			m_pPRTMesh->SetAdaptiveMesh(m_pd3dDevice,m_pPRTMesh->m_pMesh,pMesh,vertRemap,vertWeights);

			delete[] vertRemap;
			delete[] vertWeights;
        }

        SAFE_RELEASE( pBufferA );
        SAFE_RELEASE( pBufferB );
    }
    else
    {
        // Doing subsurface scattering

        if( m_Options.bAdaptive && m_Options.bRobustMeshRefine ) 
            V( m_pPRTEngine->RobustMeshRefine( m_Options.fRobustMeshRefineMinEdgeLength, m_Options.dwRobustMeshRefineMaxSubdiv ) );

        DWORD dwNumSamples = m_pPRTEngine->GetNumVerts();
        V( CreateBuffer( dwNumSamples, m_Options.dwOrder*m_Options.dwOrder, 
                                m_Options.dwNumChannels, &pBufferA ) );
        V( CreateBuffer( dwNumSamples, m_Options.dwOrder*m_Options.dwOrder, 
                                m_Options.dwNumChannels, &pBufferB ) );
        V( CreateBuffer( dwNumSamples, m_Options.dwOrder*m_Options.dwOrder, 
                                m_Options.dwNumChannels, &pDataTotal ) );

        m_nCurPass = 1;
        wcscpy( m_strCurPass, L"Computing Direct Lighting" );
        m_fPercentDone = 0.0f;
        if( m_Options.bAdaptive && m_Options.bAdaptiveDL )
        {
            hr = m_pPRTEngine->ComputeDirectLightingSHAdaptive( m_Options.dwOrder, 
                                                                m_Options.fAdaptiveDLThreshold, m_Options.fAdaptiveDLMinEdgeLength, m_Options.dwAdaptiveDLMaxSubdiv, 
                                                                pBufferA );
            if( FAILED(hr ) )
                goto LEarlyExit; // handle user aborting simulator via callback 
        }
        else
        {
            hr = m_pPRTEngine->ComputeDirectLightingSH( m_Options.dwOrder, pBufferA );
            if( FAILED(hr ) )
                goto LEarlyExit; // handle user aborting simulator via callback 
        }

		// Convert per-pixel buf to per-vertex buf
		LPD3DXPRTBUFFER prtVertexA, prtVertexB;
		if(m_Options.bPerPixel){
			V( D3DXCreatePRTBuffer( m_pPRTEngine->GetNumVerts(), m_Options.dwOrder*m_Options.dwOrder, 
									m_Options.dwNumChannels, &prtVertexA));
			V(m_pPRTEngine->ResampleBuffer(pBufferA,prtVertexA));


			// Create output per-vertex buf
			V( D3DXCreatePRTBuffer( m_pPRTEngine->GetNumVerts(), m_Options.dwOrder*m_Options.dwOrder, 
									m_Options.dwNumChannels, &prtVertexB));

				// Run!
			m_nCurPass++;
			wcscpy( m_strCurPass, L"Computing Subsurface Direct Lighting" );
			hr = m_pPRTEngine->ComputeSS( prtVertexA, prtVertexB, 0 );
			if( FAILED(hr ) )
				goto LEarlyExit; // handle user aborting simulator via callback 

			// PerVertex->PerPixel
			V(m_pPRTEngine->ResampleBuffer(prtVertexB,pBufferB));
			pDataTotal->AddBuffer(pBufferB);

			SAFE_RELEASE(prtVertexA);
			SAFE_RELEASE(prtVertexB);
		}
		else{
			// Run!
			m_nCurPass++;
			wcscpy( m_strCurPass, L"Computing Subsurface Direct Lighting" );
			hr = m_pPRTEngine->ComputeSS( pBufferA, pBufferB, pDataTotal );
			if( FAILED(hr ) )
				goto LEarlyExit; // handle user aborting simulator via callback 
		}

			
        for( UINT iBounce=1; iBounce<m_Options.dwNumBounces; ++iBounce )
        {
            m_nCurPass++;
            swprintf( m_strCurPass, L"Computing Bounce %d Lighting", iBounce+1 );
            m_fPercentDone = 0.0f;
            if( m_Options.bAdaptive && m_Options.bAdaptiveBounce )
                hr = m_pPRTEngine->ComputeBounceAdaptive( pBufferB, m_Options.fAdaptiveBounceThreshold, m_Options.fAdaptiveBounceMinEdgeLength, m_Options.dwAdaptiveBounceMaxSubdiv, pBufferA, NULL );
            else
                hr = m_pPRTEngine->ComputeBounce( pBufferB, pBufferA, NULL );

            if( FAILED(hr ) )
                goto LEarlyExit; // handle user aborting simulator via callback 

            m_nCurPass++;

			//
			// TIM: FIXME: Support per-pixel SSS bounces
			//
			swprintf( m_strCurPass, L"Computing Subsurface Bounce %d Lighting", iBounce+1 );


			if(m_Options.bPerPixel){
				V( D3DXCreatePRTBuffer( m_pPRTEngine->GetNumVerts(), m_Options.dwOrder*m_Options.dwOrder, 
										m_Options.dwNumChannels, &prtVertexA));
				V(m_pPRTEngine->ResampleBuffer(pBufferB,prtVertexA));


				// Create output per-vertex buf
				V( D3DXCreatePRTBuffer( m_pPRTEngine->GetNumVerts(), m_Options.dwOrder*m_Options.dwOrder, 
										m_Options.dwNumChannels, &prtVertexB));

				// Run!
				hr = m_pPRTEngine->ComputeSS( prtVertexA, prtVertexB, 0 );
				if( FAILED(hr ) )
					goto LEarlyExit; // handle user aborting simulator via callback 

				// PerVertex->PerPixel
				V(m_pPRTEngine->ResampleBuffer(prtVertexB,pBufferA));
				pDataTotal->AddBuffer(pBufferB);

				SAFE_RELEASE(prtVertexA);
				SAFE_RELEASE(prtVertexB);
			}
			else{
				hr = m_pPRTEngine->ComputeSS( pBufferB, pBufferA, pDataTotal );
				if( FAILED(hr ) )
					goto LEarlyExit; // handle user aborting simulator via callback 
			}
        }

        if( m_Options.bAdaptive )
        {
           	UINT numVerts = m_pPRTEngine->GetNumVerts();
			UINT* vertRemap = new UINT[numVerts*3];
			FLOAT* vertWeights = new FLOAT[numVerts*3];
            V( m_pPRTEngine->GetAdaptedMesh( m_pd3dDevice, NULL, vertRemap, vertWeights, &pMesh ) );
			m_pPRTMesh->SetAdaptiveMesh(m_pd3dDevice,m_pPRTMesh->m_pMesh,pMesh,vertRemap,vertWeights);

			delete[] vertRemap;
			delete[] vertWeights;;
        }

        SAFE_RELEASE( pBufferA );
        SAFE_RELEASE( pBufferB );
    }

	LogPrintf("[Compile] Compressing");
    m_nCurPass++;
    wcscpy( m_strCurPass, L"Compressing Buffer" );
    m_fPercentDone = -1.0f;

	if(m_Options.bSubsurfaceScattering)
		pDataTotal->ScaleBuffer(m_Options.fSubSurfaceMultiplier);

	/*LPD3DXTEXTUREGUTTERHELPER  helper;
	D3DXCreateTextureGutterHelper (pDataTotal->GetHeight(),pDataTotal->GetWidth(),m_pPRTMesh->m_pMesh,5.0,&helper);
	pDataTotal->ReleaseGH();
	pDataTotal->AttachGH(helper);
	pDataTotal->EvalGH();*/

    SetCurrentDirectory( m_Options.strInitialDir );
    m_pPRTMesh->SetPRTBuffer( pDataTotal, m_Options.strResultsFile );
    m_pPRTMesh->CompressBuffer( m_Options.Quality, 1, 24 );

    if( m_Options.bSaveCompressedResults )
    {
        ID3DXPRTCompBuffer* pCompBuffer = m_pPRTMesh->GetCompBuffer();
        V( D3DXSavePRTCompBufferToFile( m_Options.strResultsFile, pCompBuffer ) );        
    }
    else
    {
        V( D3DXSavePRTBufferToFile( m_Options.strResultsFile, pDataTotal ) );
    }
	// TIM: Don't do this
    //m_pPRTMesh->ExtractCompressedDataForPRTShader(m_pd3dDevice);

	LogPrintf("[Compile] Done");
    m_bRunning = false;
    m_fPercentDone = 1.0f;

    SAFE_RELEASE( m_pPRTEngine );
    SAFE_DELETE_ARRAY( pMatPtr );

    return 1;

LEarlyExit:

    // Usually fails becaused user stoped the simulator
    m_bFailed = true;
    m_bRunning = false;
    SAFE_RELEASE( m_pPRTEngine );
    SAFE_RELEASE( pBufferA );
    SAFE_RELEASE( pBufferB );
    SAFE_RELEASE( pDataTotal );
    SAFE_DELETE_ARRAY( pMatPtr );

    // It returns E_FAIL if the simulation was aborted from the callback
    if( hr == E_FAIL ) 
        return 0;

    DXTRACE_ERR( TEXT("D3DXSHPRTSimulation"), hr );
    return 1;
}


//-----------------------------------------------------------------------------
// static helper function
//-----------------------------------------------------------------------------
HRESULT WINAPI CPRTSimulator::StaticPRTSimulatorCB( float fPercentDone, LPVOID pParam )
{
    return g_pSimulator->PRTSimulatorCB( fPercentDone );
}


//-----------------------------------------------------------------------------
// records the percent done and stops the simulator if requested
//-----------------------------------------------------------------------------
HRESULT CPRTSimulator::PRTSimulatorCB( float fPercentDone )
{
    EnterCriticalSection( &m_cs );
    m_fPercentDone = fPercentDone;

    HRESULT hr = S_OK;
    // In this callback, returning anything except S_OK will stop the simulator
    if( m_bStopSimulator )
        hr = E_FAIL; 

    LeaveCriticalSection( &m_cs );

    return hr;
}


