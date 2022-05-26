//--------------------------------------------------------------------------------------
// File: PRTOptionsDlg.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "dxstdafx.h"
#include <msxml.h>
#include <oleauto.h>
#include <commdlg.h>
#include "prtmesh.h"
//#include "prtsimulator.h"
#include <string>
#include <vector>
#include <rmxfguid.h>
using namespace std;
#include "..\Shared\Shared.h"
#include "..\EngineInc\SharedStructures.h"
#include "prtoptionsdlg.h"
#include "..\Max HLSL\libs\src\nv_gui\resource.h"

extern HINSTANCE g_hInstance;

#define MAX_PCA_VECTORS 24
COptionsFile g_OptionsFile;
CPRTOptionsDlg* g_pDlg = NULL;
CPRTLoadDlg* g_pLoadDlg = NULL;
CPRTAdaptiveOptionsDlg* g_pAdaptiveDlg = NULL;

SIMULATOR_OPTIONS& GetGlobalOptions()
{
    return g_OptionsFile.m_Options;
}

COptionsFile& GetGlobalOptionsFile()
{
    return g_OptionsFile;
}


//--------------------------------------------------------------------------------------
// Display error msg box to help debug 
//--------------------------------------------------------------------------------------
HRESULT WINAPI DXUTTrace( const CHAR* strFile, DWORD dwLine, HRESULT hr,
                          const WCHAR* strMsg, bool bPopMsgBox )
{
    bool bShowMsgBoxOnError = true;
    if( bPopMsgBox && bShowMsgBoxOnError == false )
        bPopMsgBox = false;

    return DXTrace( strFile, dwLine, hr, strMsg, bPopMsgBox );
}


//--------------------------------------------------------------------------------------
// Struct to store material params
//--------------------------------------------------------------------------------------
struct PREDEFINED_MATERIALS
{
    const TCHAR* strName;
    D3DCOLORVALUE ReducedScattering;
    D3DCOLORVALUE Absoption;
    D3DCOLORVALUE Diffuse;
    float fRelativeIndexOfRefraction;
};


//--------------------------------------------------------------------------------------
// Subsurface scattering parameters from:
// "A Practical Model for Subsurface Light Transport", 
// Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan.
// SIGGRAPH 2001
//--------------------------------------------------------------------------------------
const PREDEFINED_MATERIALS g_aPredefinedMaterials[] = 
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


//--------------------------------------------------------------------------------------
CPRTOptionsDlg::CPRTOptionsDlg(void)
{
    m_hDlg = NULL;
    g_pDlg = this;
    m_hToolTip = NULL;
    m_hMsgProcHook = NULL;
    m_hDlg = NULL;
    m_bShowTooltips = true;
    m_bComboBoxSelChange = false;
}


//--------------------------------------------------------------------------------------
CPRTOptionsDlg::~CPRTOptionsDlg(void)
{
    g_pDlg = NULL;
}


//--------------------------------------------------------------------------------------
HRESULT CPRTOptionsDlg::SaveOptions( WCHAR* strFile )
{
    return g_OptionsFile.SaveOptions( strFile );
}


//--------------------------------------------------------------------------------------
COptionsFile::COptionsFile()
{
    ResetSettings();

    // Get exe path
    WCHAR* strLastSlash = NULL;
    WCHAR strExePath[MAX_PATH];
    GetModuleFileName( NULL, strExePath, MAX_PATH );
    strExePath[MAX_PATH-1]=0;
    strLastSlash = wcsrchr( strExePath, TEXT('\\') );
    if( strLastSlash )
        *strLastSlash = 0;

    wcscpy( m_strFile, strExePath );
    wcscat( m_strFile, L"\\options.xml" );

    LoadOptions( m_strFile );
}


//--------------------------------------------------------------------------------------
COptionsFile::~COptionsFile()
{
}


//--------------------------------------------------------------------------------------
HRESULT COptionsFile::SaveOptions( WCHAR* strFile )
{
    if( strFile == NULL )
        strFile = m_strFile;

    HRESULT hr = S_OK;
    IXMLDOMDocument *pDoc = NULL;

    CoInitialize(NULL);

    // Create an empty XML document
    V_RETURN( CoCreateInstance( CLSID_DOMDocument, NULL, 
                                CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, 
                                (void**)&pDoc ) );

    IXMLDOMNode* pRootNode = NULL;
    IXMLDOMNode* pTopNode = NULL;    
    pDoc->QueryInterface( IID_IXMLDOMNode, (VOID**)&pRootNode );
    CXMLHelper::CreateChildNode( pDoc, pRootNode, L"PRTOptions", NODE_ELEMENT, &pTopNode );

//    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"strInitialDir", m_Options.strInitialDir );
 //   CXMLHelper::CreateNewValue( pDoc, pTopNode, L"strInputMesh", m_Options.strInputMesh );
//    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"strResultsFile", m_Options.strResultsFile );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"dwOrder", (DWORD)m_Options.dwOrder );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"dwNumRays", (DWORD)m_Options.dwNumRays );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"dwNumBounces", (DWORD)m_Options.dwNumBounces );
 //   CXMLHelper::CreateNewValue( pDoc, pTopNode, L"bShowTooltips", (DWORD)m_Options.bShowTooltips );

    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"dwNumClusters", (DWORD)m_Options.dwNumClusters );
 //   CXMLHelper::CreateNewValue( pDoc, pTopNode, L"Quality", (DWORD) m_Options.Quality );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"dwNumPCA", (DWORD)m_Options.dwNumPCA );

    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"bSubsurfaceScattering", (DWORD)m_Options.bSubsurfaceScattering );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"fLengthScale", m_Options.fLengthScale );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"dwNumChannels", (DWORD)m_Options.dwNumChannels );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"dwPredefinedMatIndex", (DWORD)m_Options.dwPredefinedMatIndex );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"Diffuse.r", m_Options.Diffuse.r );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"Diffuse.g", m_Options.Diffuse.g );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"Diffuse.b", m_Options.Diffuse.b );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"Absoption.r", m_Options.Absoption.r );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"Absoption.g", m_Options.Absoption.g );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"Absoption.b", m_Options.Absoption.b );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"ReducedScattering.r", m_Options.ReducedScattering.r );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"ReducedScattering.g", m_Options.ReducedScattering.g );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"ReducedScattering.b", m_Options.ReducedScattering.b );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"fRelativeIndexOfRefraction", m_Options.fRelativeIndexOfRefraction );

    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"bAdaptive", (DWORD)m_Options.bAdaptive );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"bRobustMeshRefine", (DWORD)m_Options.bRobustMeshRefine );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"fRobustMeshRefineMinEdgeLength", m_Options.fRobustMeshRefineMinEdgeLength );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"dwRobustMeshRefineMaxSubdiv", (DWORD)m_Options.dwRobustMeshRefineMaxSubdiv );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"bAdaptiveDL", (DWORD)m_Options.bAdaptiveDL );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"fAdaptiveDLMinEdgeLength", m_Options.fAdaptiveDLMinEdgeLength );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"fAdaptiveDLThreshold", m_Options.fAdaptiveDLThreshold );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"dwAdaptiveDLMaxSubdiv", (DWORD)m_Options.dwAdaptiveDLMaxSubdiv );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"bAdaptiveBounce", (DWORD)m_Options.bAdaptiveBounce );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"fAdaptiveBounceMinEdgeLength", m_Options.fAdaptiveBounceMinEdgeLength );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"fAdaptiveBounceThreshold", m_Options.fAdaptiveBounceThreshold );
    CXMLHelper::CreateNewValue( pDoc, pTopNode, L"dwAdaptiveBounceMaxSubdiv", (DWORD)m_Options.dwAdaptiveBounceMaxSubdiv );

	// TIM: Texture size
	 CXMLHelper::CreateNewValue( pDoc, pTopNode, L"dwTextureSize", (DWORD)m_Options.dwTextureSize );
	 CXMLHelper::CreateNewValue( pDoc, pTopNode, L"bPerPixel", (DWORD)m_Options.bPerPixel );

  //  CXMLHelper::CreateNewValue( pDoc, pTopNode, L"strOutputMesh", m_Options.strOutputMesh );

  //  CXMLHelper::CreateNewValue( pDoc, pTopNode, L"bSaveCompressedResults", (DWORD)m_Options.bSaveCompressedResults );
  //  CXMLHelper::CreateNewValue( pDoc, pTopNode, L"bBinaryOutputXFile", (DWORD)m_Options.bBinaryOutputXFile );

    SAFE_RELEASE( pTopNode );
    SAFE_RELEASE( pRootNode );

    // Save the doc
    VARIANT vName;
    vName.vt = VT_BSTR;
    vName.bstrVal = SysAllocString(strFile);
    hr = pDoc->save(vName);
    VariantClear( &vName );

    SAFE_RELEASE( pDoc );

    return hr;
}

//--------------------------------------------------------------------------------------
HRESULT CPRTOptionsDlg::LoadOptions( WCHAR* strFile )
{
    return g_OptionsFile.LoadOptions( strFile );
}

