/*********************************************************************NVMH4****
Path:  NVSDK\Common\include\nv_debug
File:  idebugconsole.h

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


Simple interface to the debug console.


******************************************************************************/

#ifndef __INVDEBUGCONSOLE_H
#define __INVDEBUGCONSOLE_H

namespace nv_gui
{

//! This interface is a simple dialog box which shows a list of report strings.
/*! Useful for displaying debug information in a seperate window
*/
class INVDebugConsole : public nv_sys::INVObject
{
public:

	static INVDebugConsole* CreateInstance();

	//! Add a string to the console
    virtual bool INTCALLTYPE OutputDebug(const char* pszChar) = 0;
	
	//! Change the visibility of the dialog
	virtual bool INTCALLTYPE SetVisible(bool bHide) = 0;

	//! Check the visibility of the dialog
	virtual bool INTCALLTYPE IsVisible() = 0;

	//! Set the log file to write debug strings to.
	virtual bool INTCALLTYPE SetLogFile(const char* pszLogFile) = 0; 

	//! Change the title
	virtual bool INTCALLTYPE SetTitle(const char* pszTitle) = 0;
};

//! An interface to allow an object to dump info to a debug console
class INVInfoDump : public nv_sys::INVObject
{
public:
	//! Dump info to a debug console
	virtual bool INTCALLTYPE InfoDump(INVDebugConsole* pConsole) = 0;
};

}; // namespace nv_gui

// {24473DF4-82C2-419d-AD09-BD8C7F36D379}
static const nv_sys::NVGUID IID_INVDebugConsole = 
{ 0x24473df4, 0x82c2, 0x419d, { 0xad, 0x9, 0xbd, 0x8c, 0x7f, 0x36, 0xd3, 0x79 } };

// {DAD0F9ED-FED7-42e6-AD12-D3F3241D7BE9}
static const nv_sys::NVGUID IID_INVInfoDump = 
{ 0xdad0f9ed, 0xfed7, 0x42e6, { 0xad, 0x12, 0xd3, 0xf3, 0x24, 0x1d, 0x7b, 0xe9 } };

// Helper #defines
// In next release these move into nv_sys...
#ifndef DISPDBG
#ifdef _DEBUG
#define DISPDBG(a, b)											\
do																\
{																\
	extern unsigned int g_DebugLevel;							\
	extern nv_gui::INVDebugConsole* g_pDebugConsole;			\
	if (g_pDebugConsole && a <= g_DebugLevel) {					\
		std::ostringstream strStream;							\
		strStream << b;											\
		g_pDebugConsole->OutputDebug(strStream.str().c_str());	\
	}															\
} while(0)
#else
#define DISPDBG(a, b) do { } while(0)
#endif
#endif

#ifndef DISPDBG_ALWAYS
#define DISPDBG_ALWAYS(a, b)									\
do																\
{																\
	extern unsigned int g_DebugLevel;							\
	extern nv_gui::INVDebugConsole* g_pDebugConsole;			\
	if (g_pDebugConsole && a <= g_DebugLevel) {					\
		std::ostringstream strStream;							\
		strStream << b;											\
		g_pDebugConsole->OutputDebug(strStream.str().c_str());	\
	}															\
} while(0)
#endif

#ifndef DISPCONSOLE
#define DISPCONSOLE(a)											\
do																\
{																\
	extern nv_gui::INVDebugConsole* g_pDebugConsole;			\
	std::ostringstream strStream;								\
	if (g_pDebugConsole)										\
	{															\
		strStream << a;											\
		g_pDebugConsole->OutputDebug(strStream.str().c_str());	\
	}															\
} while(0)
#endif

#ifndef DISPCONSOLE_ERROR
#define DISPCONSOLE_ERROR(a)									\
do																\
{																\
	extern nv_gui::INVDebugConsole* g_pDebugConsole;			\
	std::ostringstream strStream;								\
	if (g_pDebugConsole)										\
	{															\
		strStream << a;											\
		g_pDebugConsole->OutputDebug(strStream.str().c_str());	\
	    if (!g_pDebugConsole->IsVisible())                      \
		    g_pDebugConsole->SetVisible(true);                  \
	}															\
} while(0)
#endif


#endif // __INVDEBUGCONSOLE_H