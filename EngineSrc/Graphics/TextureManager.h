//=========== (C) Copyright 2003, Tim Johnson. All rights reserved. ===========
/// Name: TextureManager
/// Desc: Manages all textures for the renderer
/// It keeps copies of all the names, so if the device is destroyed it reloads
/// them all from file. If the device is simply lost, then nothing need be done
/// as textures are kept in D3DPOOL_MANAGED
//=============================================================================


class TextureManager{
private:
	friend class RenderDevice;

	/// Settings. These are set through the rendering device
	static bool CompressNormalMaps;
	static int  TextureSizePercent;
	static string m_CachePath;

		
	/// D3d-specific info
	struct TexInfo{
		TexInfo(){for(int i=0;i<10;i++)otherOwners[i]=0; m_Movie = 0; tex = 0; texture = 0;}

		/// Hack so if one texture class is destroyed we have the others to use
		Texture* otherOwners[10]; 
		Texture* texture;

		/// Movie texture container
		class CTextureRenderer* m_Movie;

		LPDIRECT3DBASETEXTURE9 tex;
		bool hadMips;
		int ref; /// 0 means it has 1 owner
	};
	static vector<TexInfo> textures;


	static int FindTexture(string name, TextureType type);
	static void CreateSurface(TexInfo* tInfo);

	/// For animated textures
	static vector<class Texture*> AnimatedTextures;
public:

	static float FrameTime;
	static int maxTextures;   /// simultaneously that the device supports
	static int textureMemory; /// Approximate texture memory used, not including mips
	static bool SaveMips; /// When packing dds textures for distribution this is set to false
	static ConfigFile cache; /// Holds info about files in cache, so we can rebuild them when they become out-of-sync
	static int CheckRuntimeCache(string& name, Texture* texture);

	static bool CheckDDSCache(string& file, Texture* texture, bool originalExists);

	TextureManager(){ }
	static void Initialize(const D3DSURFACE_DESC* pBackBufferSurfaceDesc);


	/// Returns a texture ID
	static int Load(Texture* pTexture);
	static inline LPDIRECT3DBASETEXTURE9 GetTex(int id){ return textures[id].tex; }

	static void UpdateAllLODs();
	static void RestoreAll();
	static void OnLostDevice();
	static void FreeAll();
	static void Destroy(int texID, Texture* owner);
	static void Free(int texID);
	static void CacheAllTextures();

	/// For animated textures
	static void AddAnimatedTexture(Texture* tex);
	static void UpdateAnimatedTextures();
	static void RemoveAnimatedTexture(Texture* tex);

	/// Misc
	static bool FillChannel(Channel chan, string strPath, LPDIRECT3DTEXTURE9 ptex);
	static bool CreateNormalMap(TexInfo* tInfo, D3DSURFACE_DESC& desc);
	//static void Compress(LPDIRECT3DTEXTURE9& tex, TexInfo* tInfo, D3DSURFACE_DESC& desc);
	static void ChangeFormat(LPDIRECT3DTEXTURE9& tex, D3DFORMAT format, int mips=0);
	static void CacheTextureToDisk(string fileName, TexInfo* tInfo);
	static void CreateNewTexture(TexInfo* tInfo);
	static void LoadTexture(LPDIRECT3DTEXTURE9& tex, TexInfo* t, const CHAR* name, bool& cacheIt);

};

