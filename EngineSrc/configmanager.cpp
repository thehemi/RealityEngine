//=========== (C) Copyright 2004, Artficial Studios. All rights reserved. ============
// File: ConfigManager.cpp
//
// Implementation of configuration system functions.
//
//====================================================================================
#include "stdafx.h"
#include <stdio.h>
#include "ConfigDatabase.h"
#include "ConfigManager.h"
#include <ddraw.h>
#include <dsetup.h>
#include "dxstdafx.h"
#include <dsound.h>




static DWORD GetNumber( const char* value )
{
    int result;
    int nRet = sscanf( value, "%d", &result );
    if( nRet == 0 || nRet == EOF )
        result = 0;
    return result;
}


static float GetFloatNumber( const char* value )
{
    float result;
    int nRet = sscanf( value, "%f", &result );
    if( nRet == 0 || nRet == EOF )
        result = 0.0f;
    return result;
}


BOOL WINAPI DDEnumCB( GUID FAR *lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext, HMONITOR hm )
{
    CConfigManager *pCM = (CConfigManager*)lpContext;

    if( pCM->m_nAdapterCount )
    {
        memcpy( &pCM->m_AdapterArray[pCM->m_nAdapterCount].guid, lpGUID, sizeof(GUID) );
    }
    if( strlen( lpDriverName )<31 )
        strcpy( pCM->m_AdapterArray[pCM->m_nAdapterCount].DriverName, lpDriverName );
    else
        pCM->m_AdapterArray[pCM->m_nAdapterCount].DriverName[0] = 0;

    pCM->m_nAdapterCount++;
    if( pCM->m_nAdapterCount == 10 )
        return false;
    else
        return true;
}


CConfigManager::CConfigManager()
{
    InitConfigProperties();
    ZeroMemory( m_AdapterArray, sizeof(m_AdapterArray) );
    m_nAdapterCount = 0;
}


void CConfigManager::InitConfigProperties()
{
    cf_SafeMode = false;
}


void CConfigManager::InitConfigInformation( D3DCAPS9 &d3dCaps )
{
    //
    // Get system memory size
    //
    MEMORYSTATUS ms;
    GlobalMemoryStatus( &ms );
    SysMemory = (DWORD)(((ms.dwTotalPhys+(16*1024*1024-1)) & ~(16*1024*1024-1) ) >> 20);

    //
    // Get CPU speed
    //
#if defined( _WIN64 ) && defined( _IA64_ )
    CpuSpeed = ULONG_MAX;  // On Ia64, skip processor speed detection and just return a large value
                           // so requirement would pass.
#else
    __int64 time,qpc,compare;

    HANDLE hThread = GetCurrentThread();
    HANDLE hProcess = GetCurrentProcess();
    long OldPriority = GetThreadPriority(hThread);
    DWORD OldClass = GetPriorityClass(hProcess);
    SetPriorityClass( hProcess,REALTIME_PRIORITY_CLASS );
    SetThreadPriority( hThread,THREAD_PRIORITY_TIME_CRITICAL ); // Run code at realtime priority
    Sleep(100);                                                 // Give windows a chance to run background tasks

    QueryPerformanceFrequency((LARGE_INTEGER*)&qpc);
    QueryPerformanceCounter((LARGE_INTEGER*)&compare);
    compare+=qpc/4;                                             // Find time .5 seconds ahead

#ifdef _WIN64
    time = __rdtsc();  // Use intrinsic for AMD64 environment
#else
    __asm rdtsc
    __asm mov dword ptr time,eax
    __asm mov dword ptr time+4,edx
#endif

    do
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&qpc);          // Spin until time equal
    }
    while( compare>qpc );

#ifdef _WIN64
    time = __rdtsc() - time;  // Use intrinsic for AMD64 environment
#else
    __asm rdtsc
    __asm sub eax,dword ptr time
    __asm sbb edx,dword ptr time+4
    __asm mov dword ptr time,eax
    __asm mov dword ptr time+4,edx