//--------------------------------------------------------------------------------------
HRESULT COptionsFile::LoadOptions( WCHAR* strFile )
{
    if( strFile == NULL )
        strFile = m_strFile;

    HRESULT hr = S_OK;
    VARIANT v;
    VARIANT_BOOL vb;
    IXMLDOMDocument *pDoc = NULL;
    IXMLDOMNode* pRootNode = NULL;
    IXMLDOMNode* pTopNode = NULL;
    IXMLDOMNode* pNode = NULL;

    CoInitialize(NULL);
    V_RETURN( CoCreateInstance( CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
                                IID_IXMLDOMDocument, (void**)&pDoc ) );

    VariantInit(&v);
    v.vt = VT_BSTR;
    V_BSTR(&v) = SysAllocString(strFile);
    hr = pDoc->load(v, &vb);
    if( FAILED(hr) || hr == S_FALSE ) 
        return E_FAIL;
    VariantClear(&v);

    pDoc->QueryInterface( IID_IXMLDOMNode, (void**)&pRootNode );
    SAFE_RELEASE( pDoc );
    pRootNode->get_firstChild(&pTopNode);
    SAFE_RELEASE( pRootNode );
    pTopNode->get_firstChild(&pNode);
    SAFE_RELEASE( pTopNode );

    ZeroMemory( &m_Options, sizeof(SIMULATOR_OPTIONS) );
 //   CXMLHelper::GetValue( pNode, L"strInitialDir", m_Options.strInitialDir );
 //   CXMLHelper::GetValue( pNode, L"strInputMesh", m_Options.strInputMesh );
 //   CXMLHelper::GetValue( pNode, L"strResultsFile", m_Options.strResultsFile );
    CXMLHelper::GetValue( pNode, L"dwOrder", &m_Options.dwOrder );
    CXMLHelper::GetValue( pNode, L"dwNumRays", &m_Options.dwNumRays );
    CXMLHelper::GetValue( pNode, L"dwNumBounces", &m_Options.dwNumBounces );
 //   CXMLHelper::GetValue( pNode, L"bShowTooltips", &m_Options.bShowTooltips );

    CXMLHelper::GetValue( pNode, L"dwNumClusters", &m_Options.dwNumClusters );
  //  DWORD dwQuality;
 //   CXMLHelper::GetValue( pNode, L"Quality", &dwQuality );
 //   m_Options.Quality = static_cast<D3DXSHCOMPRESSQUALITYTYPE>( dwQuality );
    CXMLHelper::GetValue( pNode, L"dwNumPCA", &m_Options.dwNumPCA );

    CXMLHelper::GetValue( pNode, L"bSubsurfaceScattering", &m_Options.bSubsurfaceScattering );
    CXMLHelper::GetValue( pNode, L"fLengthScale", &m_Options.fLengthScale );
    CXMLHelper::GetValue( pNode, L"dwNumChannels", &m_Options.dwNumChannels );
    CXMLHelper::GetValue( pNode, L"dwPredefinedMatIndex", &m_Options.dwPredefinedMatIndex );
    CXMLHelper::GetValue( pNode, L"Diffuse.r", &m_Options.Diffuse.r );
    CXMLHelper::GetValue( pNode, L"Diffuse.g", &m_Options.Diffuse.g );
    CXMLHelper::GetValue( pNode, L"Diffuse.b", &m_Options.Diffuse.b );
    CXMLHelper::GetValue( pNode, L"Absoption.r", &m_Options.Absoption.r );
    CXMLHelper::GetValue( pNode, L"Absoption.g", &m_Options.Absoption.g );
    CXMLHelper::GetValue( pNode, L"Absoption.b", &m_Options.Absoption.b );
    CXMLHelper::GetValue( pNode, L"ReducedScattering.r", &m_Options.ReducedScattering.r );
    CXMLHelper::GetValue( pNode, L"ReducedScattering.g", &m_Options.ReducedScattering.g );
    CXMLHelper::GetValue( pNode, L"ReducedScattering.b", &m_Options.ReducedScattering.b );
    CXMLHelper::GetValue( pNode, L"fRelativeIndexOfRefraction", &m_Options.fRelativeIndexOfRefraction );

    CXMLHelper::GetValue( pNode, L"bAdaptive", &m_Options.bAdaptive );
    CXMLHelper::GetValue( pNode, L"bRobustMeshRefine", &m_Options.bRobustMeshRefine );
    CXMLHelper::GetValue( pNode, L"fRobustMeshRefineMinEdgeLength", &m_Options.fRobustMeshRefineMinEdgeLength );
    CXMLHelper::GetValue( pNode, L"dwRobustMeshRefineMaxSubdiv", &m_Options.dwRobustMeshRefineMaxSubdiv );
    CXMLHelper::GetValue( pNode, L"bAdaptiveDL", &m_Options.bAdaptiveDL );
    CXMLHelper::GetValue( pNode, L"fAdaptiveDLMinEdgeLength", &m_Options.fAdaptiveDLMinEdgeLength );
    CXMLHelper::GetValue( pNode, L"fAdaptiveDLThreshold", &m_Options.fAdaptiveDLThreshold );
    CXMLHelper::GetValue( pNode, L"dwAdaptiveDLMaxSubdiv", &m_Options.dwAdaptiveDLMaxSubdiv );
    CXMLHelper::GetValue( pNode, L"bAdaptiveBounce", &m_Options.bAdaptiveBounce );
    CXMLHelper::GetValue( pNode, L"fAdaptiveBounceMinEdgeLength", &m_Options.fAdaptiveBounceMinEdgeLength );
    CXMLHelper::GetValue( pNode, L"fAdaptiveBounceThreshold", &m_Options.fAdaptiveBounceThreshold );
    CXMLHelper::GetValue( pNode, L"dwAdaptiveBounceMaxSubdiv", &m_Options.dwAdaptiveBounceMaxSubdiv );
	// TIM: Texture size
	 CXMLHelper::GetValue( pNode, L"dwTextureSize", &m_Options.dwTextureSize );
	 CXMLHelper::GetValue( pNode, L"bPerPixel", &m_Options.bPerPixel );

 //   CXMLHelper::GetValue( pNode, L"strOutputMesh", m_Options.strOutputMesh );
 //   CXMLHelper::GetValue( pNode, L"bSaveCompressedResults", &m_Options.bSaveCompressedResults );
 //   CXMLHelper::GetValue( pNode, L"bBinaryOutputXFile", &m_Options.bBinaryOutputXFile );

    SAFE_RELEASE( pNode );

    return S_OK;
}


//--------------------------------------------------------------------------------------
bool CPRTOptionsDlg::Show(HWND hwnd)
{
    // Ask the user about param settings for the PRT Simulation
    int nResult = (int) DialogBox( g_hInstance, MAKEINTRESOURCE(IDD_SH), 
                                   hwnd, StaticDlgProc );

    UnhookWindowsHookEx( m_hMsgProcHook );
    DestroyWindow( m_hToolTip );

    if( nResult == IDOK )
        return true;
    else
        return false;
}


//--------------------------------------------------------------------------------------
INT_PTR CALLBACK CPRTOptionsDlg::StaticDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return g_pDlg->DlgProc( hDlg, uMsg, wParam, lParam );
}


