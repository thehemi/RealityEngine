#pragma once
#ifndef __DXCC_H__
#define __DXCC_H__


//#ifndef WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN
//#endif

//unreferenced label
#pragma warning(disable : 4102)
//if statement is constant
#pragma warning(disable : 4127)
//unreachable code
#pragma warning(disable : 4702)
//local variable initialized but not used
#pragma warning(disable : 4189)
//formal parameter unreferenced
#pragma warning(disable : 4100)
//local variable unreferenced
#pragma warning(disable : 4100)
//local variable unreferenced
#pragma warning(disable : 4101)
//conversion truncation
#pragma warning(disable : 4311)
//conversion has more room than expection
#pragma warning(disable : 4312)
//conversion possible loss of data
#pragma warning(disable : 4244)
//return conversion possible loss of data
#pragma warning(disable : 4267)



#include "initguid.h"
#include "comdef.h"


#include "d3d9.h"
#include "d3dx9.h"
#include <stdlib.h>

//#define COM_NO_WINDOWS_H
//#include <objbase.h>
//#include <windows.h>
//#if defined( _WIN32 ) && !defined( _NO_COM)

typedef interface	IDXCCHeap					IDXCCHeap						,*LPDXCCHEAP;
typedef interface	IDXCCResource				IDXCCResource					,*LPDXCCRESOURCE;
typedef interface	IDXCCManager				IDXCCManager					,*LPDXCCMANAGER;
typedef interface	IDXCCFrame					IDXCCFrame						,*LPDXCCFRAME;
typedef interface	IDXCCFrameIterator			IDXCCFrameIterator				,*LPDXCCFRAMEITERATOR;
typedef interface	IDXCCMesh					IDXCCMesh						,*LPDXCCMESH;
typedef interface	IDXCCDeclaration			IDXCCDeclaration				,*LPDXCCDECLARATION;
typedef interface	IDXCCVertexBundler			IDXCCVertexBundler				,*LPDXCCVERTEXBUNDLER;
//typedef interface	IDXCCMaterial				IDXCCMaterial					,*LPDXCCMATERIAL;
typedef interface	IDXCCAnimationStream		IDXCCAnimationStream			,*LPDXCCANIMATIONSTREAM;

typedef interface	IDXCCPropertyBag			IDXCCPropertyBag				,*LPDXCCPROPERTYBAG;


typedef interface	IDXCCSaveUserData			IDXCCSaveUserData				,*LPDXCCSAVEUSERDATA;
typedef interface	IDXCCEnumEffectParameters	IDXCCEnumEffectParameters		,*LPDXCCENUMEFFECTPARAMETERS;
typedef interface	IDXCCEffectDlgCallbacks		IDXCCEffectDlgCallbacks			,*LPDXCCFXDLGCALLBACKS;
typedef interface	IDXCCHierarchyCallbacks		IDXCCHierarchyCallbacks			,*LPDXCCHIERARCHYCALLBACKS;

//pointers to internal data
typedef const		ID3DXMesh*					LPCD3DXMESH;
typedef const		ID3DXSkinInfo*				LPCD3DXSKININFO;

typedef UINT		DXCCHANDLE;
typedef DWORD		DXCCATTRIBUTE;
typedef UINT_PTR	DXCCATTRIBUTE3264;
typedef UINT64		DXCCATTRIBUTE64;

typedef struct		DXCCSaveSceneArgs			DXCCSaveSceneArgs				,*LPDXCCSAVESCENEARGS;

interface __declspec( uuid("640ed315-dec3-477e-84ce-17d7ce2f11da") ) IDXCCMesh;
DEFINE_GUID( IID_IDXCCMesh, 
			0x640ed315, 
			0xdec3, 
			0x477e, 
			0x84, 0xce, 0x17, 0xd7, 0xce, 0x2f, 0x11, 0xda);

interface __declspec( uuid("f9acf835-d202-4ccb-a42c-6bb79cafdc48") ) IDXCCFrame;
DEFINE_GUID( IID_IDXCCFrame, 
			0xf9acf835, 
			0xd202, 
			0x4ccb, 
			0xa4, 0x2c, 0x6b, 0xb7, 0x9c, 0xaf, 0xdc, 0x48);

interface __declspec( uuid("8d7815e8-3589-4938-bdab-366dbc57424d") ) IDXCCDeclaration;
DEFINE_GUID( IID_IDXCCDeclaration, 
	0x8d7815e8, 
	0x3589, 
	0x4938, 
	0xbd, 0xab, 0x36, 0x6d, 0xbc, 0x57, 0x42, 0x4d);

interface __declspec( uuid("8dd12056-b4e3-4195-976a-5fc56c2d8ae0") ) IDXCCHeap;
DEFINE_GUID( IID_IDXCCHeap, 
	0x8dd12056, 
	0xb4e3, 
	0x4195, 
	0x97, 0x6a, 0x5f, 0xc5, 0x6c, 0x2d, 0x8a, 0xe0);

interface __declspec( uuid("c39daab3-5e71-43ba-9218-d25c947d746f") ) IDXCCVertexBundler;
DEFINE_GUID( IID_IDXCCVertexBundler, 
			0xc39daab3, 
			0x5e71, 
			0x43ba, 
			0x92, 0x18, 0xd2, 0x5c, 0x94, 0x7d, 0x74, 0x6f);

interface __declspec( uuid("435d9fb7-e9df-4d5e-ab69-d7cf54332d4e") ) IDXCCManager;
DEFINE_GUID( IID_IDXCCManager,
			0x435d9fb7, 
			0xe9df, 
			0x4d5e, 
			0xab, 0x69, 0xd7, 0xcf, 0x54, 0x33, 0x2d, 0x4e);

interface __declspec( uuid("75ebc313-afa7-4f1b-80f7-2b1256a28b29") ) IDXCCResource;
DEFINE_GUID( IID_IDXCCResource, 
			0x75ebc313, 
			0xafa7, 
			0x4f1b, 
			0x80, 0xf7, 0x2b, 0x12, 0x56, 0xa2, 0x8b, 0x29);


interface __declspec( uuid("ae831c8a-7d7d-4959-be8f-0f5b82699367") ) IDXCCAnimationStream;
DEFINE_GUID( IID_IDXCCAnimationStream,
    0xae831c8a,
    0x7d7d,
    0x4959,
    0xbe, 0x8f, 0x0f, 0x5b, 0x82, 0x69, 0x93, 0x67);

interface __declspec( uuid("64a4b9a0-58d4-4d00-9e8f-14bc96fab35c") ) IDXCCFrameIterator;
DEFINE_GUID( IID_IDXCCFrameIterator, 
    0x64a4b9a0,
    0x58d4,
    0x4d00,
    0x9e, 0x8f, 0x14, 0xbc, 0x96, 0xfa, 0xb3, 0x5c);


interface __declspec( uuid("bb54ea69-4bf3-45ab-a8e5-f8149022912c") ) IDXCCPropertyBag;
DEFINE_GUID( IID_IDXCCPropertyBag,
			0xbb54ea69, 
			0x4bf3, 
			0x45ab, 
			0xa8, 0xe5, 0xf8, 0x14, 0x90, 0x22, 0x91, 0x2c);


#define DXCC_DEFAULT_STRING_LENGTH 256

#ifdef _DEBUG
	#define DXCC_DPFA_MESSAGE					DXCCDebugPrintfMessageA
	#define DXCC_DPFW_MESSAGE					DXCCDebugPrintfMessageW
	#define DXCC_DPFA_SHORT						DXCCDebugPrintfA
	#define DXCC_DPFW_SHORT						DXCCDebugPrintfW
	#define DXCC_DPFA_REPORT					DXCC_DPF_HEADER(TEXT("INFO")), DXCC_DPFA_MESSAGE
	#define DXCC_DPFW_REPORT					DXCC_DPF_HEADER(TEXT("INFO")), DXCC_DPFW_MESSAGE
	#define DXCC_DPFA_ERROR						DXCC_DPF_HEADER(TEXT("ERROR")), DXCC_DPFA_MESSAGE
	#define DXCC_DPFW_ERROR						DXCC_DPF_HEADER(TEXT("ERROR")), DXCC_DPFW_MESSAGE
	#define DXCC_DPFA							DXCC_DPFA_REPORT
	#define DXCC_DPFW							DXCC_DPFW_REPORT
	#define DXCC_DPF_HEADER(_type_)				DXCCDebugPrintfHeader( _type_, TEXT( __FILE__ ) , __LINE__, TEXT( __FUNCTION__ ) )
	#define DXCC_DPF_STATUS( _status_ )			DXCCDebugPrintfHRESULT(_status_)

	#define DXCC_ASSERT(_exp_)					do{ if(!(_exp_)){ DXCC_DPF_HEADER(TEXT("ASSERT")); DXCC_DPF_MESSAGE(TEXT( #_exp_ )); _asm{int 3};} }while(0)
	#define DXCC_ASSERT_RANGE(min, exp, max)	DXCC_ASSERT(((min) >= (exp))&&((exp) <= (max)))
	#define DXCC_ASSERT_EXCEPTIONS_BEGIN()		try{
	#define DXCC_ASSERT_EXCEPTIONS_END()		}catch(...){DXCC_ASSERT(FALSE);throw;}
	#define DXCC_SUCCEEDED(_hr_)				SUCCEEDED(_hr_)
	#define DXCC_FAILED(_hr_)					( FAILED(_hr_) ? DXCC_DPF_STATUS(_hr_), TRUE : FALSE )

	#ifdef _MEMDEBUG
		#define DXCCCREATEBUFFER(_size_, _ppBuf_)										\
													(DXCCDebugPrintfHeader( TEXT("NEW BUFFER"), TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__)),	\
													DXCC_DPF_MESSAGE( TEXT("SIZE:%d"), (_size_)),										\
													D3DXCreateBuffer((_size_), (_ppBuf_)))
	#else 
		#define DXCCCREATEBUFFER(_size_, _ppBuf_)	D3DXCreateBuffer(_size_, _ppBuf_)

	#endif

#else 
	#define DXCC_DPFA_MESSAGE					
	#define DXCC_DPFW_MESSAGE					
	#define DXCC_DPFA_REPORT						
	#define DXCC_DPFW_REPORT						
	#define DXCC_DPFA_ERROR						
	#define DXCC_DPFW_ERROR						
	#define DXCC_DPFA_SHORT	
	#define DXCC_DPFW_SHORT
	#define DXCC_DPFA	
	#define DXCC_DPFW	
	#define DXCC_DPF_HEADER(_type_)	
	#define DXCC_DPF_STATUS(_status_)			

    #define DXCC_ASSERT(_exp_)	
	#define DXCC_ASSERT_RANGE(min, exp, max)
	#define DXCC_ASSERT_EXCEPTIONS_BEGIN()			
	#define DXCC_ASSERT_EXCEPTIONS_END()			
	#define DXCC_SUCCEEDED(_hr_)				SUCCEEDED(_hr_)
	#define DXCC_FAILED(_hr_)					FAILED(_hr_)

	#define DXCCCREATEBUFFER(_size_, _ppBuf_)	D3DXCreateBuffer(_size_, _ppBuf_)

#endif


#define DXCC_STATUS_EXIT(_hr,_Value,_e_Exit,_Break)		do{ (_hr)=(_Value); DXCC_GOTO_EXIT(_e_Exit, _Break); }while(0)
#define DXCC_GOTO_EXIT(_e_Exit, _Break)					do{ DXCC_ASSERT(!(_Break)); goto _e_Exit; }while(0)

