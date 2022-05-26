/*********************************************************************NVMH4****
Path:  plugins\nvmax
File:  vertexshader.cpp

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

Implements the IVertexShader interface for max.  Responsible for drawing and setting
up the draw state.

Has a notify handler to watch for light nodes arriving/going away

Has texture loading code, effect loading code, and connection parameter setup for
any non-light items (which are handled in lighting.cpp)

Maintains a file event procedure thread which notices when fx files are edited and
re-applies them.


******************************************************************************/

#include "pch.h"
#include "RenderMesh.h"
#include "Material.h"
#include "vertexshader.h"
#include "connectionpblock.h"
#include "nvtexture.h"
#include "nv_dds_common\nv_dds_common.h"
#include "effectmgr.h"
#include "invtexture_d3d9.h"
#include "IGlobalDXDisplayManager.h"

using namespace std;
using namespace nv_dds_common;
using namespace nv_fx;
//using namespace nv_plugin;
using namespace nv_sys;
using namespace nv_renderdevice;

UINT FileEventProc (LPVOID lpParam);

CEffectManager	TheEffectManager;
    

MaxVertexShader::MaxVertexShader(ReferenceTarget *rt) 
: m_pDevice(NULL),
m_RTarget(rt),
m_Draw(false),
m_Ready(false),
m_Mtl(NULL),
m_GWindow(NULL),
m_hTechnique(0),
m_MaxCache(0),
m_CurCache(0),
m_hEvent(NULL),
m_hWatchThread(NULL),
m_Reload(false),
m_bTimeDependant(false),
m_bLightsDirty(true)
{
	NVPROF_FUNC("MaxVertexShader::MaxVertexShader");
	int i;

    m_hEvent = ::CreateEvent( 
        NULL,           // no security attributes
        TRUE,           // manual-reset event
        TRUE,           // initial state is signaled
        "FileEvent"     // object name
        ); 

    if (m_hEvent == NULL)
    {
		LPVOID lpMsgBuf;
		FormatMessage(  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
						FORMAT_MESSAGE_FROM_SYSTEM | 
						FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						GetLastError(),
						0, // Default language
						(LPTSTR) &lpMsgBuf,
						0,
						NULL );
		// Display the string.
		::MessageBox(NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
		// Free the buffer.
		LocalFree( lpMsgBuf );
    }


	DISPDBG(3, "MaxVertexShader::MaxVertexShader");

	m_StdDualVS = NULL;
	m_StdDualVS = (IStdDualVS *)CreateInstance(REF_MAKER_CLASS_ID, 
											   STD_DUAL_VERTEX_SHADER);

	if(m_StdDualVS)
	{
		m_StdDualVS->SetCallback((IStdDualVSCallback*)this);
	}

	m_T	= GetCOREInterface()->GetTime();
	badDevice = false;


	m_pMaterial = (CgFXMaterial *)GetRefTarg();

	if(m_pMaterial)
	{
		m_pDevice = m_pMaterial->GetDevice();
	}

	//lets check for compatibility here, we do it here as ConfirmDevice is 
	//in the draw thread, and you can get continuous error messages
	if(InternalCheck()!=S_OK)
		badDevice = true;
    else
    {
	    for(i=0; i < MAX_MESH_CACHE; i++)
	    {
		    m_Mesh[i] = NULL;
		    m_Node[i] = NULL;
	    }
	    
	    GetCOREInterface()->RedrawViews(m_T);

	    SetDefaultMaxState();
    }
}

CgFXMaterial* MaxVertexShader::GetMaterial()
{
	NVPROF_FUNC("MaxVertexShader::GetMaterial");
    return m_pMaterial;
}

MaxVertexShader::~MaxVertexShader()
{
	NVPROF_FUNC("MaxVertexShader::~MaxVertexShader");
    DISPDBG(3, "MaxVertexShader::~MaxVertexShader");

	StopWatching();

	// Free effect
	GetMaterial()->GetShaderInfo()->FreeEffect_Ver2(OWNER_NONE);

    ::CloseHandle(m_hEvent);

	if(m_StdDualVS)
	{
		m_StdDualVS->DeleteThis();
	}
	m_RTarget = NULL;
}

TimeValue MaxVertexShader::GetTime() const
{
	NVPROF_FUNC("MaxVertexShader::GetTime");
	return m_T;
}

void MaxVertexShader::SetTime(TimeValue T)
{
	NVPROF_FUNC("MaxVertexShader::SetTime");
	m_T = T;
}

LPDIRECT3DDEVICE9 MaxVertexShader::GetDevice()
{
	NVPROF_FUNC("MaxVertexShader::GetDevice");
	return m_pDevice;
}

void MaxVertexShader::SetDevice(LPDIRECT3DDEVICE9 pDevice)
{
	NVPROF_FUNC("MaxVertexShader::SetDevice");
	if (pDevice == m_pDevice)
		return;

	m_pDevice = pDevice;

	// TIM: Reset effect here
	/*if (m_pRenderDevice)
	{
		m_pRenderDevice->UnInitialize();
		if (m_pEffectDevice)
			m_pEffectDevice->UnInitialize();

		m_pRenderDevice->InitializeByDeviceHandle(reinterpret_cast<DWORD>(m_pDevice));
		m_pEffect->Initialize(m_pRenderDevice);
	}*/
}

void MaxVertexShader::SetMeshCache(Mesh *mesh, INode *node, TimeValue T)
{
	int i;
	NVPROF_FUNC("MaxVertexShader::SetMeshCache");

	DISPDBG(4, "MaxVertexShader::SetMeshCache");
	//
	//	Search for node
	//
	for(i=0; i < m_MaxCache; i++)
	{
		if(node == m_Node[i])
		{
			m_CurCache = i;
			break;
		}
	}
	//
	//	Not found
	//	
	if(i == m_MaxCache)
	{
		//
		//	Check for a deleted node slot
		//
		for(i=0; i < m_MaxCache; i++)
		{
			if(m_Node[i] == NULL)
			{
				m_CurCache		   = i;
				m_Mesh[m_CurCache] = mesh;
				m_Node[m_CurCache] = node;
				m_RMesh[m_CurCache].Invalidate();

				break;
			}
		}
		//
		//	Not found
		//
		if(i == m_MaxCache)
		{
			m_CurCache = m_MaxCache; 

			//
			//	Full
			//		
			if(m_MaxCache + 1 >= MAX_MESH_CACHE)
			{
				m_Mesh[m_CurCache] = mesh;
				m_Node[m_CurCache] = node;
				m_RMesh[m_CurCache].Invalidate();

			}
			else
			{	//
				//	Add
				//	
				m_Mesh[m_CurCache] = mesh;
				m_Node[m_CurCache] = node;
				m_RMesh[m_CurCache].Invalidate();

				m_MaxCache++;
			
			}
		}
		
	}

	if(mesh != m_Mesh[m_CurCache])
	{
		m_RMesh[m_CurCache].Invalidate();
		m_Mesh[m_CurCache] = mesh;
	}

	m_T = T;
		
}

HRESULT MaxVertexShader::Initialize(Mesh *mesh, INode *node)
{
	HRESULT hr = E_FAIL;
	NVPROF_FUNC("MaxVertexShader::Initialize");
	DISPDBG(4, "MaxVertexShader::Initialize Mesh");

	if(m_StdDualVS)
	{
		hr = m_StdDualVS->Initialize(mesh, node);
		if (!FAILED(hr))
		{
			SetMeshCache(mesh,node,GetCOREInterface()->GetTime());
			Draw();
		}
	}
	return hr;

}

HRESULT MaxVertexShader::Initialize(MNMesh *mnmesh, INode *node)
{
	HRESULT hr = E_FAIL;
	NVPROF_FUNC("MaxVertexShader::Initialize");
	DISPDBG(3, "MaxVertexShader::Initialize MNMesh");

	if(m_StdDualVS)
	{
		hr = m_StdDualVS->Initialize(mnmesh, node);
		if (!FAILED(hr))
		{
			Mesh msh;
			mnmesh->OutToTri(msh);
			SetMeshCache(&msh,node,GetCOREInterface()->GetTime());
			Draw();
		}
	}
	return hr;
}
 
int MaxVertexShader::GetMatIndex()
{
	NVPROF_FUNC("MaxVertexShader::GetMatIndex");
	int				i;
	Mtl			*Std;
	CgFXMaterial *Shader, *s;

	DISPDBG(4, "MaxVertexShader::GetMatIndex");

	if(m_Mtl->IsMultiMtl())
	{
		Shader = (CgFXMaterial *)GetRefTarg();

		// I use the SubAnims here, as I do not want any NULL material introduced that are not visible to the user
		// this can happen when you have N materials and you select N+1 mat ID - the material will add a NULL material 
		// to the list to compensate - this screws with the index into the paramblock
		for(i=0; i < m_Mtl->NumSubs(); i++)
		{	
			Std = (Mtl*)m_Mtl->SubAnim(i);

			if(Std!=NULL)
			{
				ICustAttribContainer* cc = Std->GetCustAttribContainer();

				if(cc!=NULL)	//might happen if the material has never been loaded....
				{
					for (int kk = 0; kk < cc->GetNumCustAttribs(); kk++)
					{
						CustAttrib *ca = cc->GetCustAttrib(kk);
						IViewportShaderManager *manager = (IViewportShaderManager*)ca->GetInterface(VIEWPORT_SHADER_MANAGER_INTERFACE);
						if (manager) {
							s = (CgFXMaterial*)manager->GetActiveEffect();
						}
					}


					if(s == Shader)
					{
						int id=0;
						// this gets interesting - the MatID are editable !! must get them from the PAramblock
						IParamBlock2 * pblock = m_Mtl->GetParamBlock(0);	// there is only one
						if(pblock)
						{
							ParamBlockDesc2 * pd = pblock->GetDesc();

							for(int j=0;j<pd->count;j++)
							{
								if(strcmp(_T("materialIDList"),pd->paramdefs[j].int_name)==0)	//not localised
								{
									//int id;
									pblock->GetValue(j,0,id,FOREVER,i); 
									id = id+1;	//for some reason this is stored as a zero index, when a 1's based index is used
								}	

							}

						}
						return(id);
					}
				}
			}
		}

	}

	return(0);

}

void MaxVertexShader::Draw(){
	// Trigger Deferred Load, if not yet loaded
	m_pMaterial->TriggerLoad();
	ShaderInfoData* pShader = m_pMaterial->GetShaderInfo()->GetCurrentShader();
	VS_ShaderData& VSShaderData = GetVSShaderData();

	if(!m_Ready || !m_pDevice || badDevice || !VSShaderData.m_pLighting || !pShader->GetEffect() || !pShader->GetParameterList())
	{
		m_Draw = false;
		return;
	}

	m_Draw				= true;
	CgFXMaterial* pMat	= (CgFXMaterial *)GetRefTarg();
	m_Mtl				= m_Node[m_CurCache]->GetMtl();
	bool NegScale		= TMNegParity(m_Node[m_CurCache]->GetObjectTM(m_T));

	// Info on the mesh
	if (!m_RMesh[m_CurCache].IsValid())
	{
		DISPDBG(3, "Mesh not valid: " << m_Node[m_CurCache]->GetName());
	}

	//	Rebuild mesh if needed, or nuke it if necessary
	if(!m_RMesh[m_CurCache].Evaluate(m_pDevice, m_Mesh[m_CurCache],GetMatIndex(),NegScale))
	{
		m_Draw = false;
		return;
	}

	// Setup the connections
	SetupConnectionParameters();

	// HACK to force the state manager default state.  Don't know how to interact with MAX state...
	//pShader->GetEffect()->OnResetDevice();
	pShader->GetConnectionManager()->ApplyEffect(pShader->GetEffect(), pShader->GetParameterList());

	// TIM: Default alpha states
	m_pDevice->SetRenderState( D3DRS_ZENABLE, TRUE);
	m_pDevice->SetRenderState( D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
	m_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE);
	m_pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE);
	m_pDevice->SetRenderState( D3DRS_ALPHAREF , 0xFF);
	m_pDevice->SetRenderState( D3DRS_ZWRITEENABLE , TRUE);
	m_pDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
	m_pDevice->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_SRCALPHA); 
	m_pDevice->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA); 
	// TIM: This doesn't get set properly by max on multi/sub matierlas
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	unsigned int dwPasses;

	HRESULT hr = pShader->GetEffect()->Begin(&dwPasses, 0);
	if (SUCCEEDED(hr))
	{
		for (int i = 0; i < dwPasses; i++)
		{
			pShader->GetEffect()->BeginPass(i);
			if (!m_RMesh[m_CurCache].Render(m_pDevice))
			{
				m_Draw = false;
			}
			pShader->GetEffect()->EndPass();
		}
		pShader->GetEffect()->End();
	}
	else
	{
		m_Draw = false;
#ifdef _DEBUG
		DISPDBG(0,DXGetErrorString9(hr));
#endif
		// TIM: *Reload* the effect, must have lost device
		SetReload(TRUE);
	}

	//SetDefaultMaxState();

	//DISPDBG(5, "MaxVertexShader::Draw - Exit");
}