bool CPRTOptionsDlg::GetSettings()
{
	HWND hDlg = m_hDlg;
	//GetDlgItemText( hDlg, IDC_MESH_NAME, GetGlobalOptions().strInputMesh, MAX_PATH );
	//GetDlgItemText( hDlg, IDC_OUTPUT_MESH_EDIT, GetGlobalOptions().strOutputMesh, MAX_PATH );

	GetGlobalOptions().dwOrder = (DWORD) SendMessage( GetDlgItem( hDlg, IDC_ORDER_SLIDER ), TBM_GETPOS, 0, 0 );
	GetGlobalOptions().dwNumBounces = (DWORD) SendMessage( GetDlgItem( hDlg, IDC_NUM_BOUNCES_SPIN ), UDM_GETPOS, 0, 0 );
	GetGlobalOptions().dwNumRays = (DWORD) SendMessage( GetDlgItem( hDlg, IDC_NUM_RAYS_SPIN ), UDM_GETPOS, 0, 0 );
	GetGlobalOptions().bSubsurfaceScattering = (IsDlgButtonChecked( hDlg, IDC_SUBSURF_CHECK ) == BST_CHECKED );
	GetGlobalOptions().bAdaptive = (IsDlgButtonChecked( hDlg, IDC_ADAPTIVE_CHECK ) == BST_CHECKED );
	GetGlobalOptions().dwNumChannels = 3;//(IsDlgButtonChecked( hDlg, IDC_SPECTRAL_CHECK ) == BST_CHECKED ) * 3;
	GetGlobalOptions().dwPredefinedMatIndex = (DWORD) SendMessage( GetDlgItem( hDlg, IDC_PREDEF_COMBO ), CB_GETCURSEL, 0, 0 );
	//                    GetGlobalOptions().bSaveCompressedResults = (IsDlgButtonChecked( hDlg, IDC_COMPRESSED_CHECK ) == BST_CHECKED);
	//                    GetGlobalOptions().bBinaryOutputXFile = (IsDlgButtonChecked( hDlg, IDC_MESH_SAVE_BINARY_RADIO ) == BST_CHECKED);


	// TIM: Texture selection
	GetGlobalOptions().bPerPixel = (IsDlgButtonChecked( hDlg, IDC_PERPIXEL ) == BST_CHECKED );
	
	int nIndex = (int) SendMessage( GetDlgItem( hDlg, IDC_TEXTURE_SIZE_COMBO ), CB_GETCURSEL, 0, 0 );
	LRESULT lResult = SendMessage( GetDlgItem( hDlg, IDC_TEXTURE_SIZE_COMBO ), CB_GETITEMDATA, nIndex, 0 );
	if( lResult != CB_ERR )
		GetGlobalOptions().dwTextureSize = (DWORD)lResult;

	if( GetGlobalOptions().bAdaptive )
	{
		if( !GetGlobalOptions().bRobustMeshRefine &&
			!GetGlobalOptions().bAdaptiveDL &&
			!GetGlobalOptions().bAdaptiveBounce )
		{
			GetGlobalOptions().bAdaptive = false;
		}
	}

	TCHAR sz[256];
	GetDlgItemText( hDlg, IDC_REFRACTION_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().fRelativeIndexOfRefraction ) == 0 ) return false;
	GetDlgItemText( hDlg, IDC_ABSORPTION_R_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().Absoption.r ) == 0 ) return false;
	GetDlgItemText( hDlg, IDC_ABSORPTION_G_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().Absoption.g ) == 0 ) return false;
	GetDlgItemText( hDlg, IDC_ABSORPTION_B_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().Absoption.b ) == 0 ) return false;
	GetDlgItemText( hDlg, IDC_REFLECTANCE_R_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().Diffuse.r ) == 0 ) return false;
	GetDlgItemText( hDlg, IDC_REFLECTANCE_G_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().Diffuse.g ) == 0 ) return false;
	GetDlgItemText( hDlg, IDC_REFLECTANCE_B_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().Diffuse.b ) == 0 ) return false;
	GetDlgItemText( hDlg, IDC_SCATTERING_R_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().ReducedScattering.r ) == 0 ) return false;
	GetDlgItemText( hDlg, IDC_SCATTERING_G_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().ReducedScattering.g ) == 0 ) return false;
	GetDlgItemText( hDlg, IDC_SCATTERING_B_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().ReducedScattering.b ) == 0 ) return false;
	GetDlgItemText( hDlg, IDC_LENGTH_SCALE_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().fLengthScale ) == 0 ) return false;

	return true;
}

//--------------------------------------------------------------------------------------
// Handles messages for the Simulation options dialog
//--------------------------------------------------------------------------------------
INT_PTR CALLBACK CPRTOptionsDlg::DlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_INITDIALOG:
        {
            m_hDlg = hDlg;

            UpdateControlsWithSettings(&GetGlobalOptions(), hDlg );

            m_hToolTip = CreateWindowEx( 0, TOOLTIPS_CLASS, NULL, TTS_ALWAYSTIP, 
                                         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                                         hDlg, NULL, (HINSTANCE)g_hInstance, NULL );                            
            SendMessage( m_hToolTip, TTM_SETMAXTIPWIDTH, 0, 300 );
            SendMessage( m_hToolTip, TTM_SETDELAYTIME, (WPARAM)(DWORD) TTDT_AUTOPOP, 32000 );
            SendMessage( m_hToolTip, TTM_SETDELAYTIME, (WPARAM)(DWORD) TTDT_INITIAL, 0 );
            SendMessage( m_hToolTip, TTM_SETDELAYTIME, (WPARAM)(DWORD) TTDT_RESHOW, 0 );
            EnumChildWindows( hDlg, (WNDENUMPROC) EnumChildProc, 0);
            m_hMsgProcHook = SetWindowsHookEx( WH_GETMESSAGE, GetMsgProc, (HINSTANCE)g_hInstance, GetCurrentThreadId() );

            return TRUE;
        }
        
        case WM_NOTIFY:
        {
            NMHDR* pNMHDR = (LPNMHDR) lParam;
            if( pNMHDR == NULL )
                break;

            if( pNMHDR->code == TTN_NEEDTEXT )
            {
                NMTTDISPINFO* pNMTDI = (LPNMTTDISPINFO)lParam;
                int nDlgId = GetDlgCtrlID( (HWND)pNMHDR->idFrom );
                if( m_bShowTooltips )
                {
                    GetToolTipText( nDlgId, pNMTDI );
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            break;
        }

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
//                case IDC_SETTINGS_RESET:
//                    ResetSettings();
//                    UpdateControlsWithSettings( hDlg );
//                    break;

			case IDC_PERPIXEL:
				{
					BOOL bPerPixel   = (IsDlgButtonChecked( hDlg, IDC_PERPIXEL ) == BST_CHECKED );
					EnableWindow( GetDlgItem( hDlg, IDC_TEXTURE_SIZE_COMBO ), bPerPixel );
					break;
				}

                case IDC_ADAPTIVE_SETTINGS:
                {
                    CPRTAdaptiveOptionsDlg dlg;
                    dlg.Show( hDlg );
                    break;
                }


                case IDC_ADAPTIVE_CHECK:
                case IDC_SPECTRAL_CHECK:
                case IDC_SUBSURF_CHECK:
                {
                    BOOL bSubSurface = (IsDlgButtonChecked( hDlg, IDC_SUBSURF_CHECK ) == BST_CHECKED );
                    BOOL bSpectral   = (IsDlgButtonChecked( hDlg, IDC_SPECTRAL_CHECK ) == BST_CHECKED );
                    BOOL bAdaptive   = (IsDlgButtonChecked( hDlg, IDC_ADAPTIVE_CHECK ) == BST_CHECKED );

                   // EnableWindow( GetDlgItem( hDlg, IDC_OUTPUT_MESH_TEXT ), bAdaptive );
                   // EnableWindow( GetDlgItem( hDlg, IDC_OUTPUT_MESH_EDIT ), bAdaptive );
                    //EnableWindow( GetDlgItem( hDlg, IDC_OUTPUT_MESH_BROWSE_BUTTON ), bAdaptive );
                    EnableWindow( GetDlgItem( hDlg, IDC_ADAPTIVE_SETTINGS ), bAdaptive );
                   // EnableWindow( GetDlgItem( hDlg, IDC_MESH_SAVE_BINARY_RADIO ), bAdaptive );
                   // EnableWindow( GetDlgItem( hDlg, IDC_MESH_SAVE_TEXT_RADIO ), bAdaptive );

                    EnableWindow( GetDlgItem( hDlg, IDC_REFRACTION_TEXT ), bSubSurface );
                    EnableWindow( GetDlgItem( hDlg, IDC_ABSORPTION_TEXT ), bSubSurface );
                    EnableWindow( GetDlgItem( hDlg, IDC_SCATTERING_TEXT ), bSubSurface );
                    EnableWindow( GetDlgItem( hDlg, IDC_REFLECTANCE_R_EDIT ), true );
                    EnableWindow( GetDlgItem( hDlg, IDC_REFLECTANCE_G_EDIT ), bSpectral );
                    EnableWindow( GetDlgItem( hDlg, IDC_REFLECTANCE_B_EDIT ), bSpectral );
                    EnableWindow( GetDlgItem( hDlg, IDC_ABSORPTION_R_EDIT ), bSubSurface );
                    EnableWindow( GetDlgItem( hDlg, IDC_ABSORPTION_G_EDIT ), bSubSurface && bSpectral );
                    EnableWindow( GetDlgItem( hDlg, IDC_ABSORPTION_B_EDIT ), bSubSurface && bSpectral );
                    EnableWindow( GetDlgItem( hDlg, IDC_SCATTERING_R_EDIT ), bSubSurface );
                    EnableWindow( GetDlgItem( hDlg, IDC_SCATTERING_G_EDIT ), bSubSurface && bSpectral );
                    EnableWindow( GetDlgItem( hDlg, IDC_SCATTERING_B_EDIT ), bSubSurface && bSpectral );
                    EnableWindow( GetDlgItem( hDlg, IDC_REFRACTION_EDIT ), bSubSurface );
                    EnableWindow( GetDlgItem( hDlg, IDC_LENGTH_SCALE_TEXT ), bSubSurface );
                    EnableWindow( GetDlgItem( hDlg, IDC_LENGTH_SCALE_EDIT ), bSubSurface );
                    break;
                }

                case IDC_PREDEF_COMBO:
                    if( HIWORD(wParam) == CBN_SELCHANGE )
                    {
                        HWND hPreDefCombo = GetDlgItem( hDlg, IDC_PREDEF_COMBO );
                        int nIndex = (int) SendMessage( hPreDefCombo, CB_GETCURSEL, 0, 0 );
                        LRESULT lResult = SendMessage( hPreDefCombo, CB_GETITEMDATA, nIndex, 0 );
                        if( lResult != CB_ERR )
                        {
                            // If this is the "Custom" material, don't change the numbers
                            if( nIndex == g_aPredefinedMaterialsSize-1 )
                                break; 

                            PREDEFINED_MATERIALS* pMat = (PREDEFINED_MATERIALS*) lResult;
                            TCHAR sz[256];
                            ZeroMemory( sz, 256 );

                            m_bComboBoxSelChange = true;
                            _snwprintf( sz, 256, TEXT("%0.4f"), pMat->Absoption.r ); SetDlgItemText( hDlg, IDC_ABSORPTION_R_EDIT, sz ); 
                            _snwprintf( sz, 256, TEXT("%0.4f"), pMat->Absoption.g ); SetDlgItemText( hDlg, IDC_ABSORPTION_G_EDIT, sz ); 
                            _snwprintf( sz, 256, TEXT("%0.4f"), pMat->Absoption.b ); SetDlgItemText( hDlg, IDC_ABSORPTION_B_EDIT, sz ); 
                            _snwprintf( sz, 256, TEXT("%0.2f"), pMat->Diffuse.r ); SetDlgItemText( hDlg, IDC_REFLECTANCE_R_EDIT, sz ); 
                            _snwprintf( sz, 256, TEXT("%0.2f"), pMat->Diffuse.g ); SetDlgItemText( hDlg, IDC_REFLECTANCE_G_EDIT, sz ); 
                            _snwprintf( sz, 256, TEXT("%0.2f"), pMat->Diffuse.b ); SetDlgItemText( hDlg, IDC_REFLECTANCE_B_EDIT, sz ); 
                            _snwprintf( sz, 256, TEXT("%0.2f"), pMat->ReducedScattering.r ); SetDlgItemText( hDlg, IDC_SCATTERING_R_EDIT, sz ); 
                            _snwprintf( sz, 256, TEXT("%0.2f"), pMat->ReducedScattering.g ); SetDlgItemText( hDlg, IDC_SCATTERING_G_EDIT, sz ); 
                            _snwprintf( sz, 256, TEXT("%0.2f"), pMat->ReducedScattering.b ); SetDlgItemText( hDlg, IDC_SCATTERING_B_EDIT, sz ); 
                            _snwprintf( sz, 256, TEXT("%0.2f"), pMat->fRelativeIndexOfRefraction ); SetDlgItemText( hDlg, IDC_REFRACTION_EDIT, sz ); 
                            m_bComboBoxSelChange = false;
                        }
                    }
                    break;

                case IDC_REFLECTANCE_R_EDIT:
                case IDC_REFLECTANCE_G_EDIT:
                case IDC_REFLECTANCE_B_EDIT:
                case IDC_SCATTERING_R_EDIT:
                case IDC_SCATTERING_G_EDIT:
                case IDC_SCATTERING_B_EDIT:
                case IDC_ABSORPTION_R_EDIT:
                case IDC_ABSORPTION_G_EDIT:
                case IDC_ABSORPTION_B_EDIT:
                {
                    if( HIWORD(wParam) == EN_CHANGE && !m_bComboBoxSelChange )
                    {
                        HWND hPreDefCombo = GetDlgItem( hDlg, IDC_PREDEF_COMBO );
                        SendMessage( hPreDefCombo, CB_SETCURSEL, g_aPredefinedMaterialsSize-1, 0 );
                    }
                    break;
                }

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;
            }
            break;

        case WM_CLOSE:
            break;
    }

    return FALSE;
}