#define DXCC_RELEASE(pIUK)								DXCCInterfaceRelease((LPUNKNOWN*)&(pIUK))

//#define DXCCFXDLGEFFECTLOADPROC DXCCFxDlgEffectLoadProc
//#define DXCCFXDLGEFFECTCHANGEPROC DXCCFxDlgEffectChangeProc
//#define DXCCFXDLGPARAMETERCHANGEPROC DXCCFxDlgParameterChangeProc

inline ULONG DXCCInterfaceRelease(LPUNKNOWN* ppUnknown);

VOID cdecl DXCCDebugPrintfHRESULT(HRESULT hr);
VOID cdecl DXCCDebugPrintfHeaderA(LPCSTR strReportType, LPCSTR strFile, UINT nLine, LPCSTR strFunction);
VOID cdecl DXCCDebugPrintfHeaderW(LPCWSTR strReportType, LPCWSTR strFile, UINT nLine, LPCWSTR strFunction);
VOID cdecl DXCCDebugPrintfMessageA(LPCSTR szFormat, ...);
VOID cdecl DXCCDebugPrintfMessageW(LPCWSTR szFormat, ...);
VOID cdecl DXCCDebugPrintfA(LPCSTR szFormat, ...);
VOID cdecl DXCCDebugPrintfW(LPCWSTR szFormat, ...);

#ifndef _UNICODE
	#define DXCC_DPF_MESSAGE	DXCC_DPFA_MESSAGE
	#define DXCC_DPF_REPORT		DXCC_DPFA_REPORT
	#define DXCC_DPF_ERROR		DXCC_DPFA_ERROR
	#define DXCC_DPF			DXCC_DPFA
	#define DXCCDebugPrintfHeader DXCCDebugPrintfHeaderA
	#define DXCCDebugPrintfMessage DXCCDebugPrintfMessageA
	#define DXCCDebugPrintf DXCCDebugPrintfA
#else
	#define DXCC_DPF_MESSAGE	DXCC_DPFW_MESSAGE
	#define DXCC_DPF_REPORT		DXCC_DPFW_REPORT
	#define DXCC_DPF_ERROR		DXCC_DPFW_ERROR
	#define DXCC_DPF			DXCC_DPFW
	#define DXCCDebugPrintfHeader DXCCDebugPrintfHeaderW
	#define DXCCDebugPrintfMessage DXCCDebugPrintfMessageW
	#define DXCCDebugPrintf DXCCDebugPrintfW
#endif


//typedef HRESULT (CALLBACK* DXCCFxDlgEffectLoadProc)(
// 	HWND hDialog, 
// 	LPDXCCMANAGER pMyManager,
// 	LPDIRECT3DDEVICE9 pDevice,
// 	LPCSTR szNewEffectFilename,
//	LPD3DXEFFECT* ppNewEffect);
//
//typedef HRESULT (CALLBACK* DXCCFxDlgEffectChangeProc)(
// 	HWND hDialog,  
// 	LPDXCCMANAGER pMyManager,
// 	LPDIRECT3DDEVICE9 pDevice,
// 	LPD3DXEFFECT pNewEffect,
//	LPDXCCRESOURCE pNewEffectResource);
//
//typedef HRESULT (CALLBACK* DXCCFxDlgParameterChangeProc)(
// 	HWND hDialog, 
// 	LPDXCCMANAGER pMyManager, 
// 	LPDIRECT3DDEVICE9 pDevice,
// 	LPD3DXEFFECT pEffect, 
// 	LPDXCCRESOURCE pEffectResource,
// 	HWND hParamterWindow, 
// 	D3DXHANDLE hRootParameter,
// 	D3DXHANDLE hNewParameter);


#ifdef DXCC_EXPORTS
	#define EXPORTDEF __declspec(dllexport)
#else
	#define EXPORTDEF
#endif

enum DXCCIteratorType
{
	DXCCITERATOR_DEPTHFIRST,
	DXCCITERATOR_BREADTH,
};

//enum DXCCTOD3DX_FLAGS
//{
//	DXCCTOD3DX_INTERFACE_NOADDREF=		0x1, //addref to any internal interfaces
//	DXCCTOD3DX_FRAME_NONAME=			0x2,
//	DXCCTOD3DX_FRAME_NOMESH=			0x4,
//	DXCCTOD3DX_MESH_NONAME=				0x8, 
//	DXCCTOD3DX_MESH_NOSKIN=				0x10,
//	DXCCTOD3DX_MESH_NOADJACENCIES= 		0x20,
//	DXCCTOD3DX_MESH_NOMATERIALS= 		0x40,
//	DXCCTOD3DX_MATERIAL_NOTEXTURES=		0x80,
//	DXCCTOD3DX_TEXTURE_UNQUALIFYPATH=	0x100,
//};

enum DXCC_PROPERTY_TYPE
{
	DXCC_PROPERTY_NONE= 0,
	DXCC_PROPERTY_BOOL,
	DXCC_PROPERTY_INT,
	DXCC_PROPERTY_FLOAT,
	DXCC_PROPERTY_STRING,
};


HINSTANCE DXCCGetHINSTANCE();
HMODULE DXCCGetHMODULE();


HRESULT WINAPI 
DXCCCreatePropertyBag(
	LPDXCCPROPERTYBAG* ppBag );

HRESULT WINAPI
	DXCCWindowlessSetup(
	HINSTANCE*			phOut_Instance,
	HWND*				phOut_Wnd,
	LPDIRECT3D9*		ppOut_D3D9,
	LPDIRECT3DDEVICE9*	ppOut_D3DDevice9);

HRESULT WINAPI
	DXCCEnumEffectParameters(
		LPD3DXEFFECT pEffect,
		LPDXCCENUMEFFECTPARAMETERS pEnum, 
		bool ContructFullNames,
		bool EnumLeavesOnly);

BOOL WINAPI
DXCCFindFile(
	const char* a_szFilepath, 
	const char* a_SecondaryDirectory, 
	DWORD a_FoundLength,
	char* a_FoundFilepath);

HRESULT WINAPI
DXSASLoadEffectDefaults(
		LPDIRECT3DDEVICE9 a_pDevice,
		LPDXCCMANAGER a_pManager,//for retrieval of effect and textures path
		LPD3DXEFFECT a_pEffect,
		LPDXCCRESOURCE a_pEffectResource,
		char* a_SecondaryDirectory);

HRESULT WINAPI
DXCCLoadTextureFromFile(
	LPDIRECT3DDEVICE9 a_pDevice, 
	LPDXCCMANAGER a_pManager,  
	const char* a_TexFilename,
	const char* a_SecondaryDirectory, 	
	LPDXCCRESOURCE* a_ppOutResource,//[out]
	LPDIRECT3DBASETEXTURE9* a_ppOutTexture);//[out]



D3DXHANDLE WINAPI
DXCCFindEffectAnnotation(
	LPD3DXEFFECT pEffect, 
	D3DXHANDLE hParameter, 
	LPCSTR szAnnotation);


//HRESULT WINAPI
//	DXCCCreateAnimationSet(
//	LPDXCCMANAGER pManager, 
//	LPDXCCFRAME pRoot, 
//	const char* SetName, 
//	DOUBLE TicksPerSecond, 
//	D3DXPLAYBACK_TYPE Playback,  
//	UINT NumCallbackKeys, 
//	CONST LPD3DXKEY_CALLBACK pCallKeys, 
//	LPD3DXKEYFRAMEDANIMATIONSET* ppAnimSet);

HRESULT WINAPI
	DXCCSaveScene(
 	LPCSTR Filename, 
 	DWORD XFormat, 
 	LPDXCCMANAGER pManager,
 	LPDXCCFRAME pRoot,
 	LPD3DXANIMATIONCONTROLLER pAnimation,		//LPD3DXKEYFRAMEDANIMIONSET or LPD3DXANIMATIONCONTROLLER
 	LPDXCCSAVESCENEARGS pSaveOptions,
 	LPDXCCSAVEUSERDATA pUserDataSaver);

HRESULT WINAPI 
	DXCCCreateManager(          
	LPDXCCMANAGER* ppManager);

HRESULT WINAPI
	DXCCCreateFrame(
	LPDXCCFRAME* ppFrame);

HRESULT WINAPI
	DXCCCreateMesh(
 	DWORD NumFaces,
 	DWORD NumVertices,
 	LPDIRECT3DDEVICE9 pDevice,
 	LPD3DVERTEXELEMENT9 pDeclaration,
	LPDXCCMESH* ppMesh);

//HRESULT WINAPI
//	DXCCCreateMaterial(
//	LPDXCCMATERIAL *pMaterial);

HRESULT WINAPI
	DXCCCreateVertexBundler(
 	UINT NumVertices,
 	D3DVERTEXELEMENT9* pDecl, 
	LPDXCCVERTEXBUNDLER* ppVBundle);

HRESULT WINAPI
	DXCCCreateDeclaration(
	LPDXCCDECLARATION* ppDeclaration);

HRESULT WINAPI
	DXCCCreateHeap(
	LPDXCCHEAP* ppHeap);

//HRESULT WINAPI
//	DXCCCreateAnimationStream(
//	LPDXCCANIMATIONSTREAM* ppOutAnimStream);	

////create a mesh container optimal for fast editing
//HRESULT WINAPI
//	DXCCCreateMeshFromD3DXMesh(
//		LPD3DXMESH pMesh,
//		//LPD3DXBUFFER pAdjacencies,
//		LPD3DXBUFFER pPointReps,
//		DWORD dxccOptions, 
//		LPDXCCMESH* ppMesh);
//
////HRESULT WINAPI
//	DXCCCreateMeshFromVertexStreamGroup(
//		LPDXCCVERTEXBUNDLER pVBundle,
//		LPDIRECT3DINDEXBUFFER9  pIndices,
//		LPD3DXBUFFER pAttributes,
//		LPD3DXBUFFER pAdjacencies,
//		LPD3DXBUFFER pPointReps,
//		DWORD dwD3DXoptions,
//		DWORD dxccOptions, 
//		LPDXCCMESH* ppMesh);
//

HRESULT WINAPI
	DXCCCreateFrameIterator(
	LPDXCCFRAMEITERATOR* ppIT);

//HRESULT WINAPI
//	DXCCCreateMeshIterator(
//	LPDXCCMESHITERATOR* ppIT);


//HRESULT WINAPI
//	DXCCCreateVertexBundlerFromD3DXMesh(
//		DWORD dxccOptions,
//		LPD3DXMESH pMesh,
//		LPDXCCVERTEXBUNDLER* ppVBundle);

LPSTR 
DXCCConvertStringToXFileObjectName(
	LPCSTR szString, 
	LPSTR szOutput, 
	UINT cOutputLength);

LPSTR 
DXCCConvertStringToXFilePath(
	LPCSTR szString, 
	LPSTR szOutput,
	UINT cOutputLength);

//HRESULT WINAPI
//DXCCShowSaveDialog(
//	DXCCMeshHierarchySave *pOptions);

HRESULT WINAPI
DXCCShowEffectDialog(
 	HWND hParent,
 	LPDIRECT3DDEVICE9 pDevice,
	LPDXCCMANAGER pManager,
 	LPDXCCRESOURCE pResource,
	LPDXCCFXDLGCALLBACKS pCallbacks,
 	BOOL bChangableEffect,
	BOOL bModal);