void MaxVertexShader::SetDefaultMaxState()
{ 
	NVPROF_FUNC("MaxVertexShader::SetDefaultMaxState");
	if (!m_pMaterial)
		return;
   
	if (!m_pDevice)
	{
		m_pDevice = m_pMaterial->GetDevice();
	}

	if (!m_pDevice)
		return;

	// Reset the state to keep max happy
	m_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_FOGCOLOR, 0xFF0000);
	m_pDevice->SetRenderState(D3DRS_FOGTABLEMODE,   D3DFOG_NONE );
	m_pDevice->SetRenderState(D3DRS_FOGVERTEXMODE,  D3DFOG_LINEAR );
	m_pDevice->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE );
	m_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_DITHERENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_CLIPPING, TRUE);
	m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_COLORVERTEX, FALSE);

    m_pDevice->SetFVF( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 );
	m_pDevice->SetVertexShader(NULL);
    m_pDevice->SetPixelShader( NULL );

	for(int i=0;i<8;i++)
	{
		m_pDevice->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
		m_pDevice->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		m_pDevice->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		m_pDevice->SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		m_pDevice->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		m_pDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, i);
		m_pDevice->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		m_pDevice->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		m_pDevice->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, 0);

	}
	
	// TIM: Disabled this. It's gay
	//	Hack to keep max from drawing over 
	//	the top me sometimes
	//  CM - taken from the metalbump sample...
	//
/*	m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,TRUE);
	m_pDevice->SetRenderState(D3DRS_ALPHAREF,0xFE);
	m_pDevice->SetRenderState(D3DRS_ALPHAFUNC,D3DCMP_LESS);//D3DCMP_GREATEREQUAL);
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);*/
}


HRESULT MaxVertexShader::InternalCheck()
{
	NVPROF_FUNC("MaxVertexShader::InternalCheck");
	D3DCAPS9		Caps;
	D3DDISPLAYMODE	Mode;
		
	m_Ready = false;
	bool ref = false;
	
	DISPDBG(3, "MaxVertexShader::InternalCheck");

	if(m_pDevice)
	{
		m_pDevice->GetDeviceCaps(&Caps);
		m_pDevice->GetDisplayMode(0, &Mode);

		// for those mad Laptop users
		if(Caps.DeviceType == D3DDEVTYPE_REF)
			ref = true;
		
		if(!ref)
		{
			if(Mode.Format != D3DFMT_A8R8G8B8 && Mode.Format != D3DFMT_X8R8G8B8)
			{
				TSTR	Str;
				Str = TSTR("Desktop needs to be True Color (32bit).\nPlease adjust and try again.");
				MessageBox(NULL,Str,"Error",MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_APPLMODAL);
				return(E_FAIL);
			}	 
			if(!(Caps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP) || 
			   !(Caps.TextureCaps & D3DPTEXTURECAPS_PROJECTED))
			{
				TSTR	Str;
				Str = TSTR("No cube or projected texture support.\nNeed a GForce3 or better card.");
				MessageBox(NULL,Str,"Error",MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_APPLMODAL);

				m_GWindow = NULL;

				return(E_FAIL);
			}
		}
	}
    else // there is no device!!!
        return(E_FAIL);

	return(S_OK);

}