//-----------------------------------------------------------------------------
// Name: EnumChildProc
// Desc: Helper function to add tooltips to all the children (buttons/etc) 
//       of the dialog box
//-----------------------------------------------------------------------------
BOOL CALLBACK CPRTOptionsDlg::EnumChildProc( HWND hwndChild, LPARAM lParam )
{
    TOOLINFO ti;
    ti.cbSize   = sizeof(TOOLINFO);
    ti.uFlags   = TTF_IDISHWND;
    ti.hwnd     = g_pDlg->m_hDlg;
    ti.uId      = (UINT_PTR) hwndChild;
    ti.hinst    = g_hInstance;
    ti.lpszText = LPSTR_TEXTCALLBACK;
    SendMessage( g_pDlg->m_hToolTip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti );

    return TRUE;
}


//-----------------------------------------------------------------------------
// Name: GetMsgProc
// Desc: msg proc hook needed to display tooltips in a dialog box
//-----------------------------------------------------------------------------
LRESULT CALLBACK CPRTOptionsDlg::GetMsgProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    MSG* pMSG = (MSG*) lParam;
    if( nCode < 0 || !(IsChild( g_pDlg->m_hDlg, pMSG->hwnd) ) )
        return CallNextHookEx( g_pDlg->m_hMsgProcHook, nCode, wParam, lParam ); 

    switch( pMSG->message )
    {
        case WM_MOUSEMOVE: 
        case WM_LBUTTONDOWN: 
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN: 
        case WM_RBUTTONUP:
        {
            if( g_pDlg->m_hToolTip )
            {
                MSG newMsg;
                newMsg.lParam  = pMSG->lParam;
                newMsg.wParam  = pMSG->wParam;
                newMsg.message = pMSG->message;
                newMsg.hwnd    = pMSG->hwnd;
                SendMessage( g_pDlg->m_hToolTip, TTM_RELAYEVENT, 0, (LPARAM) &newMsg );
            }
            break;
        }
    }

    return CallNextHookEx( g_pDlg->m_hMsgProcHook, nCode, wParam, lParam );
}


//-----------------------------------------------------------------------------
// Returns the tooltip text for the control 
//-----------------------------------------------------------------------------
void CPRTOptionsDlg::GetToolTipText( int nDlgId, NMTTDISPINFO* pNMTDI )
{
    TCHAR* str = NULL;
    switch( nDlgId )
    {
//        case IDC_INPUT_MESH_TEXT: 
//        case IDC_MESH_NAME: str = TEXT("This is the file that is loaded as a ID3DXMesh* and passed into D3DXSHPRTSimulation() so that it can compute and return spherical harmonic transfer coefficients for each vertex in the mesh. It returns these coefficients in a ID3DXBuffer*. This process takes some time and should be precomputed, however the results can be used in real time. For more detailed information, see \"Precomputed Radiance Transfer for Real-Time Rendering in Dynamic, Low-Frequency Lighting Environments\" by Peter-Pike Sloan, Jan Kautz, and John Snyder, SIGGRAPH 2002."); break;

        case IDC_ORDER_TEXT: 
        case IDC_ORDER_SLIDER: str = TEXT("This controls the number of spherical harmonic basis functions used. The simulator generates order^2 coefficients per channel. Higher order allows for higher frequency lighting environments which allow for sharper shadows with the tradeoff of more coefficients per vertex that need to be processed by the vertex shader. For convex objects (no shadows), 3rd order has very little approximation error.  For more detailed information, see \"Spherical Harmonic Lighting: The Gritty Details\" by Robin Green, GDC 2003 and \"An Efficient Representation of Irradiance Environment Maps\" by Ravi Ramamoorthi, and Pat Hanrahan, SIGGRAPH 2001."); break;

        case IDC_NUM_BOUNCES_TEXT: 
        case IDC_NUM_BOUNCES_EDIT: 
        case IDC_NUM_BOUNCES_SPIN: str = TEXT("This controls the number of bounces simulated. If this is non-zero then inter-reflections are calculated. Inter-reflections are, for example, when a light shines on a red wall and bounces on a white wall. The white wall even though it contains no red in the material will reflect some red do to the bouncing of the light off the red wall."); break;

        case IDC_NUM_RAYS_TEXT: 
        case IDC_NUM_RAYS_EDIT: 
        case IDC_NUM_RAYS_SPIN: str = TEXT("This controls the number of rays to shoot at each sample. The more rays the more accurate the final result will be, but it will increase time it takes to precompute the transfer coefficients."); break;

        case IDC_SUBSURF_CHECK: str = TEXT("If checked then subsurface scattering will be done in the simulator. Subsurface scattering is when light penetrates a translucent surface and comes out the other side. For example, a jade sculpture or a flashlight shining through skin exhibits subsurface scattering. The simulator assumes the mesh is made of a homogenous material. If subsurface scattering is not used, then the length scale, the relative index of refraction, the reduced scattering coefficients, and the absorption coefficients are not used."); break;

        case IDC_SPECTRAL_CHECK: str = TEXT("If checked then the simulator will process 3 channels: red, green, and blue and return order^2 spherical harmonic transfer coefficients for each of these channels in a single ID3DXBuffer* buffer. Otherwise it use values of only one channel (the red channel) and return the transfer coefficients for just that single channel. A single channel is useful for lighting environments that don't need to have the whole spectrum of light such as shadows"); break;

        case IDC_RED_TEXT: str = TEXT("The values below are the red coefficients. If spectral is turned off, then this is the channel that will be used."); break;
        case IDC_GREEN_TEXT: str = TEXT("The values below are the green coefficients"); break;
        case IDC_BLUE_TEXT: str = TEXT("The values below are the blue coefficients"); break;

        case IDC_PREDEF_COMBO: 
        case IDC_PREDEF_TEXT: str = TEXT("These are some example materials. Choosing one of these materials with change the all the material values below. The parameters for these materials are from \"A Practical Model for Subsurface Light Transport\" by Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan, SIGGRAPH 2001. The relative index of refraction is with respect the material immersed in air."); break;

        case IDC_REFRACTION_TEXT: 
        case IDC_REFRACTION_EDIT: str = TEXT("Relative index of refraction is the ratio between two absolute indexes of refraction. An index of refraction is ratio of the sine of the angle of incidence to the sine of the angle of refraction."); break;

        case IDC_LENGTH_SCALE_TEXT: 
        case IDC_LENGTH_SCALE_EDIT: str = TEXT("When subsurface scattering is used the object is mapped to a cube of length scale mm per side. For example, if length scale is 10, then the object is mapped to a 10mm x 10mm x 10mm cube.  The smaller the cube the more light penetrates the object."); break;

        case IDC_SCATTERING_TEXT: 
        case IDC_SCATTERING_G_EDIT: 
        case IDC_SCATTERING_B_EDIT: 
        case IDC_SCATTERING_R_EDIT: str = TEXT("The reduced scattering coefficient is a parameter to the volume rendering equation used to model light propagation in a participating medium. For more detail, see \"A Practical Model for Subsurface Light Transport\" by Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan, SIGGRAPH 2001"); break;

        case IDC_ABSORPTION_TEXT: 
        case IDC_ABSORPTION_G_EDIT: 
        case IDC_ABSORPTION_B_EDIT: 
        case IDC_ABSORPTION_R_EDIT: str = TEXT("The absorption coefficient is a parameter to the volume rendering equation used to model light propagation in a participating medium. For more detail, see \"A Practical Model for Subsurface Light Transport\" by Henrik Wann Jensen, Steve R. Marschner, Marc Levoy, Pat Hanrahan, SIGGRAPH 2001"); break;

        case IDC_REFLECTANCE_TEXT: 
        case IDC_REFLECTANCE_R_EDIT: 
        case IDC_REFLECTANCE_B_EDIT: 
        case IDC_REFLECTANCE_G_EDIT: str = TEXT("The diffuse reflectance coefficient is the fraction of diffuse light reflected back. This value is typically between 0 and 1."); break;

 //       case IDC_OUTPUT_TEXT: 
 //       case IDC_OUTPUT_EDIT: str = TEXT("This sample will save a binary buffer of spherical harmonic transfer coefficients to a file which the sample can read in later.  This is the file name of the where the resulting binary buffer will be saved"); break;

 //       case IDC_OUTPUT_MESH_TEXT: 
  //      case IDC_OUTPUT_MESH_EDIT: str = TEXT("If adaptive tessellation is on, then this sample will save the resulting tessellated mesh to this file."); break;

  //      case IDC_OUTPUT_BROWSE_BUTTON: str = TEXT("Select the output buffer file name"); break;
  //      case IDC_OUTPUT_MESH_BROWSE_BUTTON: str = TEXT("Select the output mesh file name"); break;

        case IDOK: str = TEXT("This will start the simulator based on the options selected above. This process takes some time and should be precomputed, however the results can be used in real time. When the simulator is done calculating the spherical harmonic transfer coefficients for each vertex, the sample will use D3DXSHPRTCompress() to compress the data based on the number of PCA vectors chosen. This sample will then save the binary data of coefficients to a file. This sample will also allow you to view the results by passing these coefficients to a vertex shader for real time rendering (if the Direct3D device has hardware accelerated programmable vertex shader support). This sample also stores the settings of this dialog to the registry so it remembers them for next time."); break;

        case IDCANCEL: str = TEXT("This cancels the dialog, does not save the settings, and does not run the simulator."); break;
    }

    pNMTDI->lpszText = str;
}