//Key an entire matrix rather than SRT
typedef struct DXCCKEY_MATRIX
{
    FLOAT Time;
    D3DXMATRIX Value;
} *LPDXCCKEY_MATRIX;

typedef struct DXCCKEY_TRANSFORM
{
    FLOAT Time;
    D3DXVECTOR3 ValueS;
    D3DXQUATERNION ValueR;
    D3DXVECTOR3 ValueT;
} *LPDXCCKEY_TRANSFORM;

struct DXCCSaveSceneArgs
{
	BOOL 							NoAddRefs;	
	BOOL 							NoAnimation;	
	BOOL 							NoFrameNames;		
	BOOL 							NoMeshNames;		
	BOOL 							NoMeshes;		
	BOOL 							NoMeshAdjacencies;	
	BOOL 							NoMeshSkins;	
	BOOL 							NoMeshMaterials;	
	BOOL							ReplaceEffectPaths;
	CHAR							ReplacementEffectPath[MAX_PATH];
	BOOL							ReplaceTexturePaths;
	CHAR							ReplacementTexturePath[MAX_PATH];
	BOOL 							RelativePaths;
	CHAR							RelativeToPath[MAX_PATH];
	//BOOL 							ResaveTexturesAsDDS;

	//BOOL							SkipVertexPositions;
	//BOOL							SkipVertexNormals;
	//BOOL							SkipVertexTangents;
	//BOOL							SkipVertexBinormals;
	//BOOL							SkipVertexColors;
	//BOOL							SkipVertexTextures;
	//BOOL							SkipVertexPointSizes;
	//BOOL							SkipVertexBlendWeights;
	//BOOL							SkipVertexBlendIndices;
};


//struct DXCCMeshHierarchySave
//{
//	BOOL 							Hierarchy;//export hieriarchy
//	BOOL 							HierarchyCentered;//add centering node
//	D3DXVECTOR3						HierarchyScale;//provide worldspace scaling for measurement changes
//	//BOOL 							HierarchyCollapsed;//collapse down to 1 node
//
//
//	BOOL 							Mesh;	//export meshes
//	BOOL 							MeshSkinned; //export skins with meshes
//	//BOOL 							MeshLegacyFormat; //export with legacy template formating
//	//LPDXCCDECLARATION				MeshDeclaration; //declaration restrictions //run update symantics on meshes?
//
//
//	BOOL 							Material; //export materials
//	BOOL 							MaterialFX; //export fx components  materials
//	//BOOL 							MaterialResave; //resave texture to new location
//	BOOL 							MaterialUnqualifyPath; //save filename references as unqualified paths
//	//BOOL 							MaterialDDS; //resave textures as dds
//
//	BOOL 							Animation; //export animation
//	LPD3DXANIMATIONCONTROLLER		AnimationController; //add animation to existing controller (multiset support)
//	char 							AnimationName[DXCC_DEFAULT_STRING_LENGTH]; //
//	BOOL							AnimationKeysOnly; //keyframes only
//	D3DXPLAYBACK_TYPE				AnimationPlaybackType; 
//	DOUBLE							AnimationSampleFPS;//TicksPerSecond;
//	DOUBLE							AnimationPlayFPS;//backRatio;
//	DOUBLE							AnimationStartTime;
//	DOUBLE							AnimationEndTime;
//	//BOOL							AnimationMatrixOnly; //as SRT
//	//BOOL							AnimationMultifile; //save each frame as it's own file
//
//	//BOOL 							OutputTemplates;
//	//BOOL 							OutputAuthorInfo;
//	DWORD 							OutputFormat;//D3DXF_FILEFORMAT_TEXT
//	char 							OutputSaveAs[DXCC_DEFAULT_STRING_LENGTH];
//
//	LPD3DXBUFFER					UserData;
//
//};



//Interchanger for Vertex Buffers and shader registers
//privides conversion fucntions to and from standard types.
//converts them into an array of up to 4 values
typedef struct EXPORTDEF DXCCFLOAT4
{
public:
	FLOAT		value[4];
	WORD		flags;
	BYTE		swizzle;
	BYTE		count;

	//encode value[4] from the function param
	DXCCFLOAT4* Encode( signed char* pValue,		 UINT nCount);	
	DXCCFLOAT4* Encode( unsigned char* pValue,		 UINT nCount);	
	DXCCFLOAT4* Encode( signed short* pValue,		 UINT nCount);	
	DXCCFLOAT4* Encode( unsigned short* pValue,		 UINT nCount);	
	DXCCFLOAT4* Encode( signed int* pValue,			 UINT nCount);		
	DXCCFLOAT4* Encode( unsigned int* pValue,		 UINT nCount);	
	DXCCFLOAT4* Encode( signed long* pValue,		 UINT nCount);	
	DXCCFLOAT4* Encode( unsigned long* pValue,		 UINT nCount);	
	DXCCFLOAT4* Encode( float* pValue,				 UINT nCount);			
	DXCCFLOAT4* Encode( double* pValue,				 UINT nCount);			
	DXCCFLOAT4* Encode( D3DXFLOAT16* pValue,		 UINT nCount);	
	DXCCFLOAT4* Encode( D3DCOLOR* pValue);						
	DXCCFLOAT4* Encode( D3DXCOLOR* pValue);					
	DXCCFLOAT4* Encode( D3DCOLORVALUE* pValue);				
	DXCCFLOAT4* Encode( D3DVECTOR* pValue);					
	DXCCFLOAT4* Encode( D3DXVECTOR2* pValue);					
	DXCCFLOAT4* Encode( D3DXVECTOR3* pValue);					
	DXCCFLOAT4* Encode( D3DXVECTOR4* pValue);					
	DXCCFLOAT4* Encode( D3DXPLANE* pValue);					
	DXCCFLOAT4* Encode( D3DXVECTOR2_16F* pValue);				
	DXCCFLOAT4* Encode( D3DXVECTOR3_16F* pValue);				
	DXCCFLOAT4* Encode( D3DXVECTOR4_16F* pValue);				

	//decode value[4] into the function param
	DXCCFLOAT4* Decode(  signed char* pValue,		 UINT nCount);	
	DXCCFLOAT4* Decode(  unsigned char* pValue,	 UINT nCount);	
	DXCCFLOAT4* Decode(  signed short* pValue,		 UINT nCount);	
	DXCCFLOAT4* Decode(  unsigned short* pValue,	 UINT nCount);	
	DXCCFLOAT4* Decode(  signed int* pValue,		 UINT nCount);		
	DXCCFLOAT4* Decode(  unsigned int* pValue,		 UINT nCount);	
	DXCCFLOAT4* Decode(  signed long* pValue,		 UINT nCount);	
	DXCCFLOAT4* Decode(  unsigned long* pValue,	 UINT nCount);	
	DXCCFLOAT4* Decode(  float* pValue,			 UINT nCount);			
	DXCCFLOAT4* Decode(  double* pValue,			 UINT nCount);			
	DXCCFLOAT4* Decode(  D3DXFLOAT16* pValue,		 UINT nCount);	
	DXCCFLOAT4* Decode(  D3DCOLOR* pValue);					
	DXCCFLOAT4* Decode(  D3DXCOLOR* _pValue);					
	DXCCFLOAT4* Decode(  D3DCOLORVALUE* pValue);				
	DXCCFLOAT4* Decode(  D3DVECTOR*_pValue);					
	DXCCFLOAT4* Decode(  D3DXVECTOR2* pValue);					
	DXCCFLOAT4* Decode(  D3DXVECTOR3* pValue);					
	DXCCFLOAT4* Decode(  D3DXVECTOR4* pValue);					
	DXCCFLOAT4* Decode(  D3DXPLANE* pValue);					
	DXCCFLOAT4* Decode(  D3DXVECTOR2_16F* pValue);				
	DXCCFLOAT4* Decode(  D3DXVECTOR3_16F* pValue);				
	DXCCFLOAT4* Decode(  D3DXVECTOR4_16F* pValue);

} *LPDXCCFLOAT4;

#undef INTERFACE


#define INTERFACE IDXCCPropertyBag
DECLARE_INTERFACE_(IDXCCPropertyBag, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_  REFIID riid,  void** ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

	STDMETHOD(LoadFromFile)(THIS_  LPCSTR szPath ) PURE;

	STDMETHOD(Save)(THIS) PURE;
    STDMETHOD(SaveAs)(THIS_  LPCSTR szPath ) PURE;

	STDMETHOD(ShowEditDialog)(THIS_ HWND hParent, BOOL bModal) PURE;

	STDMETHOD_(INT, GetVersion)(THIS) PURE;
	STDMETHOD(SetVersion)(THIS_ INT Version) PURE;
	STDMETHOD_(LPCSTR, GetType)(THIS) PURE;
	STDMETHOD(SetType)(THIS_ LPCSTR szType) PURE;

	STDMETHOD_(UINT, NumProperties)(THIS) PURE;
	STDMETHOD(SetPropertyCount)(THIS_ UINT count) PURE;
	STDMETHOD(RemoveProperty)(THIS_ UINT index) PURE;

	STDMETHOD(FindByName)(THIS_ LPCSTR szName, UINT* pOutIndex) PURE;

	STDMETHOD_(LPCSTR, GetName)(THIS_ UINT Index) PURE;
	STDMETHOD_(DXCC_PROPERTY_TYPE, GetType)(THIS_ UINT Index) PURE;
    STDMETHOD_(BOOL, GetValueAsBool)(THIS_ UINT Index) PURE;
	STDMETHOD_(INT, GetValueAsInt)(THIS_ UINT Index) PURE;
	STDMETHOD_(FLOAT, GetValueAsFloat)(THIS_ UINT Index) PURE;
	STDMETHOD_(LPCSTR, GetValueAsString)(THIS_ UINT Index) PURE;

	STDMETHOD(SetName)(THIS_ UINT Index, LPCSTR szName) PURE;
    STDMETHOD(SetValueAsBool)(THIS_ UINT Index, BOOL bVal) PURE;
	STDMETHOD(SetValueAsInt)(THIS_ UINT Index, INT iVal) PURE;
	STDMETHOD(SetValueAsFloat)(THIS_ UINT Index, FLOAT fVal ) PURE;
	STDMETHOD(SetValueAsString)(THIS_ UINT Index, LPCSTR szVal) PURE;


};


#undef INTERFACE



//THIS US A USER DEFINED CLASS USED AS A PARAMETER FOR DXCCEnumEffectParameters
#define INTERFACE IDXCCEnumEffectParameters
DECLARE_INTERFACE(IDXCCEnumEffectParameters)
{
	struct PathInfo
	{
		D3DXPARAMETER_DESC	Description;
		UINT				Length;
		const LPCSTR*		Names;//array of size Length
		const D3DXHANDLE*	Handles;//array of size Length

		LPCSTR MyName() { return (Length > 0)? Names[Length-1] : NULL;}
		LPCSTR ParentName() { return (Length > 1)? Names[Length-2] : NULL;}
		LPCSTR RootName() { return (Length > 0)? Names[0] : NULL;}
		D3DXHANDLE MyHandle() { return (Length > 0)? Handles[Length-1] : NULL;}
		D3DXHANDLE ParentHandle() { return (Length > 1)? Handles[Length-2] : NULL;}
		D3DXHANDLE RootHandle() { return (Length > 0)? Handles[0] : NULL;}

	};

	STDMETHOD(EnumParameter)(THIS_  LPD3DXEFFECT pEffect, PathInfo& parameter) PURE;
};
#undef INTERFACE