HRESULT MaxVertexShader::ConfirmDevice(ID3D9GraphicsWindow *d3dgw)
{
	NVPROF_FUNC("MaxVertexShader::ConfirmDevice");
	DISPDBG(4, "MaxVertexShader::ConfirmDevice");

	m_pDevice = d3dgw->GetDevice();
	
	m_Ready = false;
	bool ref = false;


	if(m_pDevice && !badDevice)
	{

		m_GWindow = d3dgw;
		m_Ready   = true;
        return(S_OK);
	}

    return(E_FAIL);
}

HRESULT MaxVertexShader::InitValid(Mesh *mesh, INode *node)
{	
	NVPROF_FUNC("MaxVertexShader::InitValid");
	DISPDBG(3, "MaxVertexShader::InitValid - Mesh");	

	CgFXMaterial* pMat = (CgFXMaterial *)GetRefTarg();
	
	if(pMat)
	{
		assert(pMat->GetVertexShader() == this);
		if(pMat->GetVertexShader())
		{
			pMat->GetVertexShader()->SetMeshCache(mesh, node, GetCOREInterface()->GetTime());
			pMat->GetVertexShader()->m_RMesh[pMat->GetVertexShader()->m_CurCache].Invalidate();			
			
			// Force a redraw when the mesh changes.
			//GetCOREInterface()->ForceCompleteRedraw();			
			
		}
	}
	return(S_OK);
}

HRESULT MaxVertexShader::InitValid(MNMesh *mnmesh, INode *node)
{
	NVPROF_FUNC("MaxVertexShader::InitValid");
	DISPDBG(3, "MaxVertexShader::InitValid - MNMesh");	
	return(S_OK);
}



HRESULT MaxVertexShader::ConfirmPixelShader(IDX9PixelShader *PS)
{
	NVPROF_FUNC("MaxVertexShader::ConfirmPixelShader");
	return(S_OK);
}

ReferenceTarget *MaxVertexShader::GetRefTarg()
{
	NVPROF_FUNC("MaxVertexShader::GetRefTarg");
	return(m_RTarget);
}

VertexShaderCache *MaxVertexShader::CreateVertexShaderCache()
{
	NVPROF_FUNC("MaxVertexShader::CreateVertexShaderCache");
	return(new IDX9VertexShaderCache);
}


bool MaxVertexShader::DrawMeshStrips(ID3D9GraphicsWindow *d3dgw, MeshData *data)
{
	NVPROF_FUNC("MaxVertexShader::DrawMeshStrips");
	return(m_Draw);
}

bool MaxVertexShader::DrawWireMesh(ID3D9GraphicsWindow *d3dgw, WireMeshData *data)
{
	NVPROF_FUNC("MaxVertexShader::DrawWireMesh");
	return(m_Draw);
}

void MaxVertexShader::StartLines(ID3D9GraphicsWindow *d3dgw, WireMeshData *data)
{
	NVPROF_FUNC("MaxVertexShader::StartLines");
}

void MaxVertexShader::AddLine(ID3D9GraphicsWindow *d3dgw, DWORD *vert, int vis)
{
	NVPROF_FUNC("MaxVertexShader::AddLine");
}

bool MaxVertexShader::DrawLines(ID3D9GraphicsWindow *d3dgw)
{
	NVPROF_FUNC("MaxVertexShader::DrawLines");
	return(m_Draw);
}

void MaxVertexShader::EndLines(ID3D9GraphicsWindow *d3dgw, GFX_ESCAPE_FN fn)
{
	NVPROF_FUNC("MaxVertexShader::EndLines");
}

void MaxVertexShader::StartTriangles(ID3D9GraphicsWindow *d3dgw, MeshFaceData *data)
{
	NVPROF_FUNC("MaxVertexShader::StartTriangles");
}

void MaxVertexShader::AddTriangle(ID3D9GraphicsWindow *d3dgw, DWORD index, int *edgeVis)
{
	NVPROF_FUNC("MaxVertexShader::AddTriangle");
}

bool MaxVertexShader::DrawTriangles(ID3D9GraphicsWindow *d3dgw)
{
	NVPROF_FUNC("MaxVertexShader::DrawTriangles");
	return(m_Draw);
}

void MaxVertexShader::EndTriangles(ID3D9GraphicsWindow *d3dgw, GFX_ESCAPE_FN fn)
{
	NVPROF_FUNC("MaxVertexShader::EndTriangles");
}

HRESULT MaxVertexShader::SetVertexShader(ID3D9GraphicsWindow *d3dgw, int numPass)
{
	NVPROF_FUNC("MaxVertexShader::SetVertexShader");
	return(S_OK);
}

bool MaxVertexShader::CanTryStrips()
{
	NVPROF_FUNC("MaxVertexShader::CanTryStrips");
	return(true);
}

int MaxVertexShader::GetNumMultiPass()
{
	NVPROF_FUNC("MaxVertexShader::GetMultiPass");
	return(1);
}

// Get private data associated with the current shader.  Where we store the light class and the
// conneciton parameters.
VS_ShaderData& MaxVertexShader::GetVSShaderData()
{
	NVPROF_FUNC("MaxVertexShader::GetVSShaderData");
	void* pVSData = m_pMaterial->GetShaderInfo()->GetCurrentShader()->GetVSData();
	assert(m_pMaterial->GetShaderInfo() && m_pMaterial->GetShaderInfo()->GetCurrentShader());

	if (!pVSData)
	{
		VS_ShaderData* pData = new VS_ShaderData;
		ZeroMemory(pData, sizeof(VS_ShaderData));
		m_pMaterial->GetShaderInfo()->GetCurrentShader()->SetVSData(pData);
		return *pData;
	}
	return *(static_cast<VS_ShaderData*>(pVSData));
}