#endif

    SetThreadPriority(hThread,OldPriority);
    SetPriorityClass(hProcess,OldClass);

    //
    // Normalize CPU speed
    //
    CpuSpeed=(DWORD)(time/250000);

    if( CpuSpeed>1000 )
    {
        CpuSpeed=((CpuSpeed+50)/100)*100;
    }

    DWORD CpuSpeed2=CpuSpeed%100;

    if( CpuSpeed<200 )
    {
        if( CpuSpeed2>95 )
        {
            CpuSpeed+=100-CpuSpeed2;
        }
        if( CpuSpeed2>61 && CpuSpeed2<71 )
            CpuSpeed+=66-CpuSpeed2;
        if( CpuSpeed2>45 && CpuSpeed2<55 )
            CpuSpeed+=50-CpuSpeed2;
        if( CpuSpeed2>28 && CpuSpeed2<38 )
            CpuSpeed+=33-CpuSpeed2;
    }
    else
    {
        if( CpuSpeed2>84 )
        {
            CpuSpeed+=100-CpuSpeed2;
        }
        else
        {
            if( CpuSpeed2>58 )
            {
                CpuSpeed+=66-CpuSpeed2;
            }
            else
            {
                if( CpuSpeed2>42 )
                {
                    CpuSpeed+=50-CpuSpeed2;
                }
                else
                {
                    if( CpuSpeed2>16 )
                    {
                        CpuSpeed+=33-CpuSpeed2;
                    }
                    else
                    {
                        if( CpuSpeed2<16 )
                        {
                            CpuSpeed-=CpuSpeed2;
                        }
                    }
                }
            }
        }
    }
#endif   // _WIN64 && _IA64_

    //
    // Get video memory size for all devices using ddraw 7 interfaces
    //
    WCHAR wszPath[MAX_PATH];
    ::GetSystemDirectoryW( wszPath, MAX_PATH );
    lstrcatW( wszPath, L"\\ddraw.dll" );
    IDirectDraw7* DDobject;
    HINSTANCE ddrawLib = LoadLibraryW( wszPath );
    if( ddrawLib )
    {
        HRESULT (WINAPI* _DirectDrawCreateEx)( GUID* lpGUID, void** lplpDD, REFIID iid, IUnknown* pUnkOuter ) = (HRESULT (WINAPI*)( GUID* lpGUID, void** lplpDD, REFIID iid, IUnknown* pUnkOuter ))GetProcAddress( ddrawLib, "DirectDrawCreateEx" ); 
        if( _DirectDrawCreateEx )
        {
            HRESULT (WINAPI* _DirectDrawEnumerateEx)( LPDDENUMCALLBACKEXA, LPVOID, DWORD ) = (HRESULT (WINAPI*)( LPDDENUMCALLBACKEXA, LPVOID, DWORD )) GetProcAddress( ddrawLib, "DirectDrawEnumerateExA" ); 
            if( _DirectDrawEnumerateEx )
            {
                //
                // DirectDraw 7 will enumerate 1 device (NULL). Or a NULL device and then a GUID for each adapter.
                //
                _DirectDrawEnumerateEx( DDEnumCB, this, DDENUM_ATTACHEDSECONDARYDEVICES );

                //
                // Create a DirectDraw object for the device specified
                //
                for( int t0 = 0; t0 < m_nAdapterCount; ++t0 )
                {
                    HRESULT result = _DirectDrawCreateEx( t0 ? &m_AdapterArray[t0].guid : NULL, (void**)&DDobject, IID_IDirectDraw7, NULL );
                    if( SUCCEEDED(result) )
                    {
                        DDobject->SetCooperativeLevel( NULL, DDSCL_NORMAL );
                        //
                        // See how much memory is on the card
                        //
                        DDSCAPS2 ddscaps2;
                        DWORD Junk;
                        DWORD VideoMemory=-1;
                        DWORD Temp=0;
                        memset( &ddscaps2, 0, sizeof(ddscaps2) );
                        ddscaps2.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
                        DDobject->GetAvailableVidMem( &ddscaps2, &Temp, &Junk );
                        if( Temp>0 && Temp<VideoMemory )
                            VideoMemory=Temp;
                        ddscaps2.dwCaps = DDSCAPS_3DDEVICE | DDSCAPS_LOCALVIDMEM | DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
                        DDobject->GetAvailableVidMem( &ddscaps2, &Temp, &Junk );
                        if( Temp>0 && Temp<VideoMemory )
                            VideoMemory=Temp;
                        ddscaps2.dwCaps = DDSCAPS_LOCALVIDMEM | DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
                        DDobject->GetAvailableVidMem( &ddscaps2, &Temp, &Junk );
                        if( Temp>0 && Temp<VideoMemory )
                            VideoMemory=Temp;
                        ddscaps2.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY;
                        DDobject->GetAvailableVidMem( &ddscaps2, &Temp, &Junk );
                        if( Temp>0 && Temp<VideoMemory )
                            VideoMemory=Temp;
                        //
                        // If cannot get memory size, there must be a problem with D3D / DirectDraw
                        //
                        if( -1==VideoMemory )
                        {
                            VideoMemory = 0;
                        } else
                        {
                            //
                            // Round the video memory number depending upon the size
                            //
                            if( VideoMemory<=16*1024*1024 )
                            {
                                VideoMemory=((VideoMemory+8*1024*1024-1) & ~(8*1024*1024-1));               // Round up to nearest 8 under 20Meg
                            }
                            else
                            {
                                if( VideoMemory <= 64*1024*1024 )
                                {
                                    VideoMemory=((VideoMemory+32*1024*1024-1) & ~(32*1024*1024-1));         // Round to neaest 32 Meg under 64Meg
                                }
                                else
                                {
                                    VideoMemory=((VideoMemory+64*1024*1024-1) & ~(64*1024*1024-1));         // Round to neaest 64 Meg over 64Meg
                                }
                            }
                        }

                        //
                        // Clean up ddraw7
                        //
                        m_AdapterArray[t0].MemorySize = VideoMemory;
                        DDobject->Release();
                    }
                }
            }
        }
        //
        // Release ddraw7 object
        //
        FreeLibrary( ddrawLib );
    }

    //
    // Find the display adapter (ddraw and d3d9 do not enumerate the same order)
    //
    D3DADAPTER_IDENTIFIER9 d3di;
    DXUTGetD3DObject()->GetAdapterIdentifier( d3dCaps.AdapterOrdinal, 0, &d3di );
    DriverVersionLowPart = d3di.DriverVersion.LowPart;
    DriverVersionHighPart = d3di.DriverVersion.HighPart;
    VideoMemory = m_AdapterArray[0].MemorySize;
    for( int i = 0; i < m_nAdapterCount; ++i )
    {
        if( !_stricmp( m_AdapterArray[i].DriverName, d3di.DeviceName ) )
            VideoMemory = m_AdapterArray[i].MemorySize;
    }

    //
    // Detect sound configuration
    //
    GUID Guid;
    GetDeviceID( &DSDEVID_DefaultPlayback, &Guid );
    InitSoundInformation( Guid, &Sound_device );
}