//-----------------------------------------------------------------------------
void CPRTOptionsDlg::ResetSettings()
{
    g_OptionsFile.ResetSettings();
}


//-----------------------------------------------------------------------------
void COptionsFile::ResetSettings()
{
    ZeroMemory( &m_Options, sizeof(SIMULATOR_OPTIONS) );

  /*  wcscpy( m_Options.strInputMesh, L"misc\\shapes1.x" );
    DXUTGetDXSDKMediaPathCch( m_Options.strInitialDir, MAX_PATH );
    wcscat( m_Options.strInitialDir, L"" );
    DXUTFindDXSDKMediaFileCch( m_Options.strInitialDir, MAX_PATH, m_Options.strInputMesh );
    WCHAR* pLastSlash =  wcsrchr( m_Options.strInitialDir, L'\\' );
    if( pLastSlash )
        *pLastSlash = 0;
    wcscpy( m_Options.strResultsFile, L"shapes1_prtresults.pca" );*/
    m_Options.dwOrder = 6;
    m_Options.dwNumRays = 1024;
    m_Options.dwNumBounces = 1;
    m_Options.bSubsurfaceScattering = false;
    m_Options.fLengthScale = 1.0f ;
    m_Options.dwNumChannels = 3;

#define ToFColor(c) FloatColor(c.r,c.g,c.b,c.a)
    m_Options.dwPredefinedMatIndex = 0;
    m_Options.fRelativeIndexOfRefraction = g_aPredefinedMaterials[0].fRelativeIndexOfRefraction;
    m_Options.Diffuse = ToFColor(g_aPredefinedMaterials[0].Diffuse);
    m_Options.Absoption = ToFColor(g_aPredefinedMaterials[0].Absoption);
    m_Options.ReducedScattering = ToFColor(g_aPredefinedMaterials[0].ReducedScattering);

    m_Options.bAdaptive = false;
    m_Options.bRobustMeshRefine = true;
    m_Options.fRobustMeshRefineMinEdgeLength = 0.0f;
    m_Options.dwRobustMeshRefineMaxSubdiv = 2;
    m_Options.bAdaptiveDL = true;
    m_Options.fAdaptiveDLMinEdgeLength = 0.03f;
    m_Options.fAdaptiveDLThreshold = 8e-5f;
    m_Options.dwAdaptiveDLMaxSubdiv = 3;
    m_Options.bAdaptiveBounce = false;
    m_Options.fAdaptiveBounceMinEdgeLength = 0.03f;
    m_Options.fAdaptiveBounceThreshold = 8e-5f;
    m_Options.dwAdaptiveBounceMaxSubdiv = 3;
   // wcscpy( m_Options.strOutputMesh, L"shapes1_adaptive.x" );
   // m_Options.bBinaryOutputXFile = true;

//    m_Options.bSaveCompressedResults = true;
//    m_Options.Quality = D3DXSHCQUAL_FASTLOWQUALITY;
//    m_Options.Quality = D3DXSHCQUAL_SLOWHIGHQUALITY;
    m_Options.dwNumPCA = 24;
    m_Options.dwNumClusters = 1;

	// TIM: Default texture size
	m_Options.dwTextureSize = 64;
	m_Options.bPerPixel = false;
}