#define INTERFACE IDXCCEFFECTDIALOGCALLBACKS
//all functions return E_NOTIMPL for default processing
DECLARE_INTERFACE(IDXCCEffectDlgCallbacks)
{
	//called after standard show procedures are performed
	//manipulate the dialog here such as embedding.
	STDMETHOD(OnShow)(THIS_  
 		LPDIRECT3DDEVICE9 pDevice,
 		LPDXCCMANAGER pManager,
 		LPDXCCRESOURCE pResource) { return E_NOTIMPL; };

	//called immediately after the load button is pressed
	STDMETHOD(OnPreLoad)(THIS_  
	 	HWND hDialog, 
 		LPDIRECT3DDEVICE9 pDevice,
 		LPDXCCMANAGER pManager,
 		LPDXCCRESOURCE pResource,
 		LPCSTR szNewEffectFilename) { return E_NOTIMPL; };//return E_NOTIMPL for default processing

	//called as a replacement of the standard load...return E_NOTIMPL for default behavior
	//load the effect.
	//recycle the resource with the effect
	//add the appropriate resource path string to the resource
	//change the title of the hDialog
	STDMETHOD(OnLoad)(THIS_  
	 	HWND hDialog, 
 		LPDIRECT3DDEVICE9 pDevice,
 		LPDXCCMANAGER pManager,
 		LPDXCCRESOURCE pResource,
 		LPCSTR szNewEffectFilename) { return E_NOTIMPL; };//return E_NOTIMPL for default processing
	
	//called after load is performed
	STDMETHOD(OnPostLoad)(THIS_  
	 	HWND hDialog, 
 		LPDIRECT3DDEVICE9 pDevice,
 		LPDXCCMANAGER pManager,
 		LPDXCCRESOURCE pResource,//recycle me
 		LPCSTR szNewEffectFilename) { return E_NOTIMPL; };//return E_NOTIMPL for default processing

	//when the Parameter is selected
	STDMETHOD(OnParameterSelect)(THIS_  
	 	HWND hDialog, 
 		LPDIRECT3DDEVICE9 pDevice,
 		LPDXCCMANAGER pManager,
		LPDXCCRESOURCE pResource,
 		LPD3DXEFFECT pEffect,
 		D3DXHANDLE hNewParameter,
		HWND hParentWindow,
		HWND* hOutWindow) { return E_NOTIMPL; }; //return E_NOTIMPL for default processing

	//when the Parameter Data is applied
	STDMETHOD(OnParameterApply)(THIS_  
	 	HWND hDialog, 
 		LPDIRECT3DDEVICE9 pDevice,
 		LPDXCCMANAGER pManager,
 		LPDXCCRESOURCE pResource,
 		LPD3DXEFFECT pEffect,
 		D3DXHANDLE hParameter) { return E_NOTIMPL; }; //return E_NOTIMPL for default processing

	//called before standard close procedures are performed
	STDMETHOD(OnClose)(THIS_  
	 	HWND hDialog, 
 		LPDIRECT3DDEVICE9 pDevice,
 		LPDXCCMANAGER pManager,
 		LPDXCCRESOURCE pResource) { return E_NOTIMPL; };

};
#undef INTERFACE

#define INTERFACE IDXCCHIERARCHYCALLBACKS
DECLARE_INTERFACE(IDXCCHierarchyCallbacks)
{
	STDMETHOD(OnCreateD3DXFrame)(THIS_ LPDXCCFRAME pThis, LPD3DXFRAME pFrame) PURE;
	STDMETHOD(OnCreateD3DXMeshContainer)(THIS_ LPDXCCMESH pThis, LPD3DXMESHCONTAINER pMesh) PURE;
};
#undef INTERFACE