HRESULT CConfigManager::Initialize( WCHAR* FileName, D3DADAPTER_IDENTIFIER9 &AdapterID, D3DCAPS9 &Caps,
                                    REFGUID DSoundDeviceGuid )
{
    HRESULT hr = S_OK;

    InitConfigInformation( Caps );

    SOUND_DEVICE SndDev;
    InitSoundInformation( DSoundDeviceGuid, &SndDev );

    LogPrintf( "Media device found:" );
    DXUTTRACE( L"%s\n", SndDev.name );

    IConfigDatabase* pCDB = IConfigDatabase::Create();
    if( pCDB->Load( FileName, SndDev, AdapterID, Caps, 1024, 64, 1700 ) )
    {
        LogPrintf( "Graphics device string: %s", pCDB->GetGfxDeviceString() );
        LogPrintf( "Graphics vendor string: %s", pCDB->GetGfxVendorString() );
        LogPrintf( "Sound device string: %s", pCDB->GetSoundDeviceString() );
        LogPrintf( "Sound vendor string: %s", pCDB->GetSoundVendorString() );

        // Iterate through properties and set any config settings
        if(Engine::Instance()->MainConfig->GetBool("FirstRun"))
        {
            LogPrintf( "Properties:" );
            int nPropCount = pCDB->GetDevicePropertyCount();
            for( int i = 0; i < nPropCount; ++i )
            {
                if(Engine::Instance()->MainConfig->KeyExists(pCDB->GetDeviceProperty( i )))
                {
                    // Setting exists, so write to config
                    Engine::Instance()->MainConfig->SetString(pCDB->GetDeviceProperty( i ),pCDB->GetDeviceValue( i ));
                    LogPrintf( "\"%s\" : \"%s\"", pCDB->GetDeviceProperty( i ), pCDB->GetDeviceValue( i ) );
                }
            }
			Engine::Instance()->MainConfig->SetBool("FirstRun",false);
        }

        ::LogPrintf( "\nRequirements:" );
        int nReqCount = pCDB->GetRequirementsPropertyCount();
        for( int i = 0; i < nReqCount; ++i )
        {
            char* Property = (char*)pCDB->GetRequirementsProperty( i );
            char* Value = (char*)pCDB->GetRequirementsValue( i );
            int nRet;

            LogPrintf( "\"%s\" : \"%s\"", Property, Value );

            if( !_stricmp( "CpuSpeed", Property ) )
            {
                nRet = sscanf( Value, "%d",&req_CPUSpeed );
                if( nRet == 0 || nRet == EOF )
                    req_CPUSpeed = 0;
            } else
            if( !_stricmp( "Memory", Property ) )
            {
                nRet = sscanf( Value, "%d",&req_Memory );
                if( nRet == 0 || nRet == EOF )
                    req_Memory = 0;
            } else
            if( !_stricmp( "VideoMemory", Property ) )
            {
                nRet = sscanf( Value, "%d",&req_VideoMemory );
                if( nRet == 0 || nRet == EOF )
                    req_VideoMemory = 0;
            } else
            if( !_stricmp( "DirectX", Property ) )
            {
                nRet = sscanf( Value, "%d.%d%c", &req_DirectXMajor, &req_DirectXMinor, &req_DirectXLetter );
                req_DirectXLetter = (char)req_DirectXLetter;  // Convert to wide char
                if( nRet == 0 || nRet == EOF )
                {
                    req_DirectXMajor = req_DirectXMinor = 0;
                    req_DirectXLetter = L' ';
                }
            } else
            if( !_stricmp( "DiskSpace", Property ) )
            {
                nRet = sscanf( Value, "%d",&req_DiskSpace );
                if( nRet == 0 || nRet == EOF )
                    req_DiskSpace = 0;
            } else
            if( !_stricmp( "OS", Property ) )
            {
                if( !_stricmp( "Win95", Value ) )
                {
                    req_OSMajor = 4;
                    req_OSMinor = 0;
                } else
                if( !_stricmp( "Win98", Value ) )
                {
                    req_OSMajor = 4;
                    req_OSMinor = 10;
                } else
                if( !_stricmp( "WinME", Value ) )
                {
                    req_OSMajor = 4;
                    req_OSMinor = 90;
                } else
                if( !_stricmp( "Win2k", Value ) )
                {
                    req_OSMajor = 5;
                    req_OSMinor = 0;
                } else
                if( !_stricmp( "WinXP", Value ) )
                {
                    req_OSMajor = 5;
                    req_OSMinor = 1;
                } else
                if( !_stricmp( "Win2003", Value ) )
                {
                    req_OSMajor = 5;
                    req_OSMinor = 2;
                }
            }
        }
    }
    else
    {
        hr = E_FAIL;
    }

    pCDB->Release();

    return hr;
}