bool MaxVertexShader::SetupConnectionParameters()
{
	//NVPROF_FUNC("MaxVertexShader::SetupConnectionParameters");
	D3DXMATRIX	MatWorld;
	D3DXMATRIX	MatView;
	D3DXMATRIX	MatProj;

	//DISPDBG(4, "MaxVertexShader::SetupConnectionParameters");

	VS_ShaderData& VSShaderData = GetVSShaderData();
	ShaderInfoData* pShader = m_pMaterial->GetShaderInfo()->GetCurrentShader();

	if (!pShader->GetParameterList())
		return false;

	MatWorld = m_GWindow->GetWorldXform();
	MatView  = m_GWindow->GetViewXform();
	MatProj  = m_GWindow->GetProjXform();

	m_matWorld = *(mat4*)&MatWorld;
	m_matView = *(mat4*)&MatView;
	m_matProjection = *(mat4*)&MatProj;


	TimeValue t = GetCOREInterface()->GetTime();

	// Matrix calculations up front
	m_matWorldView = m_matView * m_matWorld;
	m_matWorldViewProjection = m_matProjection * m_matWorldView;

    for (unsigned int i = 0; i < pShader->GetParameterList()->GetNumParameters(); i++)
	{
		INVConnectionParameter* pParam = pShader->GetParameterList()->GetConnectionParameter(i);

		eSEMANTICID SemanticID = pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID();

		if (VSShaderData.m_pLighting->ApplyAttachedNode(pParam,m_pMaterial))
			continue;
   
		switch (SemanticID)
		{
			case SEMANTICID_TIME:
			{
				m_bTimeDependant = true;
				pParam->SetKey(t, NVType::CreateFloatType((float)timeGetTime() / 1000.0f));
			}
			break;
			case SEMANTICID_WORLDTRANSPOSE:
			{
				pParam->SetKey(t, NVType::CreateMatrixType(m_matWorld.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_VIEWTRANSPOSE:
			{
				pParam->SetKey(t, NVType::CreateMatrixType(m_matView.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_PROJECTIONTRANSPOSE:
			{
				pParam->SetKey(t, NVType::CreateMatrixType(m_matProjection.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_WORLDVIEWTRANSPOSE:
			{
				pParam->SetKey(t, NVType::CreateMatrixType(m_matWorldView.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_WORLDVIEWPROJECTIONTRANSPOSE:
			{
				pParam->SetKey(t, NVType::CreateMatrixType(m_matWorldViewProjection.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_WORLD:
			{
				mat4 matWorld = m_matWorld;
				//transpose(matWorld);
				pParam->SetKey(t, NVType::CreateMatrixType(matWorld.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_VIEW:
			{
				mat4 matView = m_matView;
				//transpose(matView);
				pParam->SetKey(t, NVType::CreateMatrixType(matView.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_PROJECTION:
			{
				mat4 matProjection = m_matProjection;
				//transpose(matProjection);
				pParam->SetKey(t, NVType::CreateMatrixType(matProjection.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_WORLDVIEW:
			{
				mat4 matWorldView = m_matWorldView;
				//transpose(matWorldView);
				pParam->SetKey(t, NVType::CreateMatrixType(matWorldView.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_WORLDVIEWPROJECTION:
			{
				mat4 matWorldViewProjection = m_matWorldViewProjection;
				//transpose(matWorldViewProjection);
				pParam->SetKey(t, NVType::CreateMatrixType(matWorldViewProjection.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_WORLDINVERSETRANSPOSE:
			{
				mat4 matWorldIT;
				invert(matWorldIT, m_matWorld);
				pParam->SetKey(t, NVType::CreateMatrixType(matWorldIT.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_VIEWINVERSETRANSPOSE:
			{
				mat4 matViewIT;
				invert(matViewIT, m_matView);
				pParam->SetKey(t, NVType::CreateMatrixType(matViewIT.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_PROJECTIONINVERSETRANSPOSE:
			{
				mat4 matProjectionIT;
				invert(matProjectionIT, m_matProjection);
				pParam->SetKey(t, NVType::CreateMatrixType(matProjectionIT.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_WORLDVIEWINVERSETRANSPOSE:
			{
				mat4 matWorldViewIT;
				invert(matWorldViewIT, m_matWorldView);
				pParam->SetKey(t, NVType::CreateMatrixType(matWorldViewIT.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_WORLDVIEWPROJECTIONINVERSETRANSPOSE:
			{
				mat4 matWorldViewProjectionIT;
				invert(matWorldViewProjectionIT, m_matWorldViewProjection);
				pParam->SetKey(t, NVType::CreateMatrixType(matWorldViewProjectionIT.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_WORLDINVERSE:
			{
				mat4 matWorldI;
				invert(matWorldI, m_matWorld);
				//transpose(matWorldI);
				pParam->SetKey(t, NVType::CreateMatrixType(matWorldI.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_VIEWINVERSE:
			{
				mat4 matViewI;
				invert(matViewI, m_matView);
				//transpose(matViewI);
				pParam->SetKey(t, NVType::CreateMatrixType(matViewI.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_PROJECTIONINVERSE:
			{
				mat4 matProjectionI;
				invert(matProjectionI, m_matProjection);
				//transpose(matProjectionI);
				pParam->SetKey(t, NVType::CreateMatrixType(matProjectionI.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_WORLDVIEWINVERSE:
			{
				mat4 matWorldViewI;
				invert(matWorldViewI, m_matWorldView);
				//transpose(matWorldViewI);
				pParam->SetKey(t, NVType::CreateMatrixType(matWorldViewI.mat_array, 4, 4));
			}
			break;
			case SEMANTICID_WORLDVIEWPROJECTIONINVERSE:
			{
				mat4 matWorldViewProjectionI;
				invert(matWorldViewProjectionI, m_matWorldViewProjection);
				//transpose(matWorldViewProjectionI);
				pParam->SetKey(t, NVType::CreateMatrixType(matWorldViewProjectionI.mat_array, 4, 4));
			}
			break;

			default:
				DISPDBG(4, "UNKNOWN Param: " << pParam->GetDefaultValue().GetObjectSemantics()->GetName() << ", SEMANTIC: " << ConvertSemantic(pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID()));
				break;
		}
	}
	return true;
}

bool MaxVertexShader::IsEditableParam(nv_sys::INVConnectionParameter* pParam)
{
	// Other things we set
	switch(pParam->GetDefaultValue().GetObjectSemantics()->GetSemanticID())
	{
		case SEMANTICID_TIME:
		case SEMANTICID_WORLDTRANSPOSE:
		case SEMANTICID_VIEWTRANSPOSE:
		case SEMANTICID_PROJECTIONTRANSPOSE:
		case SEMANTICID_WORLDVIEWTRANSPOSE:
		case SEMANTICID_WORLDVIEWPROJECTIONTRANSPOSE:
		case SEMANTICID_WORLD:
		case SEMANTICID_VIEW:
		case SEMANTICID_PROJECTION:
		case SEMANTICID_WORLDVIEW:
		case SEMANTICID_WORLDVIEWPROJECTION:
		case SEMANTICID_WORLDINVERSETRANSPOSE:
		case SEMANTICID_VIEWINVERSETRANSPOSE:
		case SEMANTICID_PROJECTIONINVERSETRANSPOSE:
		case SEMANTICID_WORLDVIEWINVERSETRANSPOSE:
		case SEMANTICID_WORLDVIEWPROJECTIONINVERSETRANSPOSE:
		case SEMANTICID_WORLDINVERSE:
		case SEMANTICID_VIEWINVERSE:
		case SEMANTICID_PROJECTIONINVERSE:
		case SEMANTICID_WORLDVIEWINVERSE:
		case SEMANTICID_WORLDVIEWPROJECTIONINVERSE:
			return false;
		break;
	}

	return true;
}

int MaxVertexShader::FreeTexture(INVTexture* pTexture)
{
	NVPROF_FUNC("MaxVertexShader::FreeTexture");
	return TextureMgr::GetSingletonPtr()->Release(pTexture);
}

// TIM: Added from RenderDevice
bool CreateTextureFromHandle(NVTEX_HANDLE Handle, INVTexture** ppTexture)
{
	INVTexture_D3D9* pTexD3D9 = CreateNVTexture_D3D9();;
	pTexD3D9->SetTexture((IDirect3DTexture9*)Handle);
	pTexD3D9->GetInterface(IID_INVTexture, (void**)ppTexture);
	SAFE_RELEASE(pTexD3D9);
	return true;
}

INVTexture* MaxVertexShader::LoadBumpTexture(const char* pszFilePath, INVConnectionParameter* pTexture)
{
	NVPROF_FUNC("MaxVertexShader::LoadBumpTexture");
	std::ostringstream strStream;

	TSTR p,f,e;
	TSTR strPath(pszFilePath);
	SplitFilename(strPath, &p, &f, &e);

	strStream << (char*)p << "\\" << (char*)f << "_normal.dds" << ends;

    // Try to find it in the texture cache based on the search name
	int Index = TextureMgr::GetSingletonPtr()->Find(strStream.str().c_str());
	if (Index < 0)
	{
		INVTexture* pTexNormal;

		// Try to load the normal map as is.
		if (!FindFile(strStream.str().c_str()).empty())
		{
			pTexNormal = LoadTexture(strStream.str().c_str(), pTexture);
			if (pTexNormal)
				return pTexNormal;
		}

		// Load the bump map.
		INVTexture* pTexBump = LoadTexture(pszFilePath, pTexture);
		if (!pTexBump)
			return NULL;

		// Create the normal map
		IDirect3DTexture9* pTexNormalDevice;
		pTexNormalDevice = static_cast<IDirect3DTexture9*>(NVTexture2::CreateNormalMap(GetDevice(), reinterpret_cast<IDirect3DTexture9*>(pTexBump->GetTextureHandle()), D3DFMT_A8R8G8B8));
		if (!pTexNormalDevice)
			return NULL;

		// Write out a copy
	    D3DXSaveTextureToFile(strStream.str().c_str(), D3DXIFF_DDS, pTexNormalDevice, NULL);

		INVTexture* pNVTex = NULL;
		CreateTextureFromHandle((NVTEX_HANDLE)pTexNormalDevice, &pNVTex);
		TextureMgr::GetSingletonPtr()->Add(strStream.str().c_str(), NVTEXTURE_2D, pNVTex);
		SAFE_RELEASE(pTexNormalDevice);
		pNVTex->Release();
		assert(pNVTex);
		return pNVTex;
	}

	return TextureMgr::GetSingletonPtr()->Get(Index);
}

INVTexture* MaxVertexShader::LoadTexture(const char* pszFilePath, INVConnectionParameter* pTexture)
{
	NVPROF_FUNC("MaxVertexShader::LoadTexture");
	NVTEXTURETARGETTYPE TargetType = NVTEXTURE_UNKNOWN;

	DISPDBG(3, "Texture: " << pszFilePath << ", Semantic: " << ConvertSemantic(pTexture->GetDefaultValue().GetObjectSemantics()->GetSemanticID()));
	switch (pTexture->GetDefaultValue().GetObjectSemantics()->GetSemanticID())
	{
		case SEMANTICID_1DMAP:
			TargetType = NVTEXTURE_1D;
			break;

		case SEMANTICID_BUMPMAP:
		case SEMANTICID_NORMAL:
		case SEMANTICID_DIFFUSEMAP:
		case SEMANTICID_SPECULARMAP:
		case SEMANTICID_GLOSSMAP:
		case SEMANTICID_DIRTMAP:
		case SEMANTICID_SELFILLUMINATIONMAP:
		case SEMANTICID_2DMAP:
			TargetType = NVTEXTURE_2D;
			break;

		case SEMANTICID_3DMAP:
			TargetType = NVTEXTURE_3D;
			break;

		case SEMANTICID_NORMALIZATIONMAP:
		case SEMANTICID_CUBEMAP:
		case SEMANTICID_ENVMAP:
			TargetType = NVTEXTURE_CUBE;
			break;
		default:
			TargetType = NVTEXTURE_UNKNOWN;
			break;
	}

	if (TargetType == NVTEXTURE_UNKNOWN)
	{
		// Try to find it from annotations...
		eANNOTATIONVALUEID ValueID = pTexture->GetDefaultValue().GetObjectSemantics()->FindAnnotationValue(ANNOTATIONNAMEID_TEXTURETYPE);
		switch (ValueID)
		{
			case ANNOTATIONVALUEID_1D:
				TargetType = NVTEXTURE_1D;
				break;
			case ANNOTATIONVALUEID_2D:
				TargetType = NVTEXTURE_2D;
				break;
			case ANNOTATIONVALUEID_3D:
				TargetType = NVTEXTURE_3D;
				break;
			case ANNOTATIONVALUEID_CUBE:
				TargetType = NVTEXTURE_CUBE;
				break;
			default:
				//assert(!"Unknown texture target type!");
				//TargetType = NVTEXTURE_UNKNOWN;
				TargetType = NVTEXTURE_2D;
				break;
		}
	}

	assert(TargetType != NVTEXTURE_UNKNOWN);

	return LoadTexture(pszFilePath, TargetType);
}

void MaxVertexShader::StartWatching ()
{
	NVPROF_FUNC("MaxVertexShader::StartWatching");
    if (!m_hWatchThread)
    {
        m_ThreadID = GetCurrentThreadId();

        ::ResetEvent (m_hEvent);
        m_hWatchThread = CreateThread(NULL, 0, 
            (LPTHREAD_START_ROUTINE) FileEventProc, 
            this,                    
            0, NULL); 
        if (m_hWatchThread == NULL)
        {
		    LPVOID lpMsgBuf;
		    FormatMessage(  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
						    FORMAT_MESSAGE_FROM_SYSTEM | 
						    FORMAT_MESSAGE_IGNORE_INSERTS,
						    NULL,
						    GetLastError(),
						    0, // Default language
						    (LPTSTR) &lpMsgBuf,
						    0,
						    NULL );
		    // Display the string.
		    ::MessageBox(NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
		    // Free the buffer.
		    LocalFree( lpMsgBuf );

        }
    }
}

void MaxVertexShader::StopWatching ()
{
	NVPROF_FUNC("MaxVertexShader::StopWatching");
	DISPDBG(4, "MaxVertexShader::StopWatching");

    if (m_hWatchThread)
    {
        ::SetEvent (m_hEvent);

        // Wait for the file event thread to exit.
        WaitForSingleObject(m_hWatchThread, INFINITE);
        ::CloseHandle(m_hWatchThread);

        m_hWatchThread = NULL;
    }
}

INVTexture* MaxVertexShader::LoadTexture(const char* pszSearchName, const NVTEXTURETARGETTYPE& TargetType)
{
	NVPROF_FUNC("MaxVertexShader::LoadTexture");
	std::string strFilePath;
	INVTexture* pNVTex = NULL;
	IDirect3DBaseTexture9* pTexture = NULL;

	DISPDBG(5, "MaxVertexShader::LoadTexture");
	DISPDBG(0, "SearchName : " << pszSearchName << ", Target: " << TargetType);

	if (!pszSearchName || (strlen(pszSearchName) == 0))
		return NULL;

	// First try to find it in the texture cache based on the search name
	int Index = TextureMgr::GetSingletonPtr()->Find(pszSearchName);
	if (Index < 0)
	{
		DISPDBG(3, "MaxVertexShader::LoadTexture - finding file");

		// If we couldn't find it, try remapping it.
		strFilePath = FindFile(pszSearchName);
		static bool showMissingFiles = true;
		if (strFilePath.empty() && showMissingFiles)
		{
			//\nNote: In future you may want to add the location of your .fx directory to the MAX search paths\nTo do this:\nFrom MAX’s main menu; Customize, Configure Paths…, External Files tab, Add…, then add the paths\nTypically this will be something like: C:\\Helix Core\\Shaders\\Standard"

			// TODO: Make this build a list on init, rather than error
			DISPDBG(0, "ERROR: Failed to load: " << pszSearchName);
			ostringstream strStream;
			strStream << "ERROR: Failed to load: " << pszSearchName << ". \nClick 'OK' to be alerted of further missing textures, or 'Cancel' to hide all warnings for the remainder of this session";
			DWORD ret = MessageBox(NULL, strStream.str().c_str(), "Error", MB_ICONEXCLAMATION | MB_OKCANCEL);
			if(ret == IDCANCEL)
				showMissingFiles = false;
			return NULL;
		}
		else if(strFilePath.empty()){
			return NULL;
		}
 
		Index = TextureMgr::GetSingletonPtr()->Find(strFilePath.c_str());
	}
	 
	if (Index < 0)
	{
		DISPDBG(3, "MaxVertexShader::LoadTexture - creating");

		HRESULT hr = E_FAIL;

		switch (TargetType)
		{
		case NVTEXTURE_1D:
			assert(false);
			break;
		case NVTEXTURE_2D:
			pTexture = static_cast<IDirect3DBaseTexture9*>(LoadDDSTexture2D(strFilePath.c_str()));
			break;
		case NVTEXTURE_CUBE:
			pTexture = static_cast<IDirect3DBaseTexture9*>(LoadDDSTextureCube(strFilePath.c_str()));
			break;
		case NVTEXTURE_3D:
			pTexture = static_cast<IDirect3DBaseTexture9*>(LoadDDSTexture3D(strFilePath.c_str()));
			break;
		case NVTEXTURE_RECT:
			assert(false);
			break;
		default:
			assert(false);
		}

		if (pTexture == NULL)
		{
			DISPDBG(0, "ERROR: Failed to create: " << strFilePath);
			ostringstream strStream;
			strStream << "ERROR: Failed to create: " << strFilePath;
			MessageBox(NULL, strStream.str().c_str(), "Error", MB_ICONEXCLAMATION | MB_OK);
			return NULL;
		}

		CreateTextureFromHandle((NVTEX_HANDLE)pTexture, &pNVTex);
		TextureMgr::GetSingletonPtr()->Add(strFilePath.c_str(), TargetType, pNVTex);
		assert(pNVTex);
		return pNVTex;
	}
	else
	{
		pNVTex = TextureMgr::GetSingletonPtr()->Get(Index);
		assert(pNVTex && "Couldn't find texture!");
	}
	
	return pNVTex;
}

Lighting* MaxVertexShader::GetLighting()
{
	NVPROF_FUNC("MaxVertexShader::GetLighting");
	return GetVSShaderData().m_pLighting;
}

void MaxVertexShader::SetReload(bool bReload)
{
	NVPROF_FUNC("MaxVertexShader::SetReload");
	ShaderInfoData* pShader = m_pMaterial->GetShaderInfo()->GetCurrentShader();
    m_Reload = true;

	m_pMaterial->GetShaderInfo()->UpdateEffect_Ver2();

}

UINT FileEventProc (LPVOID lpParam)
{
	//NVPROF_FUNC("FileEventProc");
	MaxVertexShader *pVS = (MaxVertexShader*) lpParam;
	std::list<std::string> listPaths;

	listPaths.push_back(pVS->GetMaterial()->GetShaderInfo()->GetCurrentShader()->GetEffectFile());
	int nFilePathCount = listPaths.size();
	
    HANDLE *phChanges = new HANDLE[nFilePathCount + 1];
    int i;
    int j = 0;
    
    for (i = 1; i < nFilePathCount + 1; i++)
    {
        phChanges[i] = INVALID_HANDLE_VALUE;
    }

    *phChanges = pVS->m_hEvent;
    std::list<std::string>::iterator itrEffects = listPaths.begin();
    std::vector<std::string> path_list;
    
    while (itrEffects != listPaths.end())
    {
        std::string path = (*itrEffects);
		std::string filename = path;
		if(path.find("\\") != -1)
			filename = strrchr(filename.c_str(),'\\');

		int pos = path.rfind(filename);
		if (pos < path.max_size())
			path.resize(pos);
		else
			path = "";

		std::vector<std::string>::iterator itrFound;
		if (!path.empty())
		{
			itrFound = std::find(path_list.begin(), path_list.end(), path);
			if (itrFound == path_list.end())
			{
				path_list.push_back(path);
				HANDLE hFC = FindFirstChangeNotification (path.c_str(), FALSE,
							FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_FILE_NAME);

				if (hFC != INVALID_HANDLE_VALUE) // if i can gen a notification --
					phChanges[++j]= hFC;
				else
				{
					LPVOID lpMsgBuf;
					FormatMessage(  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
									FORMAT_MESSAGE_FROM_SYSTEM | 
									FORMAT_MESSAGE_IGNORE_INSERTS,
									NULL,
									GetLastError(),
									0, // Default language
									(LPTSTR) &lpMsgBuf,
									0,
									NULL );
					// Display the string.
					::MessageBox(NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
					// Free the buffer.
					LocalFree( lpMsgBuf );
				}
			}
        }
        ++itrEffects;
    }

    DWORD dwResult = WaitForMultipleObjects (j + 1, phChanges, FALSE, INFINITE);
    
    for (i = 1; i < j; i++)
		FindCloseChangeNotification(phChanges[i + 1]);
    
    // Wait for a stop event or a file change event.
    int nPath = dwResult - WAIT_OBJECT_0;
    if (nPath)
	{
        // File change event, so post a message to the main app thread.
        while (!PostThreadMessage(pVS->GetThreadID(), MESSAGE_FILECHANGE, 0, (long)pVS))
            Sleep(0);
    }

    // Close and delete the handles
    for (i = 1; i < j; i++)
    {
        if (phChanges[i] != INVALID_HANDLE_VALUE)
            CloseHandle(phChanges[i]);
    }
    delete [] phChanges;

    return 0;
}



bool MaxVertexShader::RemoveEffect()
{
	NVPROF_FUNC("MaxVertexShader::RemoveEffect");
	DISPDBG(4, "MaxVertexShader::RemoveEffect");

	ShaderInfoData* pShader = m_pMaterial->GetShaderInfo()->GetCurrentShader();

	if (pShader->GetEffect())
	{
		if (pShader->GetEffect() && pShader->GetConnectionManager())
		{
			// Remove the lighting parameters
			if (GetVSShaderData().m_pLighting)
			{	
				SAFE_DELETE(GetVSShaderData().m_pLighting);
			}
		}
	}

	assert(GetVSShaderData().m_pLighting == NULL);

	return true;
}

bool MaxVertexShader::SetupEffect()
{
	NVPROF_FUNC("MaxVertexShader::SetupEffect");

	// If we have a scene update, remove connections
	DISPDBG(4, "MaxVertexShader::SetupEffect");

	// Get the current shader info
	ShaderInfoData* pShader = m_pMaterial->GetShaderInfo()->GetCurrentShader();

	// Add lighting handler
	GetVSShaderData().m_pLighting = new Lighting(m_pMaterial->GetShaderInfo());

	DirtyLights(false);

	return true;
}

bool MaxVertexShader::FreeTextures()
{
	NVPROF_FUNC("MaxVertexShader::FreeTextures");

	// Free the application parameters
	ShaderInfoData* pShader = m_pMaterial->GetShaderInfo()->GetCurrentShader();

	if (!pShader->GetParameterList())
		return true;

	DISPDBG(3, "MaxVertexShader::FreeTextures");

	for (unsigned int i = 0; i < pShader->GetParameterList()->GetNumParameters(); i++)
	{
		INVConnectionParameter* pEffectParam = pShader->GetParameterList()->GetConnectionParameter(i);
		if ((pEffectParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE) ||
			(pEffectParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE2D) ||
			(pEffectParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE3D) ||
			(pEffectParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURECUBE))
		{
			if (pEffectParam->GetValueAtTime(GetCOREInterface()->GetTime()).GetTexture())
			{
				// Free tex, and set connection to 0 if the ref count goes.
				nv_sys::NVType Texture(pEffectParam->GetDefaultValue());
				Texture.SetTexture(0);
				if (FreeTexture((pEffectParam->GetValueAtTime(GetCOREInterface()->GetTime()).GetTexture())) == 0)
					pEffectParam->SetKey(GetCOREInterface()->GetTime(), Texture);
			}
		}


	}
	return true;
}

// We addref the textures owned by the effect in the texture manager, because
// we free them at the end (as we might swap them for others along the way).
bool MaxVertexShader::AddRefEffectTextures()
{
	NVPROF_FUNC("MaxVertexShader::AddRefEffectTextures");

	// Addref effect-owned textures
	ShaderInfoData* pShader = m_pMaterial->GetShaderInfo()->GetCurrentShader();

	if (!pShader->GetParameterList())
		return true;

	DISPDBG(4, "MaxVertexShader::AddRefTextures");

	for (unsigned int i = 0; i < pShader->GetParameterList()->GetNumParameters(); i++)
	{
		INVConnectionParameter* pEffectParam = pShader->GetParameterList()->GetConnectionParameter(i);

		if (pEffectParam && ((pEffectParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE) ||
							 (pEffectParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE2D) ||
							 (pEffectParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURE3D) ||
							 (pEffectParam->GetDefaultValue().GetType() == NVTYPEID_TEXTURECUBE)))

		{
			if (pEffectParam->GetValueAtTime(GetCOREInterface()->GetTime()).GetTexture())
			{
				TextureMgr::GetSingletonPtr()->AddRef((pEffectParam->GetValueAtTime(GetCOREInterface()->GetTime()).GetTexture()));
			}
		}

	}
	return true;
}

bool MaxVertexShader::SetTechnique(unsigned int Technique)
{
	LPD3DXEFFECT pEffect = m_pMaterial->GetShaderInfo()->GetCurrentShader()->GetEffect();
	if (!pEffect)
		return false;

	D3DXEFFECT_DESC Desc;
	pEffect->GetDesc(&Desc);
	if (Technique >= Desc.Techniques)
        return false;

	NVFXHANDLE hTechnique = pEffect->GetTechnique(Technique);
	D3DXTECHNIQUE_DESC TechniqueDesc;
	pEffect->GetTechniqueDesc(hTechnique, &TechniqueDesc);
	return SetTechnique((char*)TechniqueDesc.Name);
}

bool MaxVertexShader::SetTechnique(const char* Technique)
{
	NVPROF_FUNC("MaxVertexShader::SetTechnique");

	LPD3DXEFFECT pEffect = m_pMaterial->GetShaderInfo()->GetCurrentShader()->GetEffect();
	if (!pEffect)
		return false;

	DISPDBG(3, "Setting Technique");

	m_hTechnique = pEffect->GetTechniqueByName(Technique);

	if(m_hTechnique == 0)
		return false;

	if (FAILED(pEffect->SetTechnique(m_hTechnique)))
	{
		DISPDBG(3, "FAILED SetTechnique" );
		return false;
	}

	if (FAILED(pEffect->ValidateTechnique(m_hTechnique)))
	{
		DISPDBG(3, "FAILED Validate");
		return false;
	}

	SetDefaultMaxState();

    return true;
}

LPD3DXEFFECT MaxVertexShader::LoadEffect()
{
	NVPROF_FUNC("MaxVertexShader::LoadEffect");

	LPD3DXEFFECT pEffect = NULL;
	LPD3DXBUFFER pszErrors = NULL;
	
	ShaderInfoData* pShader = m_pMaterial->GetShaderInfo()->GetCurrentShader();
	const char* pszPath = pShader->GetEffectFile().c_str();

	if (!m_pDevice || !pszPath || strlen(pszPath) == 0)
		return false;

	DISPDBG(3, "Loading Effect: " << pszPath);

	pEffect = CEffectManager::GetSingletonPtr()->FindEffect(pszPath);
	// TIM: Re-enabled this because it's vital that we have caching for speed and resource reasons
	// FIXME: Probable memory leak, but needed to avoid shader caching
	if (pEffect){
		// TIM: Recreate always, just in case
		//CEffectManager::GetSingletonPtr()->
		return pEffect;
	 }
	DISPDBG(3, "Creating D3D Effect");

reload:
	if (FAILED(D3DXCreateEffectFromFile(GetDevice(),pszPath,0,0,D3DXSHADER_PARTIALPRECISION,0, &pEffect,&pszErrors)))
	{
		DISPDBG(3, "Failure");
		DISPDBG(0, "FAILED CreateEffect: " << pszPath);

        std::ostringstream strStream;
		strStream << "There were errors compiling. Please fix these now, then click OK to recompile." << endl;

        if (pszErrors)
        {
		    DISPDBG(0, "ERROR STRING: " << (char*)pszErrors->GetBufferPointer());
		    strStream << pszPath << " : " << (char*)pszErrors->GetBufferPointer();
        }
        else
        {
            strStream << "Couldn't load effect: " << (char*)pszErrors->GetBufferPointer();
        }

		SAFE_RELEASE(pszErrors);
 
		if(MessageBox(NULL, strStream.str().c_str(), "Compiling Effect Failed!", MB_ICONEXCLAMATION | MB_OKCANCEL) != IDCANCEL ){
			goto reload;
		}
	}
	

	CEffectManager::GetSingletonPtr()->AddEffect(pszPath, pEffect);

	// Get back to default max state
	SetDefaultMaxState();

	return pEffect;
}

LPDIRECT3DTEXTURE9 MaxVertexShader::LoadDDSTexture2D(const char* pszFilePath)
{
	NVPROF_FUNC("MaxVertexShader::LoadDDSTexture2D");
    CDDSImage image;
	HRESULT hr;
	 
	LPDIRECT3DTEXTURE9 pTexture = NULL;

	D3DFORMAT format = D3DFMT_DXT5;

	// Create the texture with requested parameters
	if((hr=D3DXCreateTextureFromFileEx( m_pDevice, pszFilePath, 
		D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT , 0, format , 
		D3DPOOL_MANAGED, D3DX_DEFAULT,
		D3DX_DEFAULT, 0, NULL, NULL, &pTexture ))!=S_OK)
	{
		char buf[256];
		sprintf(buf,"Couldn't load texture: %s (D3DErr: %s)", pszFilePath,DXGetErrorString9(hr));
		MessageBox(0,buf,"Texture load failed",MB_ICONSTOP);
		return NULL;
	}

	/*if (!image.load(pszFilePath, true))
		return NULL;

	if (image.get_num_images() > 1)
		return NULL;

	CTexture& Tex = image[0];

	LPDIRECT3DTEXTURE9 pTexture = NULL;

	// Create a matching d3d texture

    hr = D3DXCreateTexture(m_pDevice, Tex.get_width(), Tex.get_height(), Tex.get_num_mipmaps(), 0, (D3DFORMAT)image.get_d3dformat(), D3DPOOL_MANAGED, &pTexture);
	if (FAILED(hr))
		return NULL;
	
	if (image.is_compressed())
	{
		PitchSrc = image.size_dxtc(image[0].get_width(), 1);
	}
	else
	{
		PitchSrc = image.size_rgb(image[0].get_width(), 1);
	}

	LPDIRECT3DSURFACE9 pSurf;
	pTexture->GetSurfaceLevel(0, &pSurf);
	rcSrc.left = rcSrc.top = 0;
	rcSrc.right = image[0].get_width();
	rcSrc.bottom = image[0].get_height();
	hr = D3DXLoadSurfaceFromMemory(pSurf, NULL, NULL, (char*)image[0], (D3DFORMAT)image.get_d3dformat(), PitchSrc, NULL, &rcSrc, D3DX_FILTER_NONE, 0);
	SAFE_RELEASE(pSurf);

	PitchSrc   = max( image.size_rgb(1, 1), PitchSrc >> 1 );
	
	// load all mipmaps
	for (unsigned int i = 1; i < Tex.get_num_mipmaps() ; i++)
	{
		CSurface& srcSurf = Tex.get_mipmap(i - 1);

		pTexture->GetSurfaceLevel(i, &pSurf);
		rcSrc.left = rcSrc.top = 0;
		rcSrc.right = srcSurf.get_width();
		rcSrc.bottom = srcSurf.get_height();
		hr = D3DXLoadSurfaceFromMemory(pSurf, NULL, NULL, (char*)srcSurf, (D3DFORMAT)image.get_d3dformat(), PitchSrc, NULL, &rcSrc, D3DX_FILTER_NONE, 0);
		SAFE_RELEASE(pSurf);

		PitchSrc = max( image.size_rgb(1, 1), PitchSrc >> 1 );
	}
    
	if (Tex.get_num_mipmaps() == 0)
	{
		D3DXFilterTexture(pTexture, NULL, 0, D3DX_DEFAULT);
	}*/

    return pTexture;
}

LPDIRECT3DCUBETEXTURE9 MaxVertexShader::LoadDDSTextureCube(const char* pszFilePath)
{
	NVPROF_FUNC("MaxVertexShader::LoadDDSTextureCube");
    CDDSImage image;
	HRESULT hr;
	RECT rcSrc;
	unsigned long PitchSrc;
	 
		LPDIRECT3DCUBETEXTURE9 pTexture = NULL;
	// Create the texture with requested parameters
	if((hr=D3DXCreateCubeTextureFromFileEx( m_pDevice, pszFilePath, 
	D3DX_DEFAULT, D3DX_DEFAULT , 0, D3DFMT_UNKNOWN , 
		D3DPOOL_MANAGED, D3DX_DEFAULT,
		D3DX_DEFAULT, 0, NULL, NULL, &pTexture ))!=S_OK)
	{
		char buf[256];
		sprintf(buf,"Couldn't load texture: %s (D3DErr: %s)", pszFilePath,DXGetErrorString9(hr));
		MessageBox(0,buf,"Texture load failed",MB_ICONSTOP);
		return NULL;
	}
	return pTexture;

	/*if (!image.load(pszFilePath, true))
		return NULL;

	// Check for right number of images.
	if (image.get_num_images() != 6)
		return NULL;

	LPDIRECT3DCUBETEXTURE9 pCubeTexture = NULL;

	// Create a matching d3d texture

	CTexture& Tex = image[0];

    hr = D3DXCreateCubeTexture(m_pDevice, Tex.get_width(), Tex.get_num_mipmaps(), 0, (D3DFORMAT)image.get_d3dformat(), D3DPOOL_MANAGED, &pCubeTexture);
	if (FAILED(hr))
		return NULL;

	for (int face = 0; face < 6; face++)
	{
		CTexture& Tex = image[face];

		if (image.is_compressed())
		{
			PitchSrc = image.size_dxtc(image[face].get_width(), 1);
		}
		else
		{
			PitchSrc = image.size_rgb(image[face].get_width(), 1);
		}

		LPDIRECT3DSURFACE9 pSurf;
		pCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)face, 0, &pSurf);
		rcSrc.left = rcSrc.top = 0;
		rcSrc.right = image[face].get_width();
		rcSrc.bottom = image[face].get_height();
		hr = D3DXLoadSurfaceFromMemory(pSurf, NULL, NULL, (char*)image[face], (D3DFORMAT)image.get_d3dformat(), PitchSrc, NULL, &rcSrc, D3DX_FILTER_NONE, 0);
		SAFE_RELEASE(pSurf);

		PitchSrc >>= 1;
		
		// load all mipmaps
		for (unsigned int i = 1; i < Tex.get_num_mipmaps(); i++)
		{
			CSurface& srcSurf = Tex.get_mipmap(i - 1);

			pCubeTexture->GetCubeMapSurface((D3DCUBEMAP_FACES)face, i, &pSurf);
			rcSrc.left = rcSrc.top = 0;
			rcSrc.right = srcSurf.get_width();
			rcSrc.bottom = srcSurf.get_height();
			hr = D3DXLoadSurfaceFromMemory(pSurf, NULL, NULL, (char*)srcSurf, (D3DFORMAT)image.get_d3dformat(), PitchSrc, NULL, &rcSrc, D3DX_FILTER_NONE, 0);
			SAFE_RELEASE(pSurf);

			PitchSrc >>= 1;
		}
	}

   	if (Tex.get_num_mipmaps() == 0)
	{
		D3DXFilterCubeTexture(pCubeTexture, NULL, 0, D3DX_DEFAULT);
	}

    return pCubeTexture;*/
}

LPDIRECT3DVOLUMETEXTURE9 MaxVertexShader::LoadDDSTexture3D(const char* pszFilePath)
{
	NVPROF_FUNC("MaxVertexShader::LoadDDSTexture3D");
    CDDSImage image;
	HRESULT hr;
//	D3DBOX boxSrc;
//	unsigned long PitchSrc;
//	unsigned long PitchDepth;
	 

	LPDIRECT3DVOLUMETEXTURE9 pTexture = NULL;
	// Create the texture with requested parameters
	if((hr=D3DXCreateVolumeTextureFromFileEx( m_pDevice, pszFilePath, 
		D3DX_DEFAULT, D3DX_DEFAULT,D3DX_DEFAULT, D3DX_DEFAULT , 0, D3DFMT_UNKNOWN , 
		D3DPOOL_MANAGED, D3DX_DEFAULT,
		D3DX_DEFAULT, 0, NULL, NULL, &pTexture ))!=S_OK)
	{
		char buf[256];
		sprintf(buf,"Couldn't load texture: %s (D3DErr: %s)", pszFilePath,DXGetErrorString9(hr));
		MessageBox(0,buf,"Texture load failed",MB_ICONSTOP);
		return NULL;
	}
	return pTexture;

	/*if (!image.load(pszFilePath, true))
		return NULL;

	CTexture& Tex = image[0];

	LPDIRECT3DVOLUMETEXTURE9 pVolumeTexture = NULL;

	// Create a matching d3d texture

    hr = D3DXCreateVolumeTexture(m_pDevice, Tex.get_width(), Tex.get_height(), Tex.get_depth(), Tex.get_num_mipmaps(), 0, (D3DFORMAT)image.get_d3dformat(), D3DPOOL_MANAGED, &pVolumeTexture);
	if (FAILED(hr))
		return NULL;
	
	if (image.is_compressed())
	{
		PitchSrc = image.size_dxtc(image[0].get_width(), 1);
		PitchDepth = image.size_dxtc(image[0].get_width(), image[0].get_height());
	}
	else
	{
		PitchSrc = image.size_rgb(image[0].get_width(), 1);
		PitchDepth = image.size_rgb(image[0].get_width(), image[0].get_height());
	}


	LPDIRECT3DVOLUME9 pVolume;
	pVolumeTexture->GetVolumeLevel(0, &pVolume);
	boxSrc.Left = boxSrc.Top = boxSrc.Front = 0;
	boxSrc.Right = image[0].get_width();
	boxSrc.Back = image[0].get_depth();
	boxSrc.Bottom = image[0].get_height();

	hr = D3DXLoadVolumeFromMemory(pVolume, NULL, NULL, (char*)image[0], (D3DFORMAT)image.get_d3dformat(), PitchSrc, PitchDepth, NULL, &boxSrc, D3DX_FILTER_NONE, 0);
	SAFE_RELEASE(pVolume);

	PitchSrc   = max( image.size_rgb(1, 1), PitchSrc >> 1 );
	PitchDepth = max( image.size_rgb(1, 1), PitchDepth >> 1 );
	
	// load all mipmaps
	for (unsigned int i = 1; i < Tex.get_num_mipmaps(); i++)
	{
		CSurface& srcSurf = Tex.get_mipmap(i - 1);

		pVolumeTexture->GetVolumeLevel(i, &pVolume);
		boxSrc.Left = boxSrc.Top = boxSrc.Front = 0;
		boxSrc.Right = srcSurf.get_width();
		boxSrc.Back = srcSurf.get_depth();
		boxSrc.Bottom = srcSurf.get_height();
		hr = D3DXLoadVolumeFromMemory(pVolume, NULL, NULL, (char*)srcSurf, (D3DFORMAT)image.get_d3dformat(), PitchSrc, PitchDepth, NULL, &boxSrc, D3DX_FILTER_NONE, 0);
		SAFE_RELEASE(pVolume);

		PitchSrc   = max( image.size_rgb(1, 1), PitchSrc >> 1 );
		PitchDepth = max( image.size_rgb(1, 1), PitchDepth >> 1 );
	}

   	if (Tex.get_num_mipmaps() == 0)
	{
		D3DXFilterVolumeTexture(pVolumeTexture, NULL, 0, D3DX_DEFAULT);
	}

    return pVolumeTexture;*/
}