//-----------------------------------------------------------------------------
void CPRTOptionsDlg::UpdateControlsWithSettings(SIMULATOR_OPTIONS* pOptions, HWND hDlg )
{
//    SetCurrentDirectory( pOptions->strInitialDir );

    //SetDlgItemText( hDlg, IDC_MESH_NAME, pOptions->strInputMesh );
    //SetDlgItemText( hDlg, IDC_OUTPUT_MESH_EDIT, pOptions->strOutputMesh );
    //SendMessage( GetDlgItem( hDlg, IDC_MESH_NAME ), EM_SETSEL, wcslen(pOptions->strInputMesh), wcslen(pOptions->strInputMesh) );
    //SendMessage( GetDlgItem( hDlg, IDC_MESH_NAME ), EM_SCROLLCARET, 0, 0 );

    HWND hOrderSlider = GetDlgItem( hDlg, IDC_ORDER_SLIDER );
    SendMessage( hOrderSlider, TBM_SETRANGE, 0, MAKELONG(2, 6) );
    SendMessage( hOrderSlider, TBM_SETTICFREQ, 1, 0 );
    SendMessage( hOrderSlider, TBM_SETPOS, 1, pOptions->dwOrder );

    HWND hNumBouncesSpin = GetDlgItem( hDlg, IDC_NUM_BOUNCES_SPIN );
    SendMessage( hNumBouncesSpin, UDM_SETRANGE, 0, (LPARAM) MAKELONG( 10, 1 ) );
    SendMessage( hNumBouncesSpin, UDM_SETPOS, 0, (LPARAM) MAKELONG( pOptions->dwNumBounces, 0) );  

    HWND hNumRaysSpin = GetDlgItem( hDlg, IDC_NUM_RAYS_SPIN );
    UDACCEL udAccel[3];
    udAccel[0].nSec = 0; udAccel[0].nInc = 1;
    udAccel[1].nSec = 1; udAccel[1].nInc = 100;           
    udAccel[2].nSec = 2; udAccel[2].nInc = 1000;           
    SendMessage( hNumRaysSpin, UDM_SETACCEL, 3, (LPARAM) udAccel );           
    SendMessage( hNumRaysSpin, UDM_SETRANGE, 0, (LPARAM) MAKELONG( 30000, 10 ) );
    SendMessage( hNumRaysSpin, UDM_SETPOS, 0, (LPARAM) MAKELONG( pOptions->dwNumRays, 0) );  

	// TIM: Texture selection
	 TCHAR sz[256];
	CheckDlgButton( hDlg, IDC_PERPIXEL, pOptions->bPerPixel ? BST_CHECKED : BST_UNCHECKED );

	// TIM: Reset
	EnableWindow( GetDlgItem( hDlg, IDC_TEXTURE_SIZE_COMBO ), pOptions->bPerPixel );
	SendMessage(GetDlgItem(hDlg,IDC_TEXTURE_SIZE_COMBO),CB_RESETCONTENT,0,0);

	HWND hTexSizeCombo = GetDlgItem( hDlg, IDC_TEXTURE_SIZE_COMBO );
	DWORD i;
	int nIndex = 0;
	int nSelection = -1;
	for( i=6; i<12; i++ )
	{                
		DWORD nSize = (DWORD)powf(2.0f,(float)i);
		_snwprintf( sz, 256, TEXT("%d"), nSize ); sz[255] = 0;                 
		nIndex = (int) SendMessage( hTexSizeCombo, CB_ADDSTRING, 0, (LPARAM) sz );                
		if( nIndex >= 0 ) 
			SendMessage( hTexSizeCombo, CB_SETITEMDATA, nIndex, (LPARAM) nSize );
		if( nSize == pOptions->dwTextureSize )
			nSelection = nIndex;
	}
	if( nSelection == -1 )
		nSelection = nIndex;
	SendMessage( hTexSizeCombo, CB_SETCURSEL, nSelection, 0 );


    CheckDlgButton( hDlg, IDC_SUBSURF_CHECK, pOptions->bSubsurfaceScattering ? BST_CHECKED : BST_UNCHECKED );
    CheckDlgButton( hDlg, IDC_ADAPTIVE_CHECK, pOptions->bAdaptive ? BST_CHECKED : BST_UNCHECKED );
    CheckDlgButton( hDlg, IDC_SPECTRAL_CHECK, (pOptions->dwNumChannels == 3) ? BST_CHECKED : BST_UNCHECKED );
    SendMessage( hDlg, WM_COMMAND, IDC_SUBSURF_CHECK, 0 );

	// TIM: Reset
	SendMessage(GetDlgItem(hDlg,IDC_PREDEF_COMBO),CB_RESETCONTENT,0,0);
    HWND hPreDefCombo = GetDlgItem( hDlg, IDC_PREDEF_COMBO );

    for( i=0; i<(DWORD)g_aPredefinedMaterialsSize; i++ )
    {
        int nIndex = (int) SendMessage( hPreDefCombo, CB_ADDSTRING, 0, (LPARAM) g_aPredefinedMaterials[i].strName );                
        if( nIndex >= 0 ) 
            SendMessage( hPreDefCombo, CB_SETITEMDATA, nIndex, (LPARAM) &g_aPredefinedMaterials[i] );
    }
    SendMessage( hPreDefCombo, CB_SETCURSEL, pOptions->dwPredefinedMatIndex, 0 );

   
    m_bComboBoxSelChange = true;
    _snwprintf( sz, 256, TEXT("%0.2f"), pOptions->fRelativeIndexOfRefraction ); sz[255] = 0; SetDlgItemText( hDlg, IDC_REFRACTION_EDIT, sz ); 
    _snwprintf( sz, 256, TEXT("%0.4f"), pOptions->Absoption.r ); sz[255] = 0; SetDlgItemText( hDlg, IDC_ABSORPTION_R_EDIT, sz ); 
    _snwprintf( sz, 256, TEXT("%0.4f"), pOptions->Absoption.g ); sz[255] = 0; SetDlgItemText( hDlg, IDC_ABSORPTION_G_EDIT, sz ); 
    _snwprintf( sz, 256, TEXT("%0.4f"), pOptions->Absoption.b ); sz[255] = 0; SetDlgItemText( hDlg, IDC_ABSORPTION_B_EDIT, sz ); 
    _snwprintf( sz, 256, TEXT("%0.2f"), pOptions->Diffuse.r ); sz[255] = 0; SetDlgItemText( hDlg, IDC_REFLECTANCE_R_EDIT, sz ); 
    _snwprintf( sz, 256, TEXT("%0.2f"), pOptions->Diffuse.g ); sz[255] = 0; SetDlgItemText( hDlg, IDC_REFLECTANCE_G_EDIT, sz ); 
    _snwprintf( sz, 256, TEXT("%0.2f"), pOptions->Diffuse.b ); sz[255] = 0; SetDlgItemText( hDlg, IDC_REFLECTANCE_B_EDIT, sz ); 
    _snwprintf( sz, 256, TEXT("%0.2f"), pOptions->ReducedScattering.r ); sz[255] = 0; SetDlgItemText( hDlg, IDC_SCATTERING_R_EDIT, sz ); 
    _snwprintf( sz, 256, TEXT("%0.2f"), pOptions->ReducedScattering.g ); sz[255] = 0; SetDlgItemText( hDlg, IDC_SCATTERING_G_EDIT, sz ); 
    _snwprintf( sz, 256, TEXT("%0.2f"), pOptions->ReducedScattering.b ); sz[255] = 0; SetDlgItemText( hDlg, IDC_SCATTERING_B_EDIT, sz ); 
    m_bComboBoxSelChange = false;

    _snwprintf( sz, 256, TEXT("%0.3f"), pOptions->fLengthScale ); sz[255] = 0; SetDlgItemText( hDlg, IDC_LENGTH_SCALE_EDIT, sz );

//    SetDlgItemText( hDlg, IDC_OUTPUT_EDIT, pOptions->strResultsFile );
//    SendMessage( GetDlgItem( hDlg, IDC_OUTPUT_EDIT ), EM_SETSEL, wcslen(pOptions->strResultsFile), wcslen(pOptions->strResultsFile) );
//    SendMessage( GetDlgItem( hDlg, IDC_OUTPUT_EDIT ), EM_SCROLLCARET, 0, 0 );

//    CheckDlgButton( hDlg, IDC_COMPRESSED_CHECK, pOptions->bSaveCompressedResults ? BST_CHECKED : BST_UNCHECKED );
 //   CheckRadioButton( hDlg, IDC_MESH_SAVE_BINARY_RADIO, IDC_MESH_SAVE_TEXT_RADIO, pOptions->bBinaryOutputXFile ? IDC_MESH_SAVE_BINARY_RADIO : IDC_MESH_SAVE_TEXT_RADIO );

//    CheckDlgButton( hDlg, IDC_COMPRESSED_CHECK, pOptions->bSaveCompressedResults ? BST_CHECKED : BST_UNCHECKED );

//    CheckDlgButton( hDlg, IDC_TOOLTIPS, m_bShowTooltips ? BST_CHECKED : BST_UNCHECKED );
}


//--------------------------------------------------------------------------------------
CPRTLoadDlg::CPRTLoadDlg()
{
    g_pLoadDlg = this;
}


//--------------------------------------------------------------------------------------
CPRTLoadDlg::~CPRTLoadDlg()
{
    g_pLoadDlg = NULL;
}


//--------------------------------------------------------------------------------------
bool CPRTLoadDlg::Show()
{
    // Ask the user about param settings for the PRT Simulation
    int nResult = (int) DialogBox( g_hInstance, MAKEINTRESOURCE(IDD_SH), 
                                   0, StaticDlgProc );

    UnhookWindowsHookEx( m_hMsgProcHook );
    DestroyWindow( m_hToolTip );

    if( nResult == IDOK )
        return true;
    else
        return false;
}


//--------------------------------------------------------------------------------------
INT_PTR CALLBACK CPRTLoadDlg::StaticDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    return g_pLoadDlg->DlgProc( hDlg, msg, wParam, lParam );
}


//--------------------------------------------------------------------------------------
INT_PTR CALLBACK CPRTLoadDlg::DlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_INITDIALOG:
        {
            m_hDlg = hDlg;

            m_hToolTip = CreateWindowEx( 0, TOOLTIPS_CLASS, NULL, TTS_ALWAYSTIP, 
                                         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                                         hDlg, NULL, (HINSTANCE)g_hInstance, NULL );                            
            SendMessage( m_hToolTip, TTM_SETMAXTIPWIDTH, 0, 300 );
            SendMessage( m_hToolTip, TTM_SETDELAYTIME, (WPARAM)(DWORD) TTDT_AUTOPOP, 32000 );
            SendMessage( m_hToolTip, TTM_SETDELAYTIME, (WPARAM)(DWORD) TTDT_INITIAL, 0 );
            SendMessage( m_hToolTip, TTM_SETDELAYTIME, (WPARAM)(DWORD) TTDT_RESHOW, 0 );
            EnumChildWindows( hDlg, (WNDENUMPROC) EnumChildProc, 0);
            m_hMsgProcHook = SetWindowsHookEx( WH_GETMESSAGE, GetMsgProc, (HINSTANCE)g_hInstance, GetCurrentThreadId() );

            return TRUE;
        }
        
        case WM_NOTIFY:
        {
            NMHDR* pNMHDR = (LPNMHDR) lParam;
            if( pNMHDR == NULL )
                break;

            if( pNMHDR->code == TTN_NEEDTEXT )
            {
                NMTTDISPINFO* pNMTDI = (LPNMTTDISPINFO)lParam;
                int nDlgId = GetDlgCtrlID( (HWND)pNMHDR->idFrom );
                GetToolTipText( nDlgId, pNMTDI );
                return TRUE;
            }
            break;
        }

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {

            }
            break;

        case WM_CLOSE:
            break;
    }

    return FALSE;
}


//--------------------------------------------------------------------------------------
void CPRTLoadDlg::GetToolTipText( int nDlgId, NMTTDISPINFO* pNMTDI )
{
}


