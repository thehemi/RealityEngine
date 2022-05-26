//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
//	Texture classes and texture management
//=============================================================================
#ifndef TEXTURE_INCLUDED
#define TEXTURE_INCLUDED

struct ShaderVar;

enum TextureType {
	TT_AUTO,    /// Will figure out if diffuse/normal from filename
	TT_DIFFUSE,
	TT_NORMALMAP,
	/// Stick this in the alpha of its corresponding shader texture
	/// This is a special flag, and does not mean the texture has alpha
	TT_PUTINALPHA,
	/// Cube & Volume are only used for creation of new surfaces
	TT_CUBE,	
	TT_VOLUME,
};

enum Channel {
	CHAN_ALPHA,
	CHAN_RED,
	CHAN_GREEN,
	CHAN_BLUE
};

#define D3DFMT int /// Really enum

//--------------------------------------------------------------------------------
///  Game-level texture objects. Device-level management is automatic.
//--------------------------------------------------------------------------------
class ENGINE_API Texture{
private:
	friend class TextureManager;
	friend class SceneLoader;
	/// This may be an animation strip
	vector<int> texIDs;

	float	m_OldTime;
	/// Randomized animation
	bool	isRandomized; 
	bool	isAnimated;
	bool	loaded;
	/// If animated strip. 0 If single texture. -1 if not valid
	int		CurrentFrame; 
	/// The actual file loaded (cached, default, etc). Only used for debugging
	string  actualFile; 
    /// Does final loading based on id
    void    SetID(int id);

public:
    /// Set this if the texture can be loaded outside of Textures folder. File path must be exactly correct
    bool m_bAllowExternalPath;
    /// Set this if you don't want a texture uploaded to video memory
    bool dontCache; 

	/// Whether the Texture scales its size according to TextureSizePercent. 
	/// Certain textures like those used by shadow projectors or HUDs & GUIs may wish to have a static size.
	bool usesLOD;

	/// Texture transform info
	float		uOff, vOff, uTile, vTile, vAng;

	/// Attributes filled in on created textures
	/// depth is for volume textures, not colour depth
	int			width, height, depth; 
	D3DFMT		format;
	TextureType type;
	/// Will be blank if texture wasn't loaded from file
	string		filename; 

	/// Returns true only if texture is valid&loaded NOW
	bool IsValid(){ return CurrentFrame != -1; } 
	/// Returns true even if load failed
	bool IsLoaded(){ return loaded; } 
	/// Animation frames are loaded if they exist and texture name/path contains animated
	bool Load(string name, TextureType type = TT_AUTO, float uOff=0, float vOff=0, float uTile=1, float vTile=1, float vAng=0);
	void CreateBlank(TextureType tType, D3DFMT d3dFormat, int x, int y, int z = 0);
	/// Fills a single channel of the texture with another texture
	/// The other texture can be color, in which case the greyscale data is taken from the R channel
	void FillChannel(Channel chan, string textureName);
	bool IsAnimated(){ return isAnimated; }
	struct IDirect3DTexture9* GetTexture();

	Texture();

	void Set(int level);
	void UnSet(int level);
	
	/// For SFX that need to set texture states etc themselves, just sets raw texture
	void Set_Raw(int level);

	/// Tell the renderer to stop worrying about caching this texture
	void Destroy();
	~Texture() { 
		Destroy(); 
	}
};


#endif
