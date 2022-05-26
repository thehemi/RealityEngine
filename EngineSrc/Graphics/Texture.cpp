//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
// Name: Texture.cpp
// Desc: Texture classes and texture management
//
// NOTE?: LOOK FOR DDS OUTSIDE THE CACHE SO WE CAN CHANGE FORMAT
//=============================================================================
#include "stdafx.h"
#include "TextureManager.h"
#include "Profiler.h"

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
Texture::Texture()
{
    m_bAllowExternalPath = false;
    dontCache   = false;
	m_OldTime	= 0;
	loaded		= false;
	width		= 0;
	height		= 0;
	depth		= 0;
	uOff		= 0;
	vOff		= 0;
	uTile		= 1;
	vTile		= 1;
	vAng		= 0;
	CurrentFrame = -1;
	filename = _U("UNLOADED");
	isAnimated = false;
	usesLOD = true;
}

//-----------------------------------------------------------------------------
// Specify effect file var this texture should bind to
//-----------------------------------------------------------------------------
/*void Texture::SetShaderVars(string texture, string textureTransform){
	m_hTexture.name			= texture;
	m_hTextureTransform.name = textureTransform;
}*/

//-----------------------------------------------------------------------------
// Return underlying texture resource
//-----------------------------------------------------------------------------
LPDIRECT3DTEXTURE9 Texture::GetTexture()
{
	if(CurrentFrame == -1 || !texIDs.size())
    {
		SeriousWarning(_U("WARNING!!: attempt to use unloaded texture: %s\nThis is unacceptable, as it will result in undefined behaviour"),filename.c_str());
		return 0;
	}
	else
    {
		return (LPDIRECT3DTEXTURE9)TextureManager::GetTex(texIDs[CurrentFrame]);
	}
}

//-----------------------------------------------------------------------------
// Name: Destroy
// Desc: Releases all links with management classes and cleans up resources
//-----------------------------------------------------------------------------
#include <crtdbg.h>
void Texture::Destroy(){
	if(filename.find(".avi")!=-1)
		filename=filename;
	ResourceManager::Instance()->Remove(this);
	if(Engine::Instance()->RenderSys == NULL) return;

	if(CurrentFrame !=-1){
		for(int i=0;i<texIDs.size();i++){
			TextureManager::Destroy(texIDs[i],this);
		}
		if(isAnimated)
			TextureManager::RemoveAnimatedTexture(this);
	}
	CurrentFrame = -1;
	texIDs.resize(0);
	loaded = false;
}


//-----------------------------------------------------------------------------
// Name: Set/UnSetTexture
// Desc: For rendering
//-----------------------------------------------------------------------------
void Texture::Set(int level){ 
	if(Engine::Instance()->RenderSys == NULL) return;

	if(CurrentFrame == -1){
		RenderWrap::ClearTextureLevel(level);
		return;
	}

	RenderWrap::dev->SetTexture(level,TextureManager::GetTex(texIDs[CurrentFrame]));
	//TextureManager::Set(level,texIDs[CurrentFrame],uTile,vTile,uOff,vOff);
}

void Texture::UnSet(int level){
	if(Engine::Instance()->RenderSys == NULL) return;

	RenderWrap::dev->SetTexture(level,0);
}


void Texture::CreateBlank(TextureType tType, D3DFMT d3dFormat, int x, int y, int z)
{
    if(tType == TT_AUTO)
        tType = TT_DIFFUSE;
	if(Engine::Instance()->RenderSys == NULL) return;

	LogPrintf(_U("Creating blank texture"));
	filename = _U("");
	format = d3dFormat;
	type = tType;
	width  = x;
	height = y;
	depth  = z;
	texIDs.push_back(TextureManager::Load(this));
	CurrentFrame = 0;
}