//--------------------------------------------------------------------------------------
LRESULT CALLBACK CPRTLoadDlg::GetMsgProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    return 0;
}


//--------------------------------------------------------------------------------------
BOOL CALLBACK CPRTLoadDlg::EnumChildProc( HWND hwnd, LPARAM lParam )
{
    return false;
}

//--------------------------------------------------------------------------------------
CPRTAdaptiveOptionsDlg::CPRTAdaptiveOptionsDlg()
{
    g_pAdaptiveDlg = this;
}


//--------------------------------------------------------------------------------------
CPRTAdaptiveOptionsDlg::~CPRTAdaptiveOptionsDlg()
{
    g_pAdaptiveDlg = NULL;
}


//--------------------------------------------------------------------------------------
bool CPRTAdaptiveOptionsDlg::Show( HWND hDlg )
{
    // Ask the user about param settings for the PRT Simulation
    int nResult = (int) DialogBox( g_hInstance, MAKEINTRESOURCE(IDD_ADAPTIVE_OPTIONS), 
                                   hDlg, StaticDlgProc );

    UnhookWindowsHookEx( m_hMsgProcHook );
    DestroyWindow( m_hToolTip );

    if( nResult == IDOK )
        return true;
    else
        return false;
}


//--------------------------------------------------------------------------------------
INT_PTR CALLBACK CPRTAdaptiveOptionsDlg::StaticDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    return g_pAdaptiveDlg->DlgProc( hDlg, msg, wParam, lParam );
}


//--------------------------------------------------------------------------------------
INT_PTR CALLBACK CPRTAdaptiveOptionsDlg::DlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_INITDIALOG:
        {
            m_hDlg = hDlg;

            SIMULATOR_OPTIONS& options = GetGlobalOptions();

            CheckDlgButton( hDlg, IDC_ENABLE_ROBUST_MESH_REFINE, options.bRobustMeshRefine ? BST_CHECKED : BST_UNCHECKED );
            CheckDlgButton( hDlg, IDC_ENABLE_ADAPTIVE_DIRECT_LIGHTING, options.bAdaptiveDL ? BST_CHECKED : BST_UNCHECKED );
            CheckDlgButton( hDlg, IDC_ENABLE_ADAPTIVE_BOUNCE, options.bAdaptiveBounce ? BST_CHECKED : BST_UNCHECKED );

            UpdateUI( hDlg );

            HWND hSpin;
            hSpin = GetDlgItem( hDlg, IDC_RMR_MAX_SUBD_SPIN );
            SendMessage( hSpin, UDM_SETRANGE, 0, (LPARAM) MAKELONG( 10, 1 ) );
            SendMessage( hSpin, UDM_SETPOS, 0, (LPARAM) MAKELONG( GetGlobalOptions().dwRobustMeshRefineMaxSubdiv, 0) );  

            hSpin = GetDlgItem( hDlg, IDC_DL_MAX_SUBD_SPIN );
            SendMessage( hSpin, UDM_SETRANGE, 0, (LPARAM) MAKELONG( 10, 1 ) );
            SendMessage( hSpin, UDM_SETPOS, 0, (LPARAM) MAKELONG( GetGlobalOptions().dwAdaptiveDLMaxSubdiv, 0) );  

            hSpin = GetDlgItem( hDlg, IDC_AB_MAX_SUBD_SPIN );
            SendMessage( hSpin, UDM_SETRANGE, 0, (LPARAM) MAKELONG( 10, 1 ) );
            SendMessage( hSpin, UDM_SETPOS, 0, (LPARAM) MAKELONG( GetGlobalOptions().dwAdaptiveBounceMaxSubdiv, 0) );  

            WCHAR sz[256];
            _snwprintf( sz, 256, L"%0.6f", GetGlobalOptions().fRobustMeshRefineMinEdgeLength ); sz[255] = 0; SetDlgItemText( hDlg, IDC_RMR_MIN_EDGE_EDIT, sz ); 
            _snwprintf( sz, 256, L"%0.6f", GetGlobalOptions().fAdaptiveDLMinEdgeLength ); sz[255] = 0; SetDlgItemText( hDlg, IDC_DL_MIN_EDGE_EDIT, sz ); 
            _snwprintf( sz, 256, L"%0.6f", GetGlobalOptions().fAdaptiveBounceMinEdgeLength ); sz[255] = 0; SetDlgItemText( hDlg, IDC_AB_MIN_EDGE_EDIT, sz ); 
            _snwprintf( sz, 256, L"%0.6f", GetGlobalOptions().fAdaptiveDLThreshold ); sz[255] = 0; SetDlgItemText( hDlg, IDC_DL_SUBD_THRESHOLD_EDIT, sz ); 
            _snwprintf( sz, 256, L"%0.6f", GetGlobalOptions().fAdaptiveBounceThreshold ); sz[255] = 0; SetDlgItemText( hDlg, IDC_AB_SUBD_THRESHOLD_EDIT, sz ); 

            m_hToolTip = CreateWindowEx( 0, TOOLTIPS_CLASS, NULL, TTS_ALWAYSTIP, 
                                         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                                         hDlg, NULL, (HINSTANCE)g_hInstance, NULL );                            
            SendMessage( m_hToolTip, TTM_SETMAXTIPWIDTH, 0, 300 );
            SendMessage( m_hToolTip, TTM_SETDELAYTIME, (WPARAM)(DWORD) TTDT_AUTOPOP, 32000 );
            SendMessage( m_hToolTip, TTM_SETDELAYTIME, (WPARAM)(DWORD) TTDT_INITIAL, 0 );
            SendMessage( m_hToolTip, TTM_SETDELAYTIME, (WPARAM)(DWORD) TTDT_RESHOW, 0 );
            EnumChildWindows( hDlg, (WNDENUMPROC) EnumChildProc, 0);
            m_hMsgProcHook = SetWindowsHookEx( WH_GETMESSAGE, GetMsgProc, (HINSTANCE)g_hInstance, GetCurrentThreadId() );

            return TRUE;
        }
        
        case WM_NOTIFY:
        {
            NMHDR* pNMHDR = (LPNMHDR) lParam;
            if( pNMHDR == NULL )
                break;

            if( pNMHDR->code == TTN_NEEDTEXT )
            {
                NMTTDISPINFO* pNMTDI = (LPNMTTDISPINFO)lParam;
                int nDlgId = GetDlgCtrlID( (HWND)pNMHDR->idFrom );
                GetToolTipText( nDlgId, pNMTDI );
                return TRUE;
            }
            break;
        }

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_ENABLE_ROBUST_MESH_REFINE:
                case IDC_ENABLE_ADAPTIVE_DIRECT_LIGHTING:
                case IDC_ENABLE_ADAPTIVE_BOUNCE:
                {
                    GetGlobalOptions().bRobustMeshRefine = (IsDlgButtonChecked( hDlg, IDC_ENABLE_ROBUST_MESH_REFINE ) == BST_CHECKED );
                    GetGlobalOptions().bAdaptiveDL       = (IsDlgButtonChecked( hDlg, IDC_ENABLE_ADAPTIVE_DIRECT_LIGHTING ) == BST_CHECKED );
                    GetGlobalOptions().bAdaptiveBounce   = (IsDlgButtonChecked( hDlg, IDC_ENABLE_ADAPTIVE_BOUNCE ) == BST_CHECKED );
                    UpdateUI( hDlg );
                    break;
                }

                case IDOK:
                {
                    GetGlobalOptions().bRobustMeshRefine = (IsDlgButtonChecked( hDlg, IDC_ENABLE_ROBUST_MESH_REFINE ) == BST_CHECKED );
                    GetGlobalOptions().bAdaptiveDL       = (IsDlgButtonChecked( hDlg, IDC_ENABLE_ADAPTIVE_DIRECT_LIGHTING ) == BST_CHECKED );
                    GetGlobalOptions().bAdaptiveBounce   = (IsDlgButtonChecked( hDlg, IDC_ENABLE_ADAPTIVE_BOUNCE ) == BST_CHECKED );

                    GetGlobalOptions().dwRobustMeshRefineMaxSubdiv = (DWORD) SendMessage( GetDlgItem( hDlg, IDC_RMR_MAX_SUBD_SPIN ), UDM_GETPOS, 0, 0 );
                    GetGlobalOptions().dwAdaptiveDLMaxSubdiv = (DWORD) SendMessage( GetDlgItem( hDlg, IDC_DL_MAX_SUBD_SPIN ), UDM_GETPOS, 0, 0 );
                    GetGlobalOptions().dwAdaptiveBounceMaxSubdiv = (DWORD) SendMessage( GetDlgItem( hDlg, IDC_AB_MAX_SUBD_SPIN ), UDM_GETPOS, 0, 0 );

                    WCHAR sz[256];
                    GetDlgItemText( hDlg, IDC_RMR_MIN_EDGE_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().fRobustMeshRefineMinEdgeLength ) == 0 ) return false;
                    GetDlgItemText( hDlg, IDC_DL_MIN_EDGE_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().fAdaptiveDLMinEdgeLength ) == 0 ) return false;
                    GetDlgItemText( hDlg, IDC_AB_MIN_EDGE_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().fAdaptiveBounceMinEdgeLength ) == 0 ) return false;
                    GetDlgItemText( hDlg, IDC_DL_SUBD_THRESHOLD_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().fAdaptiveDLThreshold ) == 0 ) return false;
                    GetDlgItemText( hDlg, IDC_AB_SUBD_THRESHOLD_EDIT, sz, 256 ); if( swscanf( sz, TEXT("%f"), &GetGlobalOptions().fAdaptiveBounceThreshold ) == 0 ) return false;

                    EndDialog(hDlg, IDOK);
                    break;
                }

                case IDCANCEL:
                {
                    EndDialog(hDlg, IDCANCEL);
                    break;
                }
            }
            break;

        case WM_CLOSE:
            break;
    }

    return FALSE;
}