#define INTERFACE IDXCCHeap
DECLARE_INTERFACE_(IDXCCHeap, ID3DXBuffer)
{
	STDMETHOD(QueryInterface)(THIS_  REFIID riid,  void** ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

	//this is the primary memory
    STDMETHOD_(LPVOID, GetBufferPointer)(THIS) PURE;
    STDMETHOD_(DWORD, GetBufferSize)(THIS) PURE;

	STDMETHOD_(LPVOID,CreateMemory)(THIS_ DWORD SizeInBytes) PURE;
	STDMETHOD(DeleteMemory)(THIS_ void* Memory) PURE;
	STDMETHOD(SetPrimaryMemory)(THIS_ void* memory) PURE;

};
#undef INTERFACE


#define INTERFACE IDXCCResource
DECLARE_INTERFACE_(IDXCCResource, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_  REFIID riid,  void** ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

	//get the fast access handle...used with the manager to find this resource
	STDMETHOD_(DXCCHANDLE, GetHandle)(THIS) PURE;

	//get the object's creations IID/UUID
	STDMETHOD_(REFIID, GetIID)(THIS) PURE;

	//Get the object it is attached to
	//this operates like QueryInterface
	//but fails if object is not attached with AttachObject
	STDMETHOD(GetObject)(THIS_ 
		 LPUNKNOWN* ppvObj) PURE;	//output a pointer to the interface

	STDMETHOD(QueryObject)(THIS_ 
		 REFIID riid,			//what would you like me to query for?
		 void** ppvObj) PURE;	//output a pointer to the interface

	//reattach an object to the resource 
	//so that the interface can be retrieved from the resource
	STDMETHOD(AttachObject)(THIS_ 
		 LPUNKNOWN pValidation) PURE; //you must supply the interface ptr in order to re-attach the object otherwise i dont know if it is deleted

	//detach an interface from the resource 
	//a detatched resource can reach ref count of 0
	//but you can no longer GetObject until you re-attach with AttachObject
	//return TRUE if ref count is zero
	//return FALSE if detached or ref count has not reached zero
	STDMETHOD_(BOOL, DetachObject)(THIS) PURE; 

	//can I use GetObject to retrieve this object
	STDMETHOD_(BOOL, Attached)(THIS) PURE;

	STDMETHOD_(BOOL, Equals)(THIS_ 
		 LPUNKNOWN pValidation) PURE; //you must supply the interface ptr in order to re-attach the object otherwise i dont know if it is deleted

	//what is my name?
	STDMETHOD_(LPCSTR, GetName)(THIS) PURE;

	//what is my name?
	STDMETHOD(SetName)(THIS_ 
		 LPCSTR name) PURE;

	//what resource did i originate from?
	STDMETHOD_(LPCSTR, GetResourcePath)(THIS) PURE;

	//what resource did i originate from?
	STDMETHOD(SetResourcePath)(THIS_ 
		 LPCSTR name) PURE;

	//Get user data that is stored in this resource
	//use of any1 or all 3 parameters is valid
	STDMETHOD(GetUserData)(THIS_ 
		 LPD3DXBUFFER* ppBuffer,	//(optional)the buffer that contains the user data
		 void** ppData,			//(optional)the data pointer from inside the buffer(can be retrieved from ppBuffer)
		 DWORD* pSize) PURE;		//(optional)the size of the data(can be retrieved from ppBuffer)

	//specify user data
	STDMETHOD(SetUserData)(THIS_ 
		 LPD3DXBUFFER pBuffer) PURE;

	//create a user data without having to do a d3dxcreatebuffer
	STDMETHOD(CreateUserData)(THIS_ 
		 DWORD nSizeInBytes) PURE;
};
#undef INTERFACE

#define INTERFACE IDXCCManager
DECLARE_INTERFACE_(IDXCCManager, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_  REFIID riid,  void** ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;


	//case sensitive
	STDMETHOD(FindResourceByName)(THIS_ 
		 LPCSTR szName, 
		 DXCCHANDLE hAfter, 
		 LPDXCCRESOURCE* ppResource) PURE;

	//non case sensitive
	STDMETHOD(FindResourceByResourcePath)(THIS_ 
		 LPCSTR szResourcePath, 
		 DXCCHANDLE hAfter, //if 0 then it starts from the beginning otherwise if it's a valid handle it searches after
		 LPDXCCRESOURCE* ppResource) PURE;

	//find a specific interface or derivative interface though QueryForInterface variable
	//ex:good for searching for all texture object since they derive from basetexture
	STDMETHOD(FindResourceByIID)(THIS_ 
		 REFIID iid, 
		 DXCCHANDLE hAfter, //if 0 then it starts from the beginning otherwise if it's a valid handle it searches after
		 LPDXCCRESOURCE* ppResource) PURE;

	STDMETHOD(FindResourceByQuery)(THIS_ 
		 REFIID iid, 
		 DXCCHANDLE hAfter, //if 0 then it starts from the beginning otherwise if it's a valid handle it searches after
		 LPDXCCRESOURCE* ppResource) PURE;

	//should be the fastest find method
	STDMETHOD(FindResourceByHandle)(THIS_ 
		 DXCCHANDLE Handle,
		 LPDXCCRESOURCE* ppResource) PURE;

	//find an object within the space 
	STDMETHOD(FindResourceByPointer)(THIS_ 
		 LPUNKNOWN pFind, //required if finding by handle
		 DXCCHANDLE hAfter, //if 0 then it starts from the beginning otherwise if it's a valid handle it searches after
		 LPDXCCRESOURCE* ppResource) PURE;

	//create a resource
	STDMETHOD(CreateResource)(THIS_ 
		 LPUNKNOWN pUnknown, 
		 REFIID riid,
		 BOOL Attached,
		 LPDXCCRESOURCE* ppResource) PURE;

	//remove a resource from the manager. 
	//The resource may remain alive if it being referenced by something 
	//but it will not be retrievable though the mananger.
	STDMETHOD(RemoveResource)(THIS_ DXCCHANDLE handle) PURE;

	STDMETHOD(RecycleResource)(THIS_ 
		DXCCHANDLE hResource,
		LPUNKNOWN pUnknown, 
		REFIID riid,
		BOOL Attached,
		BOOL EraseMetadata) PURE;//name, path. userdata 

	//STDMETHOD_(void*, CreateMemory)(THIS_ DWORD SizeInBytes) PURE;
	//STDMETHOD_(DWORD, FindMemorySize)(THIS_ void* memory) PURE;
	//STDMETHOD_(UINT, AddRefMemory)(THIS_ void* memory) PURE;
	//STDMETHOD_(UINT, ReleaseMemory)(THIS_ void* memory) PURE;
	//translator is another set of memory, set of DWORDs, each indicating an offset of a pointer.
	//STDMETHOD_(SetMemoryTranslator)(THIS_ void* memory, void* translator) PURE;
	//STDMETHOD(ShadowMemory)(THIS_ DWORD SizeInBytes, void* memory) PURE;

};

//struct DXCCMessage
//{
//	DXCCHANDLE		Handle; //if NULL op on Manager
//	UINT			Message;//GetDecoder based on GUID of Handle and Message
//	LPD3DXBUFFER	Data;
//};
//
//struct	ModifyEffect
//{
//	DXCCHANDLE hMesh;
//	LPCSTR szSemantic;
//	LPCSTR szObject;
//	UINT ObjectIndex;
//	UINT nFloatCount;
//	FLOAT* pFloats;
//	LPCSTR szUnits;
//	LPCSTR szSpace;
//};

//#undef INTERFACE
//#define INTERFACE IDXCCSerializer
//DECLARE_INTERFACE(IDXCCSerializer)
//{
//	STDMETHOD_(REFIID,GetIID)(THIS) PURE;
//	STDMETHOD(Encode)(THIS_ HANDLE hFile) PURE;
//	STDMETHOD(Decode)(THIS) PURE;
//};
//
//

#undef INTERFACE
#define INTERFACE IDXCCDeclaration
//#define INTERFACE IDXCCDeclarationBuilder
//extends read only functionality of IDXCCDeclarationReadOnly to provide set/edit functionality.
//provides the ability to generate a declaration from scratch.
//DECLARE_INTERFACE_(IDXCCDeclarationBuilder, IDXCCDeclaration)
DECLARE_INTERFACE_(IDXCCDeclaration, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_  REFIID riid,  void** ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

	//STDMETHOD(Clone)(THIS_ D3DVERTEXELEMENT9* pDecl) PURE;
	STDMETHOD_(BOOL, Identical)(THIS_ 
		 CONST D3DVERTEXELEMENT9* pDecl) PURE;

	STDMETHOD_(UINT, NumElements)(THIS) PURE;

	//1to4 fields per element.
	STDMETHOD_(UINT, NumElementFields)(THIS_ 
		 UINT iElement) PURE;

	STDMETHOD_(CONST LPD3DVERTEXELEMENT9, GetElements)(THIS) PURE;

	STDMETHOD_(CONST LPD3DVERTEXELEMENT9, GetElement)(THIS_ 
		 UINT iElement) PURE;

	//a custom name can be attached per Element
	STDMETHOD_(LPCSTR, GetElementName)(THIS_ 
		 UINT iElement) PURE;

	//a custom name can be attached per Element
	STDMETHOD(FindElementByName)(THIS_ 
		 LPCSTR name, 
		 UINT* piElement, 
		 LPD3DVERTEXELEMENT9* ppElement) PURE;

	STDMETHOD(FindElementBySemantic)(THIS_ 
		 UINT _Usage, 
		 UINT _UsageIndex, 
		 UINT* piElement, 
		 LPD3DVERTEXELEMENT9* ppElement) PURE;

	STDMETHOD_(UINT, NumBlendWeightFields)(THIS) PURE;

	STDMETHOD_(UINT, NumBlendIndexFields)(THIS) PURE;

	STDMETHOD_(UINT, NumTexcoordFields)(THIS_ 
		 UINT UsageIndex) PURE;

	STDMETHOD(GetPositionElement)(THIS_ 
		 UINT* piElement, 
		 LPD3DVERTEXELEMENT9* ppElement) PURE;

	STDMETHOD(GetPointSizeElement)(THIS_ 
		 UINT* piElement,
		 LPD3DVERTEXELEMENT9* ppElement) PURE;

	STDMETHOD(GetNormalElement)(THIS_ 
		 UINT* piElement, 
		 LPD3DVERTEXELEMENT9* ppElement) PURE;

	STDMETHOD(GetTangentElement)(THIS_ 
		 UINT* piElement, 
		 LPD3DVERTEXELEMENT9* ppElement) PURE;

	STDMETHOD(GetBinormalElement)(THIS_ 
		 UINT* piElement, 
		 LPD3DVERTEXELEMENT9* ppElement) PURE;

	STDMETHOD(GetDiffuseElement)(THIS_ 
		 UINT* piElement, 
		 LPD3DVERTEXELEMENT9* ppElement) PURE;

	STDMETHOD(GetSpecularElement)(THIS_ 
		 UINT* piElement, 
		 LPD3DVERTEXELEMENT9* ppElement) PURE;

	STDMETHOD(GetTexcoordElement)(THIS_ 
		 BYTE UsageIndex, 
		 UINT* piElement, 
		 LPD3DVERTEXELEMENT9* ppElement) PURE;

	STDMETHOD(GetBlendWeightsElement)(THIS_ 
		 UINT* piElement, 
		 LPD3DVERTEXELEMENT9* ppElement) PURE;

	STDMETHOD(GetBlendIndicesElement)(THIS_ 
		 UINT* piElement, 
		 LPD3DVERTEXELEMENT9* ppElement) PURE;

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(SetDeclaration)(THIS_ 
		 DWORD FVF) PURE;

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(SetDeclaration)(THIS_ 
		 CONST D3DVERTEXELEMENT9* pDecl) PURE;

	//a custom name can be attached per Element
	STDMETHOD(SetElementName)(THIS_ 
		 UINT iElement, 
		 LPCSTR name) PURE;

	//a custom name can be attached per Element but this is the default name
	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(SetElementNameToDefault)(THIS_ 
		 UINT iElement) PURE;

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(RemoveElement)(THIS_ 
		 UINT Index) PURE;

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(InsertElement)(THIS_ 
		 UINT iBefore, //use NumElements() to hit the end
		 LPD3DVERTEXELEMENT9 pNew, 
		 LPCSTR _name) PURE;

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(InsertPositionElement)(THIS_ 
		 UINT iBefore) PURE;//use NumElements() to hit the end

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(InsertPointSizeElement)(THIS_ 
		 UINT iBefore) PURE;//use NumElements() to hit the end

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(InsertNormalElement)(THIS_ 
		 UINT iBefore) PURE;//use NumElements() to hit the end

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(InsertTangentElement)(THIS_ 
		 UINT iBefore) PURE;//use NumElements() to hit the end

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(InsertBinormalElement)(THIS_ 
		 UINT iBefore) PURE;//use NumElements() to hit the end

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(InsertDiffuseElement)(THIS_ 
		 UINT iBefore) PURE;//use NumElements() to hit the end

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(InsertSpecularElement)(THIS_ 
		 UINT iBefore) PURE;//use NumElements() to hit the end

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(InsertTexcoordElement)(THIS_ 
		 UINT iBefore, //use NumElements() to hit the end
		 UINT iTexIndex, 
		 UINT cCoordCount, 
		 LPCSTR name) PURE; //texcoords should be named for linkage reasons

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(InsertBlendWeightsElement)(THIS_ 
		 UINT iBefore) PURE;//use NumElements() to hit the end

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(InsertBlendIndicesElement)(THIS_ 
		 UINT iBefore) PURE;//use NumElements() to hit the end
};
#undef INTERFACE

#define INTERFACE IDXCCVertexBundler
//simple encapsilation of vertex streams
//this is an interface to simply read/write all of the vertex data associated with a single declaration.
//the class holds all of the stream information and vertex buffers.
//but no functionality to set the streams....see derived classes
//you can read the declaration but not change it.
//then you can edit the data by element for each vertex of the appropriate vertex buffer.
DECLARE_INTERFACE_(IDXCCVertexBundler, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_  REFIID riid,  void** ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(Recycle)(THIS_ 
 		UINT NumVertices,
 		LPD3DVERTEXELEMENT9 pDecl) PURE;

	STDMETHOD(GetVertexBuffer)(THIS_ 
		 UINT iStream, 
		 LPDIRECT3DVERTEXBUFFER9* ppVB) PURE;

	STDMETHOD(GetDeclaration)(THIS_ 
		 LPDXCCDECLARATION* ppDeclRead) PURE;
	
	STDMETHOD_(DWORD, NumVertices)(THIS) PURE;

	//Get data based on arbitrary vertex element
	STDMETHOD(GetData)(THIS_ 
		 UINT iVertex, 
		 UINT iElem, 
		 DXCCFLOAT4* pVED) PURE;

	//Get data based on standard vertex element	
	STDMETHOD(GetPosition)(THIS_ 
		 UINT iVertex, 
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(GetPointSize)(THIS_ 
		 UINT iVertex, 
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(GetNormal)(THIS_ 
		 UINT iVertex, 
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(GetTangent)(THIS_ 
		 UINT iVertex, 
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(GetBinormal)(THIS_ 
		 UINT iVertex, 
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(GetDiffuse)(THIS_ 
		 UINT iVertex, 
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(GetSpecular)(THIS_ 
		 UINT iVertex, 
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(GetTexcoord)(THIS_ 
		 UINT iVertex, 
		 UINT iTexcoord,  
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(GetBlendWeights)(THIS_ 
		 UINT iVertex, 
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(GetBlendIndices)(THIS_ 
		 UINT iVertex, 
		 DXCCFLOAT4* pVED) PURE;

	//Set data based on arbitrary vertex element
	STDMETHOD(SetData)(THIS_ 
		 UINT iVertex, 
		 UINT iElem, 
		 DXCCFLOAT4* pVED) PURE;

	//Set data based on standard vertex element	
	STDMETHOD(SetPosition)(THIS_ 
		 UINT iVertex,  
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(SetPointSize)(THIS_ 
		 UINT iVertex,  
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(SetNormal)(THIS_ 
		 UINT iVertex,  
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(SetTangent)(THIS_ 
		 UINT iVertex,  
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(SetBinormal)(THIS_ 
		 UINT iVertex,  
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(SetDiffuse)(THIS_ 
		 UINT iVertex,  
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(SetSpecular)(THIS_ 
		 UINT iVertex,  
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(SetTexcoord)(THIS_ 
		 UINT iVertex,  
		 UINT iTexcoord,   
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(SetBlendWeights)(THIS_ 
		 UINT iVertex,  
		 DXCCFLOAT4* pVED) PURE;

	STDMETHOD(SetBlendIndices)(THIS_ 
		 UINT iVertex,  
		 DXCCFLOAT4* pVED) PURE;

	//Lock down the vertex buffer.  set up one of 16 vertex streams with full vertex stream data
	STDMETHOD(Lock)(THIS_  
		 UINT iStream) PURE;

	STDMETHOD(Unlock)(THIS_  
		 UINT iStream) PURE;

	STDMETHOD(LockAll)(THIS) PURE;

	STDMETHOD(UnlockAll)(THIS) PURE;

	STDMETHOD(SetStream)(THIS_  
		 UINT iStream,  //what stream index do you want? ...see vertex declaration elements
		 UINT OffsetInBytes,  //how far from the beginning of the vertex buffer does the data start
		 UINT Stride,  //how far between vertices
		 UINT iFrequencyDivider,	//instancing	(default is 1)
		 CONST LPDIRECT3DVERTEXBUFFER9 pVB) PURE;
};
#undef INTERFACE

//#define INTERFACE IDXCCMaterial
////provides access to all material data from IDXCCMesh
//DECLARE_INTERFACE_(IDXCCMaterial, IUnknown)
//{
//	STDMETHOD(QueryInterface)(THIS_  REFIID riid,  void** ppvObj) PURE;
//	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
//	STDMETHOD_(ULONG,Release)(THIS) PURE;
//
//	/* BASE MATERIAL */
//	//this is the default material...ie no effect based material colors
//	STDMETHOD_(D3DMATERIAL9*, GetD3DMaterial)(THIS) PURE;
//
//	/* DEFAULT TEXTURE */
//	//this is the default diffuse texture...providing backward compatability with old xfile loaders
//	STDMETHOD(GetD3DXMaterialTexture)(THIS_ 
//		 LPDIRECT3DBASETEXTURE9* ppTexture) PURE; 
//
//	STDMETHOD(SetD3DXMaterialTexture)(THIS_  
//		 LPDIRECT3DBASETEXTURE9 pTexture) PURE;
//	
//	/* EFFECT*/
//	//Storage of a material's shaders.  this is here primarily to track linkage information and param default values
//	STDMETHOD(SetEffect)(THIS_  
//		 LPD3DXEFFECT pEffect) PURE;
//
//	STDMETHOD(GetEffect)(THIS_  
//		 LPD3DXEFFECT* ppEffect) PURE;
//};

#undef INTERFACE
#define INTERFACE IDXCCMesh
//This is a derivative of d3dx mesh container.  
//it provides simple access to all vertex, face, material, bone/skin information
//which makes up the concept of a renderable mesh
//It is a hierarchy component which lives under IDXCCFrame
DECLARE_INTERFACE_(IDXCCMesh, IUnknown)
{
    /*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_  REFIID riid,  void** ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

	//E_ACCESSDENIED may be returned when the structure is embedded
	STDMETHOD(Recycle)(THIS_ 
 		DWORD NumFaces,
 		DWORD NumVertices,
 		LPDIRECT3DDEVICE9 pDevice,
 		LPD3DVERTEXELEMENT9 pDeclaration) PURE;

	STDMETHOD_(DWORD, NumVertices)(THIS) PURE;
	STDMETHOD_(DWORD, NumFaces)(THIS) PURE;

	//access to declaration and vertex buffers
	STDMETHOD(GetVertexBundler)(THIS_ 
		 LPDXCCVERTEXBUNDLER* ppVBundle) PURE;

	//locks and unlocks index buffer (everything else is nonvideo) (attribute and adjacency)
    STDMETHOD(LockFaces)(THIS) PURE; 

	//locks and unlocks index buffer (everything else is nonvideo) (attribute and adjacency)
    STDMETHOD(UnlockFaces)(THIS) PURE;

	//must  call LockFaces to use 
	//must  call UnlockFaces to submit geometry to vid card
	//associate a triangle's local vertex to a vertex  GetVertexBundler
	STDMETHOD(SetFaceVertex)( 
		 UINT iFace, //0 to NumFaces()-1
		 UINT iLocalVertex, //0 to 2
		 DWORD iVertex) PURE; //0 to NumVertices()-1

	//must  call LockFaces to use 
	STDMETHOD(GetFaceVertex)(UINT 
		 iFace, //0 to NumFaces()-1
		 UINT iLocalVertex,//0 to 2
		 DWORD* piVertex) PURE;

	///////////////////////////////////////
	//ADJACENCY
	///////////////////////////////////////

	//adjacency information is for topology based decisions and optimizations
	STDMETHOD(SetFaceAdjacency)(
		 UINT iFace, //0 to NumFaces()-1
		 UINT iLocalAdjacency, //0 to 2
		 DWORD iNewFace) PURE; //0 to NumFaces()-1

	STDMETHOD(GetFaceAdjacency)(
		 UINT iFace, //0 to NumFaces()-1
		 UINT iLocalAdjacency, //0 to 2
		 DWORD* piNewFace) PURE;

	//if you do not wish to provide topology information then it may be generated 
    STDMETHOD(GenerateAdjacency)(THIS_ 
		 FLOAT Epsilon) PURE; //where epsilon is the max distance between vertices for them to be considered the same point

	//users may choose to use pointreps at times for convenience. these functions provide conversion.
	STDMETHOD(ConvertPointRepsToAdjacency)(THIS_ 
		 CONST DWORD* pPRep) PURE;//take pointreps to fix internal adjacency

	STDMETHOD(ConvertAdjacencyToPointReps)(THIS_ 
		 LPD3DXBUFFER* ppPRep) PURE;//get pointreps instead of adjacency


	///////////////////////////////////////
	//ATTRIBUTES
	///////////////////////////////////////

	//set an ID per face that can be associated with materials though Get/SetAttributedMaterial
	STDMETHOD(SetFaceAttribute)(
		 UINT iFace, //0 to NumFaces()-1
		 DWORD Attribute) PURE; //any value you would like to group your polygons by

	STDMETHOD(GetFaceAttribute)(
		 UINT iFace, //0 to NumFaces()-1
		 DWORD* pAttribute) PURE;

	//get a list of all DWORD attributes that have had materials assigned to them
	STDMETHOD(GetAttributedMaterialList)(THIS_ 
		 LPD3DXBUFFER* ppbAttribute, //a buffer full of DWORDs.  one for each material assignment
		 DWORD* pNumAttributedMaterials) PURE;

	//assign a material to a DWORD attributes
	STDMETHOD(SetAttributedMaterial)(THIS_ 
		 DWORD dwAttribute, 
		 LPD3DXEFFECT pMaterial) PURE;
		 //LPDXCCMATERIAL pMaterial) PURE;

	//retrieve a material assigned to a DWORD attributes
	STDMETHOD(GetAttributedMaterial)(THIS_ 
		 DWORD dwAttribute, 
		 LPD3DXEFFECT* ppMaterial) PURE;
		 //LPDXCCMATERIAL* ppMaterial) PURE;

	STDMETHOD(RemoveAttributedMaterial)(THIS_ 
		DWORD dwAttribute) PURE;

	STDMETHOD(RemoveAllAttributedMaterials)(THIS) PURE;

	STDMETHOD(GetAttributeTable)(THIS_ 
		 LPD3DXBUFFER* ppAttribTable,  //d3dxattribute structures
		 DWORD* pAttribTableSize) PURE;

	///////////////////////////////////////
	//SKIN
	///////////////////////////////////////

	//Creates Bones - Frame bindings, OffsetMatrices, and Influences
	//Creates BindPose Stream and modifies existing VertexBundler
	//multiple calls to this function will result in loss of previous skin data
	//NumBones == 0 is same as calling RemoveSkin
	STDMETHOD(CreateSkin)(THIS_ 
		 DWORD NumBones) PURE; 
	//Removes Bones
	//Remove BindPose Stream
	STDMETHOD(RemoveSkin)(THIS) PURE; 
	
	STDMETHOD(SetSkinParent)(THIS_ 
		 LPDXCCFRAME pParent) PURE;

	STDMETHOD(GetSkinParent)(THIS_
		 LPDXCCFRAME* ppParent) PURE;

	STDMETHOD(DoSkinning)(THIS) PURE; 

	STDMETHOD(UndoSkinning)(THIS) PURE; 

	STDMETHOD_(DWORD, NumBones)(THIS) PURE;		

	STDMETHOD(SetBoneFrame)(THIS_ 
		 DWORD Bone, //0 to NumBones()-1
		 LPDXCCFRAME pBoneFrame, //the frame of the bone you are associating
		 BOOL CalcBoneOffset) PURE;

	STDMETHOD(GetBoneFrame)(THIS_ 
		 DWORD Bone, //0 to NumBones()-1
		 LPDXCCFRAME* ppBoneFrame) PURE;	

	//matrix that brings a vertex from the mesh's space to the bone's space
	STDMETHOD(SetBoneOffsetMatrix)(THIS_ 
		 DWORD Bone, //0 to NumBones()-1
		 LPD3DXMATRIX pBone) PURE;	

	//matrix that brings a vertex from the mesh's space to the bone's space
	STDMETHOD(GetBoneOffsetMatrix)(THIS_ 
		 DWORD Bone, //0 to NumBones()-1
		 LPD3DXMATRIX* ppBone) PURE;	

	STDMETHOD(SetBoneInfluence)(THIS_ 
		 DWORD bone, //0 to NumBones()-1
		 UINT iVertex, //0 to NumVertices()-1
		 FLOAT weight) PURE;	//% of total that that bone effects the vertex

	STDMETHOD(GetBoneInfluence)(THIS_ 
		 DWORD bone, //0 to NumBones()-1
		 UINT iVertex, //0 to NumVertices()-1
		 FLOAT* pWeight) PURE;
 


/*	
	STDMETHOD(SkinCaptureMesh)(THIS_ 
 		LPD3DVERTEXELEMENT9 pDeclaration,//the decl elements that you would like to match by usage & usageIndex
		DWORD NumBones) PURE;

	//restore Mesh (skin elements) to captured state
	STDMETHOD(SkinRestoreMesh)(THIS) PURE;
	//deform's Mesh (skin elements) to based on bones
	STDMETHOD(SkinDeformMesh)(THIS) PURE;


	//Edits to decompiled data are fast
	STDMETHOD(SkinLock)(THIS) PURE;
	SkinSetBoneInfluences(DWORD* verts, FLOAT* weights);
	SkinSetBoneInfluences(FLOAT* weights);
	SkinSetVertexInfluences(DWORD* verts, FLOAT* weights);
	SkinSetVertexInfluences(FLOAT* weights);
	STDMETHOD(SkinUnlock)(THIS) PURE;


	//this sets up the skinning allocation.  all skinning data is lost on set calls
	STDMETHOD(SkinCreateBones)(THIS_ 
		 DWORD NumBones) PURE; 

	STDMETHOD_(DWORD, SkinNumBones)(THIS) PURE;		

	STDMETHOD(SkinSetBoneFrame)(THIS_ 
		 DWORD Bone, //0 to NumBones()-1
		 LPDXCCFRAME pBoneFrame, //the frame of the bone you are associating
		 LPDXCCFRAME pSkinFrame) PURE; // the frame this mesh providing world space relativity

	STDMETHOD(SkinGetBoneFrame)(THIS_ 
		 DWORD Bone, //0 to NumBones()-1
		 LPDXCCFRAME* ppBoneFrame) PURE;	

	//matrix that brings a vertex from the mesh's space to the bone's space
	STDMETHOD(SkinSetBoneOffsetMatrix)(THIS_ 
		 DWORD Bone, //0 to NumBones()-1
		 LPD3DXMATRIX pBone) PURE;	

	//matrix that brings a vertex from the mesh's space to the bone's space
	STDMETHOD(SkinGetBoneOffsetMatrix)(THIS_ 
		 DWORD Bone, //0 to NumBones()-1
		 LPD3DXMATRIX* ppBone) PURE;	

	STDMETHOD(SkinSetBoneInfluence)(THIS_ 
		 DWORD bone, //0 to NumBones()-1
		 UINT iVertex, //0 to NumVertices()-1
		 FLOAT weight) PURE;	//% of total that that bone effects the vertex

	STDMETHOD(SkinGetBoneInfluence)(THIS_ 
		 DWORD bone, //0 to NumBones()-1
		 UINT iVertex, //0 to NumVertices()-1
		 FLOAT* pWeight) PURE;
 
*/
	//inplace only
    STDMETHOD(Optimize)(THIS_ 
		 D3DXCLEANTYPE CleanType, 
		 DWORD OptimizeFlags, //use one or more D3DXMESHOPT flags
		 LPD3DXBUFFER* ppFaceRemap, 
		 LPD3DXBUFFER *ppVertexRemap) PURE;  

	STDMETHOD(DrawSubset)(THIS_ 
		 DWORD AttribId) PURE;

    //get the index buffer for private rendering
	STDMETHOD(GetIndexBuffer)(THIS_ 
		 LPDIRECT3DINDEXBUFFER9* ppIB) PURE;

	STDMETHOD(GetAdjacencyBuffer)(THIS_ 
		 LPD3DXBUFFER* ppBuf) PURE;


	//pManager is to fetch Name and ResourcePath
	//flags are DXCCTOD3DX_FLAGS && DXCCTOD3DXMESH_FLAGS
	STDMETHOD(CreateD3DXMeshContainer)(THIS_ LPDXCCMANAGER pManager, LPDXCCSAVESCENEARGS pSaveOptions, DWORD ExtensionSize, LPDXCCHIERARCHYCALLBACKS MeshCallback, LPDXCCHEAP pHeap) PURE;

	//COMING SOON//STDMETHOD(GetIndexedBlendedSkinningInfo)(THIS_ LPD3DXBUFFER* ppBoneCombinationTable, DWORD* pNumBoneCombinations, DWORD* pMaxVertexInfl) PURE;
    //COMING SOON//STDMETHOD(ConvertToIndexedBlendedSkinning)(THIS_ DWORD paletteSize, DWORD* pFaceRemap, LPD3DXBUFFER *ppVertexRemap) PURE;
    //COMING SOON//STDMETHOD(Clone)(THIS_ CONST D3DVERTEXELEMENT9 *pDeclaration, LPDXCCMESH* ppCloneMesh) PURE; //COMING SOON
    //COMING SOON//STDMETHOD(Skin)(THIS_ LPDXCCVERTEXBUNDLER pVerticesDst) PURE;
    //COMING SOON//STDMETHOD(Unskin)(THIS_ LPDXCCVERTEXBUNDLER pVerticesDst) PURE;
};


#undef INTERFACE
#define INTERFACE IDXCCFrame
DECLARE_INTERFACE_(IDXCCFrame, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_  REFIID riid,  void** ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

	//am I the root of a tree? 
	STDMETHOD_(BOOL, Root)(THIS) PURE;

	//[IMPLIMENTATION INFO]this should cascade and call RemoveChild on old parent and AddChild on new parent if child is not already retrievable
	//[IMPLIMENTATION INFO]this should NOT perform AddRef/Release on pParent to avoid circular referencing
	STDMETHOD(SetParent)(THIS_ 
		 LPDXCCFRAME pParent, 
		 bool DelayRecalcs) PURE;

	STDMETHOD(GetParent)(THIS_
		 LPDXCCFRAME* ppParent) PURE;

	STDMETHOD_(UINT, NumChildren)(THIS) PURE;

	//[IMPLIMENTATION INFO]this should cascade and call SetParent(this) on child if parent is not set properly
	STDMETHOD(AddChild)(THIS_ 
		 LPDXCCFRAME pChild, 
		 bool DelayRecalcs) PURE;

	//[IMPLIMENTATION INFO]this should cascade and call SetParent(NULL) on child if parent is not set properly
	STDMETHOD(RemoveChild)(THIS_ 
		 UINT index, 
		 bool DelayRecalcs) PURE;

	STDMETHOD(GetChild)(THIS_ 
		 UINT index, 
		 LPDXCCFRAME* ppChild) PURE;

	//find an object within the space 
	STDMETHOD(FindChildByHandle)(THIS_ 
		 LPDXCCMANAGER pManager, //required if finding by handle
		 DXCCHANDLE Handle,
		 UINT *pIndex,
		 LPDXCCFRAME* ppChild) PURE;

	//find an object within the space 
	STDMETHOD(FindChildByPointer)(THIS_ 
		 LPDXCCFRAME pFind, //required if finding by handle
		 UINT *pIndex) PURE;

	//objects within this space
	STDMETHOD_(UINT, NumMembers)(THIS) PURE;

	//add an object to this space
	STDMETHOD(AddMember)(THIS_ 
		 LPUNKNOWN pMember) PURE;

	//remove an object from this space
	STDMETHOD(RemoveMember)(THIS_ 
		 UINT index) PURE;

	//get an object within the space by index
	STDMETHOD(GetMember)(THIS_ 
		 UINT index,  
		 LPUNKNOWN* ppMember) PURE;

	//get an object within the space by index
	STDMETHOD(QueryMember)(THIS_ 
		 UINT index, 
		 REFIID riid, 
		 void** ppMember) PURE;

	//find an object within the space 
	STDMETHOD(FindMemberByHandle)(THIS_ 
		 LPDXCCMANAGER pManager, //required if finding by handle
		 DXCCHANDLE Handle,
		 UINT *pIndex,
		 LPUNKNOWN* ppMember) PURE;

	//find an object within the space 
	STDMETHOD(FindMemberByPointer)(THIS_ 
		 LPUNKNOWN pUnknown, //required if finding by handle
		 UINT *pIndex) PURE;

	//find an object within the space 
	STDMETHOD(FindMemberByQuery)(THIS_ 
		 REFIID riid, //required if finding by handle
		 UINT *pIndex,
		 void** ppMember) PURE;

	//get/set matrix in local space ...ie relative to parent
	STDMETHOD_(CONST LPD3DXMATRIX, GetLocalMatrix)(THIS) PURE;	

	STDMETHOD(SetLocalMatrix)(THIS_ 
		 CONST LPD3DXMATRIX pMatrix, bool DelayRecalcs) PURE;//this affects all m_Children
	STDMETHOD(RecalcLocalsFromWorlds)(THIS) PURE;


	//get/set matrix in world space
	STDMETHOD_(CONST LPD3DXMATRIX, GetWorldMatrix)(THIS) PURE;

	STDMETHOD(SetWorldMatrix)(THIS_ 
		 CONST LPD3DXMATRIX pMatrix, bool DelayRecalcs) PURE;//this affects all m_Children
	STDMETHOD(RecalcWorldsFromLocals)(THIS) PURE;


	//STDMETHOD(SetLocalAnimation)(THIS_ 
	//	 LPDXCCANIMATIONSTREAM pAnim) PURE;

	STDMETHOD(GetLocalAnimation)(THIS_ 
		 LPDXCCANIMATIONSTREAM* ppAnim) PURE;

	//pManager is to fetch Name and ResourcePath
	//flags are DXCCTOD3DX_FLAGS
	STDMETHOD(CreateD3DXFrame)(THIS_ LPDXCCMANAGER pManager, LPDXCCSAVESCENEARGS pSaveOptions, DWORD ExtensionSize, LPDXCCHIERARCHYCALLBACKS MeshCallback, LPDXCCHEAP pHeap) PURE;

	//flags are DXCCTOD3DX_FLAGS
	STDMETHOD(CreateD3DXHierarchy)(THIS_
		LPDXCCMANAGER pManager,
		LPDXCCSAVESCENEARGS pSaveOptions, 
		DWORD FrameExtensionSize, 
		DWORD MeshExtensionSize, 
		LPDXCCHIERARCHYCALLBACKS pCallbacks,
		LPDXCCHEAP pHeap) PURE;


};

#undef INTERFACE
#define INTERFACE IDXCCAnimationStream
DECLARE_INTERFACE_(IDXCCAnimationStream, IUnknown)
{
    // ID3DXAnimationSet
    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // Period
    STDMETHOD_(DOUBLE, GetPeriod)(THIS) PURE;
    STDMETHOD_(DOUBLE, GetPeriodicPosition)(THIS_ DOUBLE Position) PURE;    // Maps position into animation period

     // SRT
    STDMETHOD(GetTransform)(THIS_ 
        DOUBLE PeriodicPosition,            // Position mapped to period (use GetPeriodicPosition)
        D3DXVECTOR3 *pScale,                // Returns the scale
        D3DXQUATERNION *pRotation,          // Returns the rotation as a quaternion
        D3DXVECTOR3 *pTranslation) PURE;    // Returns the translation

     // SRT
    STDMETHOD(GetMatrix)(THIS_ 
        DOUBLE PeriodicPosition,            // Position mapped to period (use GetPeriodicPosition)
        D3DXMATRIX *pMatrix) PURE;			// Returns the matrix

    // Callbacks
    //STDMETHOD(GetCallback)(THIS_ 
    //    DOUBLE Position,                    // Position from which to find callbacks
    //    DWORD Flags,                        // Callback search flags
    //    DOUBLE *pCallbackPosition,          // Returns the position of the callback
    //    LPVOID *ppCallbackData) PURE;       // Returns the callback data pointer

    // Playback
    STDMETHOD_(D3DXPLAYBACK_TYPE, GetPlaybackType)(THIS) PURE;
    STDMETHOD_(DOUBLE, GetSourceTicksPerSecond)(THIS) PURE;

    // Scale keys
    STDMETHOD_(UINT, NumScaleKeys)(THIS) PURE;
	STDMETHOD(GetScaleKey)(THIS_ UINT Key, LPD3DXKEY_VECTOR3 pScaleKey) PURE;
	STDMETHOD(SetScaleKey)(THIS_ UINT Key, LPD3DXKEY_VECTOR3 pScaleKey) PURE;
	STDMETHOD(FindScaleKey)(THIS_ FLOAT fKeyTime, UINT* pKey, LPD3DXKEY_VECTOR3 pValue) PURE;

    // Rotation keys
    STDMETHOD_(UINT, NumRotationKeys)(THIS) PURE;
	STDMETHOD(GetRotationKey)(THIS_ UINT Key, LPD3DXKEY_QUATERNION pRotationKey) PURE;
	STDMETHOD(SetRotationKey)(THIS_ UINT Key, LPD3DXKEY_QUATERNION pRotationKey) PURE;
	STDMETHOD(FindRotationKey)(THIS_ FLOAT fKeyTime, UINT* pKey, LPD3DXKEY_QUATERNION pValue) PURE;

    // Translation keys
    STDMETHOD_(UINT, NumTranslationKeys)(THIS) PURE;
	STDMETHOD(GetTranslationKey)(THIS_ UINT Key, LPD3DXKEY_VECTOR3 pTranslationKey) PURE;
	STDMETHOD(SetTranslationKey)(THIS_ UINT Key, LPD3DXKEY_VECTOR3 pTranslationKey) PURE;
	STDMETHOD(FindTranslationKey)(THIS_ FLOAT fKeyTime, UINT* pKey, LPD3DXKEY_VECTOR3 pValue) PURE;


    STDMETHOD_(UINT, NumTransformKeys)(THIS) PURE;
	STDMETHOD(GetTransformKey)(THIS_ UINT Key, LPDXCCKEY_TRANSFORM pTransformKey) PURE;
	STDMETHOD(SetTransformKey)(THIS_ UINT Key, LPDXCCKEY_TRANSFORM pTransformKey) PURE;
	STDMETHOD(FindTransformKey)(THIS_ FLOAT fKeyTime, UINT* pKey, LPDXCCKEY_TRANSFORM pValue) PURE;

	STDMETHOD(GetTransformKeyAsMatrix)(THIS_ UINT Key, LPDXCCKEY_MATRIX pMatrixKey) PURE;
	STDMETHOD(SetTransformKeyAsMatrix)(THIS_ UINT Key, LPDXCCKEY_MATRIX pMatrixKey) PURE;
	STDMETHOD(FindTransformKeyAsMatrix)(THIS_ FLOAT fKeyTime, UINT* pKey, LPDXCCKEY_MATRIX pValue) PURE;


    // Callback keys
    //STDMETHOD_(UINT, NumCallbackKeys)(THIS) PURE;
	//STDMETHOD(GetCallbackKey)(THIS_ UINT Key, LPD3DXKEY_CALLBACK pCallbackKey) PURE;
	//STDMETHOD(SetCallbackKey)(THIS_ UINT Key, LPD3DXKEY_CALLBACK pCallbackKey) PURE;

	// Key removal methods. These are slow, and should not be used once the animation starts playing
	STDMETHOD(RemoveScaleKey)(THIS_ UINT Key) PURE;
	STDMETHOD(RemoveRotationKey)(THIS_ UINT Key) PURE;
	STDMETHOD(RemoveTranslationKey)(THIS_ UINT Key) PURE;
	STDMETHOD(RemoveTransformKey)(THIS_ UINT Key) PURE;

	// Key removal methods. These are slow, and should not be used once the animation starts playing
	STDMETHOD(InsertScaleKey)(THIS_ UINT iBeforeKey, LPD3DXKEY_VECTOR3 pScaleKey) PURE;
	STDMETHOD(InsertRotationKey)(THIS_ UINT iBeforeKey, LPD3DXKEY_QUATERNION pRotationKey) PURE;
	STDMETHOD(InsertTranslationKey)(THIS_ UINT iBeforeKey, LPD3DXKEY_VECTOR3 pTranslationKey) PURE;
	STDMETHOD(InsertTransformKey)(THIS_ UINT iBeforeKey, LPDXCCKEY_TRANSFORM pTransformKey) PURE;
	STDMETHOD(InsertTransformKeyAsMatrix)(THIS_ UINT iBeforeKey, LPDXCCKEY_MATRIX pMatrixKey) PURE;

    // One-time animaton SRT keyframe registration
    STDMETHOD(CreateSRTKeys)(THIS_ 
		DOUBLE TicksPerSecond,
		D3DXPLAYBACK_TYPE Playback,
        UINT NumScaleKeys,                              // Number of scale keys
        UINT NumRotationKeys,                           // Number of rotation keys
        UINT NumTranslationKeys) PURE;	// Returns the animation index 

   // One-time animaton Matrix keyframe registration
    STDMETHOD(CreateTransformKeys)(THIS_ 
		DOUBLE TicksPerSecond,
		D3DXPLAYBACK_TYPE Playback,
        UINT NumTransforms) PURE;	// Returns the animation index 

    STDMETHOD(DestroyKeys)(THIS) PURE;

    //STDMETHOD(SetName)(THIS_ char* name) PURE;

    STDMETHOD(GetD3DXAnimationSet)(THIS_ LPDXCCMANAGER pManager, LPD3DXKEYFRAMEDANIMATIONSET* ppSet) PURE;


};


#undef INTERFACE
//#define INTERFACE IDXCCAnimationSet
//DECLARE_INTERFACE_(IDXCCAnimationSet, ID3DXKeyframedAnimationSet)
//{
//	STDMETHOD(QueryInterface)(THIS_  REFIID riid,  void** ppvObj) PURE;
//	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
//	STDMETHOD_(ULONG,Release)(THIS) PURE;
//
//	//get the stream sizes
//	STDMETHOD_(UINT, NumMatrices)(THIS) PURE;	
//
//	//access a stream element by key index.  the key may be a tick or an actual keyframe
//	//the find functions can instead find a closest time
//	STDMETHOD(GetMatrix)(THIS_ UINT iKey, LPDXCCKEY_MATRIX* ppValue) PURE;	
//
//	//find  a stream the key/tick index and ptr to keydata closest matching the desired time
//	STDMETHOD(FindMatrix)(THIS_ FLOAT fKeyTime, UINT* piKey, LPDXCCKEY_MATRIX* ppValue) PURE;	
//}



#undef INTERFACE
#define INTERFACE IDXCCFrameIterator

DECLARE_INTERFACE_(IDXCCFrameIterator, IUnknown)
{
	STDMETHOD(QueryInterface)(THIS_  REFIID riid,  void** ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

	STDMETHOD_(BOOL, Done)(THIS) PURE;
	STDMETHOD(Get)(THIS_ LPDXCCFRAME* ppFrame) PURE;
	STDMETHOD_(BOOL, Next)(THIS) PURE;

	STDMETHOD(Reset)(THIS_ LPDXCCFRAME pNewRoot, DXCCIteratorType NewType) PURE;
	STDMETHOD_(DXCCIteratorType, GetType)(THIS) PURE;
	STDMETHOD(GetRoot)(THIS_ LPDXCCFRAME* ppFrame) PURE;
	STDMETHOD_(UINT, GetDepth)(THIS) PURE;


};

//#undef INTERFACE
//#define INTERFACE IDXCCMeshIterator
//
//DECLARE_INTERFACE_(IDXCCMeshIterator, IUnknown)
//{
//	STDMETHOD(QueryInterface)(THIS_  REFIID riid,  void** ppvObj) PURE;
//	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
//	STDMETHOD_(ULONG,Release)(THIS) PURE;
//
//	STDMETHOD_(BOOL, Done)(THIS) PURE;
//	STDMETHOD(Get)(THIS_ LPDXCCMESH* ppMesh) PURE;
//	STDMETHOD_(BOOL, Next)(THIS) PURE;
//	STDMETHOD(Reset)(THIS_ LPDXCCFRAME pNewRoot) PURE;
//
//};

/*
#undef INTERFACE
#define INTERFACE IDXCCSaveUserData

DECLARE_INTERFACE(IDXCCSaveUserData)
{
    STDMETHOD(AddFrameChildData)(CONST LPDXCCFRAME pFrame, 
                            LPD3DXFILESAVEOBJECT pXofSave, 
                            LPD3DXFILESAVEDATA pXofFrameData) PURE;
                            
    STDMETHOD(AddFrameAnimationChildData)(CONST LPDXCCANIMATIONSTREAM *pAnimation, 
                            LPD3DXFILESAVEOBJECT pXofSave, 
                            LPD3DXFILESAVEDATA pXofMeshData) PURE;

    STDMETHOD(AddMeshChildData)(CONST LPDXCCMESH pMesh, 
                            LPD3DXFILESAVEOBJECT pXofSave, 
                            LPD3DXFILESAVEDATA pXofMeshData) PURE;
 
    //STDMETHOD(AddMeshMaterialChildData)(CONST LPDXCCMATERIAL *pMaterial, 
    STDMETHOD(AddMeshMaterialChildData)(CONST LPD3DXEFFECT *pMaterial, 
                           LPD3DXFILESAVEOBJECT pXofSave, 
                            LPD3DXFILESAVEDATA pXofMeshData) PURE;


    // NOTE: this is called once per Save.  All top level objects should be added using the 
    //    provided interface.  One call adds objects before the frame hierarchy, the other after
    STDMETHOD(AddTopLevelDataObjectsPre)(LPD3DXFILESAVEOBJECT pXofSave) PURE; 
    STDMETHOD(AddTopLevelDataObjectsPost)(LPD3DXFILESAVEOBJECT pXofSave) PURE;                             

    // callbacks for the user to register and then save templates to the XFile
    STDMETHOD(RegisterTemplates)(LPD3DXFILE pXFileApi) PURE;                             
    STDMETHOD(SaveTemplates)(LPD3DXFILESAVEOBJECT pXofSave) PURE;                             
};
*/


//#undef INTERFACE
//#define INTERFACE IDXCCSkinInfo
//DECLARE_INTERFACE_(IDXCCSkinInfo, IDXCCUserData)
//{
//    // IUnknown
//    STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv) PURE;
//    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
//    STDMETHOD_(ULONG, Release)(THIS) PURE;
//
//    STDMETHOD_(DXCCHANDLE, GetHandle)(THIS) PURE;
//	//IDXCCUserData
//    STDMETHOD(GetUserData)(THIS_ LPD3DXBUFFER* ppBuffer) PURE;
//    STDMETHOD(SetUserData)(THIS_ LPD3DXBUFFER pBuffer) PURE;
//
//
//    // Specify the which vertices do each bones influence and by how much
//    STDMETHOD_(DWORD, NumVertices)(THIS) PURE;
//    STDMETHOD_(DWORD, NumBones)(THIS) PURE;
//    STDMETHOD(SetBoneInfluence)(THIS_ DWORD bone, UINT iVertex, FLOAT weight) PURE;
//    STDMETHOD(GetBoneInfluence)(THIS_ DWORD bone, UINT iVertex, FLOAT* weights) PURE;
//
//    // Bone names are returned by D3DXLoadSkinMeshFromXof. They are not used by any other method of this object
//    STDMETHOD(SetBoneFrame)(THIS_ DWORD Bone, LPDXCCFRAME pFrame) PURE; // pName is copied to an internal string buffer
//    STDMETHOD(GetBoneFrame)(THIS_ DWORD Bone, LPDXCCFRAME* ppFrame) PURE; // A pointer to an internal string buffer is returned. Do not free this.
//    
//    // Bone offset matrices are returned by D3DXLoadSkinMeshFromXof. They are not used by any other method of this object
//    STDMETHOD(SetBoneOffsetMatrix)(THIS_ DWORD Bone, CONST D3DXMATRIX *pBoneTransform) PURE; // pBoneTransform is copied to an internal buffer
//    STDMETHOD_(LPD3DXMATRIX, GetBoneOffsetMatrix)(THIS_ DWORD Bone) PURE; // A pointer to an internal matrix is returned. Do not free this.
//
//    // Apply SW skinning based on current pose matrices to the target vertices.
//    STDMETHOD(UpdateSkinnedMesh)(THIS_ 
//        LPDXCCVERTEXBUNDLER pVerticesDst) PURE;
//
//    // Takes a mesh and returns a new mesh with per vertex blend weights and a bone combination
//    // table that describes which bones affect which subsets of the mesh
//    STDMETHOD(ConvertToBlendedMesh)(THIS_ 
//        LPD3DXMESH pMesh,
//        DWORD Options, 
//        CONST DWORD *pAdjacencyIn, 
//        LPDWORD pAdjacencyOut,
//        DWORD* pFaceRemap, 
//        LPD3DXBUFFER *ppVertexRemap, 
//        DWORD* pMaxFaceInfl,
//        DWORD* pNumBoneCombinations, 
//        LPD3DXBUFFER* ppBoneCombinationTable, 
//        LPD3DXMESH* ppMesh) PURE;
//
//    // Takes a mesh and returns a new mesh with per vertex blend weights and indices 
//    // and a bone combination table that describes which bones palettes affect which subsets of the mesh
//    STDMETHOD(ConvertToIndexedBlendedMesh)(THIS_ 
//        LPD3DXMESH pMesh,
//        DWORD Options, 
//        DWORD paletteSize, 
//        CONST DWORD *pAdjacencyIn, 
//        LPDWORD pAdjacencyOut, 
//        DWORD* pFaceRemap, 
//        LPD3DXBUFFER *ppVertexRemap, 
//        DWORD* pMaxVertexInfl,
//        DWORD* pNumBoneCombinations, 
//        LPD3DXBUFFER* ppBoneCombinationTable, 
//        LPD3DXMESH* ppMesh) PURE;
//};



//#undef INTERFACE
//#define INTERFACE IDXCCParent
//DECLARE_INTERFACE_(IDXCCParent, IUnknown)
//{
//	GetParent(LPDXCCFRAME* ppParent)
//	SetParent(LPDXCCFRAME pParent)
//}
//
//#undef INTERFACE
//#define INTERFACE IDXCCMembers
//DECLARE_INTERFACE_(IDXCCMembers, IUnknown)
//
//#undef INTERFACE
//#define INTERFACE IDXCCChildren
//DECLARE_INTERFACE_(IDXCCChildren, IUnknown)
//

#endif //__DXCC_H__