//-----------------------------------------------------------------------------
// Name: Load
// Desc: Takes texture name. Names with animated or in the animated folder will be so
//-----------------------------------------------------------------------------
bool Texture::Load(string sName, TextureType type, float uOff, float vOff, float uTile, float vTile, float vAng){
	if(sName.length() == 0 || sName == "NONE" || sName == "UNLOADED")
		return false;

    // For now, paths have no meaning, and just confuse us
    if(!m_bAllowExternalPath)
        sName = StripPath(sName);

	if(type == TT_AUTO){
		// NORMAL MAP
        string name = AsLower(sName);
		if(name.find("bump") != -1 || name.find("normal") != -1 || name.find("_nm") != -1 || name.find("_n.") != -1)
        {
			if(!Load(sName,TT_NORMALMAP)){
				// Load default if couldn't find tex
				Load("DefaultNormal.dds",TT_NORMALMAP);
				// Force filename to hold original name though, so re-saving doesn't lose info
				filename = sName;
                return false; // Didn't really succeed
			}
		}
		// DIFFUSE MAP or similar
		else{
			if(!Load(sName,TT_DIFFUSE)){
				// Load default if couldn't find tex
				Load("DefaultTexture.dds",TT_DIFFUSE);
				// Force filename to hold original name though, so re-saving doesn't lose info
				filename = sName;
                return false; // Didn't really succeed
			}
		}

		return true;
	}

	Destroy();

	filename = sName;
	loaded = true; // Ya ya, even if it's not loaded, we want to acknowledge that it was called

	ResourceManager::Instance()->Add(this);

	if(Engine::Instance()->RenderSys == NULL) 
		return true;

    StartMiniTimer();

	this->vAng = vAng;
	this->uOff = uOff;
	this->vOff = vOff;
	this->uTile = uTile;
	this->vTile = vTile;
	this->type = type;

	// Hacky -- should eventually replace the systems further down
	int id = TextureManager::CheckRuntimeCache(sName,this);
	if(id != -1){
		LogPrintf(LOG_HIGH,_U("Loading texture '%s' [from runtime cache]"),filename.c_str());
		SetID(id);

        Profiler::Get()->TextureLoadSecs += StopMiniTimer()/1000.f;
		return true;
	}

	// Search for file in original location, game dir, and dds cache
	// Our order of preference is:
	// 1) cache
	// 2) game dir
	// 3) original 3dsmax-saved location - NO MORE!!!
	string originalName = filename;
	//bool exists = FileExists(originalName);
	filename = StripPath(filename);
	bool existsInGameDir = FindMedia(filename,"Textures");
    if(m_bAllowExternalPath)
    {
        filename = originalName;
        existsInGameDir = existsInGameDir || FileExists(filename.c_str());
    }

	// HACKY: If it failed, try with different extensions. 
	// This makes it easy for us to sneak in DDS files to save memory or whatever
	// because it's illegal to put files in the dds cache (though you could, as another hack)
	if(!existsInGameDir)
    {
		string oldFile = filename;
		// Try DDS first
		filename = StripExtension(filename) + _U(".dds");
		existsInGameDir = FindMedia(filename,"Textures");

		// Then PNG
		if(!existsInGameDir){
			filename = StripExtension(filename) + _U(".png");
			existsInGameDir = FindMedia(filename,"Textures");
		}

        // Finally TGA
        if(!existsInGameDir){
			filename = StripExtension(filename) + _U(".tga");
			existsInGameDir = FindMedia(filename,"Textures");
		}

		// Give up, restore original extension
		if(!existsInGameDir){
			filename = oldFile;
		}
	}

	originalName = filename;
	bool existsInCache = TextureManager::CheckDDSCache(filename,this,existsInGameDir);

	// File doesn't exist anywhere...
	if(!existsInCache && !existsInGameDir){
		if(RenderWrap::dev && Engine::Instance()->MainConfig->GetBool("WarnOnMissingFiles"))
			Warning(_U("Couldn't find texture: %s"),filename.c_str());
		else
			LogPrintf(LOG_MEDIUM,_U("Couldn't find texture: %s"),filename.c_str());

		CurrentFrame = -1;
        Profiler::Get()->TextureLoadSecs += StopMiniTimer()/1000.f;
		return false;
	}

	SetID(TextureManager::Load(this));

	// Set proper names
	actualFile = filename;
	filename = sName;

    Profiler::Get()->TextureLoadSecs += StopMiniTimer()/1000.f;
	return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Texture::SetID(int id)
{
    string tmp = filename;
	ToLowerCase(tmp);
	// No animations
	if(tmp.find(_U("animated"))==-1 && tmp.find(_U("animation"))==-1) {
		texIDs.push_back(id); 
		CurrentFrame = 0;
	}
	// Animations
	else { 
        // FIXME: Add back
		//if(existsInCache)
		//	filename = originalName; // Don't want to use cache name to look for other frames!

		// Now the animation frames, if any
		string sName = filename.substr(0,filename.find(_U("1.")));
		string ext = filename.substr(filename.find_last_of(_U(".")),filename.length());
		int i;
		for(i=1;;i++){
			CHAR name[1024];
			sprintf(name,_U("%s%i%s"),sName.c_str(),i+1,ext.c_str());

			filename = name;
			if(FileExists(name))
            {
                string filename2 = filename;
                int id = TextureManager::CheckRuntimeCache(filename2,this);
                if(id == -1)
                    id = TextureManager::Load(this);
				texIDs.push_back(id);
            }
			else
				break;
		}
		LogPrintf(LOG_MEDIUM,_U("Animation frames found for texture. %d frames loaded."),i);

		isRandomized = tmp.find("random") != -1;
		isAnimated = true;
		TextureManager::AddAnimatedTexture(this);
		CurrentFrame = rand()%texIDs.size();
	}
}


void Texture::FillChannel(Channel chan, string strPath){
	if(!IsValid())
		Warning(_U("Warning, FillChannel for %s with %s failed (texture not loaded)"),filename.c_str(),strPath.c_str());
	LPDIRECT3DTEXTURE9 ptex = (LPDIRECT3DTEXTURE9)TextureManager::GetTex(texIDs[CurrentFrame]);
	TextureManager::FillChannel(chan,strPath,ptex);
}




