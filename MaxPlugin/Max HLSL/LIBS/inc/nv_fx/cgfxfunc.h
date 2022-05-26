#ifndef __CGFXFUNC_H_
#define __CGFXFUNC_H_

#include "nv_gui\invdebugconsole.h"

#define MAKE_VER_MS(a, b) ( ((a << 16) & 0xFFFF0000) | ((b) & 0xFFFF) )
#define MAKE_VER_LS(c, d) ( ((c << 16) & 0xFFFF0000) | ((d) & 0xFFFF) )

static const DWORD CG_MS = MAKE_VER_MS(1, 1);
static const DWORD CG_LS = MAKE_VER_LS(307, 700);

static const DWORD CGFX_MS = MAKE_VER_MS(0, 0);
static const DWORD CGFX_LS = MAKE_VER_LS(0, 4);


class CModuleCheck
{
public:
	CModuleCheck(const char* pszName, DWORD MS, DWORD LS)
		: m_strModuleName(pszName),
		m_MS(MS),
		m_LS(LS)
	{

	}

	bool SetCheckMatch(DWORD dwMS, DWORD dwLS)
	{
		m_MS_Actual = dwMS;
		m_LS_Actual = dwLS;

		if (dwMS != m_MS)
		{
			return false;
		}

		if (dwLS != m_LS)
		{
			return false;
		}

		return true;
	}

	static std::string GetString(DWORD dwMS, DWORD dwLS)
	{
		std::ostringstream strStream;
		strStream << ((dwMS >> 16) & 0xFFFF) << "." << (dwMS & 0xFFFF) << "." << ((dwLS >> 16) & 0xFFFF) << "." << (dwLS & 0xFFFF);
		return strStream.str();
	}

	std::string m_strModuleName;
	DWORD m_MS;
	DWORD m_LS;
	DWORD m_MS_Actual;
	DWORD m_LS_Actual;
};

typedef std::vector<CModuleCheck> tvecModules;

typedef HRESULT  (__cdecl* pfnCgFXCreateEffect)(
        LPCSTR               pSrcData,
        DWORD                Flags,
        ICgFXEffect**        ppEffect,
        const char**         ppCompilationErrors);

typedef HRESULT (__cdecl* pfnCgFXCreateEffectFromFileA)(
        LPCSTR               pSrcFile,
        DWORD                Flags,
        ICgFXEffect**        ppEffect,
        const char**         ppCompilationErrors);

typedef HRESULT (__cdecl* pfnCgFXCreateEffectCompiler)(
        LPCSTR                pSrcData,
        DWORD                 Flags,
        ICgFXEffectCompiler** ppEffectCompiler,
        const char**          ppCompilationErrors);

typedef HRESULT (__cdecl* pfnCgFXCreateEffectCompilerFromFileA)(
        LPCSTR                pSrcFile,
        DWORD                 Flags,
        ICgFXEffectCompiler** ppEffectCompiler,
        const char**          ppCompilationErrors);

typedef HRESULT (__cdecl* pfnCgFXSetDevice)(const char* pDeviceName,LPVOID pDevice);

typedef HRESULT (__cdecl* pfnCgFXFreeDevice)(const char* pDeviceName, LPVOID pDevice);

typedef HRESULT (__cdecl* pfnCgFXGetErrors)(const char** ppErrors);

typedef HRESULT (__cdecl* pfnCgFXRelease)();

class CgFXFuncs : public Singleton<CgFXFuncs>
{
public:
	CgFXFuncs();
	~CgFXFuncs();
	void Reset();
	HRESULT CgFXCreateEffect(
		LPCSTR               pSrcData,
        DWORD                Flags,
        ICgFXEffect**        ppEffect,
        const char**         ppCompilationErrors);
	HRESULT CgFXCreateEffectFromFileA(
        LPCSTR               pSrcFile,
        DWORD                Flags,
        ICgFXEffect**        ppEffect,
        const char**         ppCompilationErrors);
	HRESULT CgFXCreateEffectCompiler(
        LPCSTR                pSrcData,
        DWORD                 Flags,
        ICgFXEffectCompiler** ppEffectCompiler,
        const char**          ppCompilationErrors);
	HRESULT CgFXCreateEffectCompilerFromFileA(
        LPCSTR                pSrcFile,
        DWORD                 Flags,
        ICgFXEffectCompiler** ppEffectCompiler,
        const char**          ppCompilationErrors);
	HRESULT CgFXSetDevice(const char* pDeviceName,LPVOID pDevice);
	HRESULT CgFXFreeDevice(const char* pDeviceName, LPVOID pDevice);
	HRESULT CgFXGetErrors(const char** ppErrors);
	HRESULT CgFXRelease();
	
	bool InfoDump(nv_gui::INVDebugConsole* pConsole);
private:
	tvecModules m_Modules;

	HINSTANCE m_hLib;
	pfnCgFXCreateEffect m_pCreateEffect;
	pfnCgFXCreateEffectFromFileA m_pCreateEffectFromFileA;
	pfnCgFXCreateEffectCompiler m_pCreateEffectCompiler;
	pfnCgFXCreateEffectCompilerFromFileA m_pCreateEffectCompilerFromFileA;
	pfnCgFXSetDevice m_pSetDevice;
	pfnCgFXFreeDevice m_pFreeDevice;
	pfnCgFXGetErrors m_pGetErrors;
	pfnCgFXRelease m_pRelease;
};

#endif // __CGFXFUNC_H_