HRESULT CConfigManager::VerifyRequirements()
{
    HRESULT hr = S_OK;
    WCHAR wszText[2048] = L"This system does not meet the following minimum requirement(s):\n\n";

    //
    // OS version
    //
    OSVERSIONINFO ovi;
    ovi.dwOSVersionInfoSize = sizeof(ovi);
    GetVersionEx( &ovi );
    if( req_OSMajor > ovi.dwMajorVersion ||
        ( req_OSMajor == ovi.dwMajorVersion && req_OSMinor > ovi.dwMinorVersion ) )
    {
        // OS version not supported.
        hr = E_FAIL;
        wcsncat( wszText, L"- Operating system not supported\n", 2048 - lstrlenW( wszText ) );
        wszText[2047] = L'\0';
    }

    //
    // CPU speed
    //
    if( CpuSpeed < req_CPUSpeed )
    {
        hr = E_FAIL;
        WCHAR wsz[256];
        swprintf( wsz, L"- Minimum processor speed of %u MHz\n", req_CPUSpeed );
        wcsncat( wszText, wsz, 2048 - lstrlenW( wszText ) );
        wszText[2047] = L'\0';
    }

    //
    // Memory size
    //
    if( SysMemory < req_Memory )
    {
        hr = E_FAIL;
        WCHAR wsz[256];
        swprintf( wsz, L"- Minimum memory of %u megabytes\n", req_Memory );
        wcsncat( wszText, wsz, 2048 - lstrlenW( wszText ) );
        wszText[2047] = L'\0';
    }

    //
    // Video memory size
    //
    if( (VideoMemory >> 20) < req_VideoMemory )
    {
        hr = E_FAIL;
        WCHAR wsz[256];
        swprintf( wsz, L"- Minimum video memory of %u megabytes\n", req_VideoMemory );
        wcsncat( wszText, wsz, 2048 - lstrlenW( wszText ) );
        wszText[2047] = L'\0';
    }

    //
    // DirectX version
    //
    DWORD dwDXMajor = 0;
    DWORD dwDXMinor = 0;
    WCHAR cDXLetter = ' ';
    GetDXVersion( &dwDXMajor, &dwDXMinor, &cDXLetter );

    if( dwDXMajor < req_DirectXMajor ||
        ( dwDXMajor == req_DirectXMajor && dwDXMinor < req_DirectXMinor ) ||
        ( dwDXMajor == req_DirectXMajor && dwDXMinor == req_DirectXMinor && cDXLetter < req_DirectXLetter ) )
    {
        hr = E_FAIL;
        WCHAR wsz[256];
        swprintf( wsz, L"- DirectX %u.%u%c\n", req_DirectXMajor, req_DirectXMinor, req_DirectXLetter );
        wcsncat( wszText, wsz, 2048 - lstrlenW( wszText ) );
        wszText[2047] = L'\0';
    }

    if( FAILED( hr ) )
    {
        ::MessageBoxW( NULL, wszText, L"Minimum Requirement", MB_OK|MB_ICONERROR );
    }

    return hr;
}