//--------------------------------------------------------------------------------------
void CPRTAdaptiveOptionsDlg::UpdateUI( HWND hDlg )
{
    SIMULATOR_OPTIONS& options = GetGlobalOptions();
    EnableWindow( GetDlgItem( hDlg, IDC_RMR_MIN_EDGE_EDIT ), options.bRobustMeshRefine );
    EnableWindow( GetDlgItem( hDlg, IDC_RMR_MIN_EDGE_TEXT ), options.bRobustMeshRefine );
    EnableWindow( GetDlgItem( hDlg, IDC_RMR_MAX_SUBD_EDIT ), options.bRobustMeshRefine );
    EnableWindow( GetDlgItem( hDlg, IDC_RMR_MAX_SUBD_SPIN ), options.bRobustMeshRefine );
    EnableWindow( GetDlgItem( hDlg, IDC_RMR_MAX_SUBD_TEXT ), options.bRobustMeshRefine );

    EnableWindow( GetDlgItem( hDlg, IDC_DL_MAX_SUBD_EDIT ), options.bAdaptiveDL );
    EnableWindow( GetDlgItem( hDlg, IDC_DL_MAX_SUBD_SPIN ), options.bAdaptiveDL );
    EnableWindow( GetDlgItem( hDlg, IDC_DL_MAX_SUBD_TEXT ), options.bAdaptiveDL );
    EnableWindow( GetDlgItem( hDlg, IDC_DL_MIN_EDGE_EDIT ), options.bAdaptiveDL );
    EnableWindow( GetDlgItem( hDlg, IDC_DL_MIN_EDGE_TEXT ), options.bAdaptiveDL );
    EnableWindow( GetDlgItem( hDlg, IDC_DL_SUBD_THRESHOLD_EDIT ), options.bAdaptiveDL );
    EnableWindow( GetDlgItem( hDlg, IDC_DL_SUBD_THRESHOLD_TEXT ), options.bAdaptiveDL );

    EnableWindow( GetDlgItem( hDlg, IDC_AB_MAX_SUBD_EDIT ), options.bAdaptiveBounce );
    EnableWindow( GetDlgItem( hDlg, IDC_AB_MAX_SUBD_SPIN ), options.bAdaptiveBounce );
    EnableWindow( GetDlgItem( hDlg, IDC_AB_MAX_SUBD_TEXT ), options.bAdaptiveBounce );
    EnableWindow( GetDlgItem( hDlg, IDC_AB_MIN_EDGE_EDIT ), options.bAdaptiveBounce );
    EnableWindow( GetDlgItem( hDlg, IDC_AB_MIN_EDGE_TEXT ), options.bAdaptiveBounce );
    EnableWindow( GetDlgItem( hDlg, IDC_AB_SUBD_THRESHOLD_EDIT ), options.bAdaptiveBounce );
    EnableWindow( GetDlgItem( hDlg, IDC_AB_SUBD_THRESHOLD_TEXT ), options.bAdaptiveBounce );
}


//--------------------------------------------------------------------------------------
void CPRTAdaptiveOptionsDlg::GetToolTipText( int nDlgId, NMTTDISPINFO* pNMTDI )
{
}


//--------------------------------------------------------------------------------------
LRESULT CALLBACK CPRTAdaptiveOptionsDlg::GetMsgProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    return 0;
}


//--------------------------------------------------------------------------------------
BOOL CALLBACK CPRTAdaptiveOptionsDlg::EnumChildProc( HWND hwnd, LPARAM lParam )
{
    return false;
}


//--------------------------------------------------------------------------------------
void CXMLHelper::CreateChildNode( IXMLDOMDocument* pDoc, IXMLDOMNode* pParentNode, 
                                  WCHAR* strName, int nType, IXMLDOMNode** ppNewNode )
{
    IXMLDOMNode* pNewNode;
    VARIANT vtype;
    vtype.vt = VT_I4;
    V_I4(&vtype) = (int)nType;
    BSTR bstrName = SysAllocString(strName);    
    pDoc->createNode( vtype, bstrName, NULL, &pNewNode );
    SysFreeString( bstrName );
    pParentNode->appendChild( pNewNode, ppNewNode );
    SAFE_RELEASE( pNewNode );
}


//--------------------------------------------------------------------------------------
void CXMLHelper::CreateNewValue( IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, WCHAR* strName, WCHAR* strValue )
{
    IXMLDOMNode* pNewNode = NULL;
    IXMLDOMNode* pNewTextNode = NULL;
    CreateChildNode( pDoc, pNode, strName, NODE_ELEMENT, &pNewNode ); 
    CreateChildNode( pDoc, pNewNode, strName, NODE_TEXT, &pNewTextNode );
    VARIANT var;
    var.vt = VT_BSTR;
    var.bstrVal = SysAllocString( strValue );
    pNewTextNode->put_nodeTypedValue( var );
    VariantClear( &var );
    SAFE_RELEASE( pNewTextNode );
    SAFE_RELEASE( pNewNode );
}


//--------------------------------------------------------------------------------------
void CXMLHelper::CreateNewValue( IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, WCHAR* strName, DWORD nValue )
{
    WCHAR strValue[MAX_PATH];
    swprintf( strValue, L"%d", nValue );
    CreateNewValue( pDoc, pNode, strName, strValue );
}


//--------------------------------------------------------------------------------------
void CXMLHelper::CreateNewValue( IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, WCHAR* strName, float fValue )
{
    WCHAR strValue[MAX_PATH];
    swprintf( strValue, L"%f", fValue );
    CreateNewValue( pDoc, pNode, strName, strValue );
}


//--------------------------------------------------------------------------------------
void CXMLHelper::GetValue( IXMLDOMNode* &pNode, WCHAR* strName, WCHAR* strValue )
{
    if( !pNode )
        return;

    BSTR nodeName;
    pNode->get_nodeName(&nodeName);
    if( wcscmp( nodeName, strName ) == 0 )
    {
        VARIANT v;
        IXMLDOMNode* pChild = NULL;    
        pNode->get_firstChild(&pChild);
        if( pChild )
        {
            HRESULT hr = pChild->get_nodeTypedValue(&v);
            if( SUCCEEDED(hr) && v.vt == VT_BSTR )
                wcscpy( strValue, v.bstrVal );
            VariantClear(&v);
            SAFE_RELEASE( pChild );
        }
    }
    SysFreeString(nodeName);

    IXMLDOMNode* pNextNode = NULL;    
    if( SUCCEEDED( pNode->get_nextSibling(&pNextNode) ) )
    {
        SAFE_RELEASE( pNode );
        pNode = pNextNode;
    }
}


//--------------------------------------------------------------------------------------
void CXMLHelper::GetValue( IXMLDOMNode* &pNode, WCHAR* strName, int* pnValue )
{
    if( NULL == pNode )
        return;
    WCHAR strValue[256];
    GetValue( pNode, strName, strValue );
    *pnValue = _wtoi(strValue);
}


//--------------------------------------------------------------------------------------
void CXMLHelper::GetValue( IXMLDOMNode* &pNode, WCHAR* strName, float* pfValue )
{
    WCHAR strValue[256];
    GetValue( pNode, strName, strValue );
    if( swscanf( strValue, L"%f", pfValue ) == 0 )
        *pfValue = 0;
}


//--------------------------------------------------------------------------------------
void CXMLHelper::GetValue( IXMLDOMNode* &pNode, WCHAR* strName, bool* pbValue )
{
    int n;
    GetValue( pNode, strName, &n );
    *pbValue = (n==1);
}


//--------------------------------------------------------------------------------------
void CXMLHelper::GetValue( IXMLDOMNode* &pNode, WCHAR* strName, D3DXCOLOR* pclrValue )
{
    int n;
    GetValue( pNode, strName, &n );
    *pclrValue = (D3DXCOLOR)n;
}


//--------------------------------------------------------------------------------------
void CXMLHelper::GetValue( IXMLDOMNode* &pNode, WCHAR* strName, DWORD* pdwValue )
{
    WCHAR strValue[256];
    CXMLHelper::GetValue( pNode, strName, strValue );
    if( swscanf( strValue, L"%d", pdwValue ) == 0 )
        *pdwValue = 0;
}



SIMULATOR_OPTIONS* CPRTOptionsDlg::GetOptions()
{
    return &g_OptionsFile.m_Options;
}

SIMULATOR_OPTIONS* CPRTLoadDlg::GetOptions()
{
    return &g_OptionsFile.m_Options;
}


