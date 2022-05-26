#ifndef __NV_PROF_H
#define __NV_PROF_H

#ifdef TIM_NVPROFILE_ON

#pragma warning (disable : 4786)
#include <sstream>
#pragma warning (disable : 4786)
#include <string>
#pragma warning (disable : 4786)
#include <map>
#pragma warning (disable : 4786)
#include <vector>
#pragma warning (disable : 4786)

static const unsigned int NVPOP_BIT = 0x80000000;

#define NVPROF_STARTEVENT "NVPROF_STARTEVENT"
#define NVPROF_STOPEVENT "NVPROF_STOPEVENT"
#define NVPROF_SETUPEVENT "NVPROF_SETUPEVENT"
#define NVPROF_ACKEVENT "NVPROF_ACKEVENT"

#define NVPROF_SHAREDMEM "NVPROF_SHAREDMEM"
#define NVPROF_FUNCTIONNAMEMEM "NVPROF_FUNCTIONNAMEMEM"

static inline __int64 ReadTSC_profile(void)
{
  __int64 mmRet;
  __asm {
	push eax
	push edx
	push ecx
    rdtsc
    mov   dword ptr [mmRet+0],eax
    mov   dword ptr [mmRet+4],edx
	pop ecx
	pop edx
	pop eax
  }
  return mmRet;
}

typedef struct tagNVProfFuncRecord
{
	unsigned int m_ID;
	__int64 m_Time;
} tNVProfFuncRecord;

static const unsigned int NVPROFID_INVALID = 0;
typedef std::map<unsigned int, std::string> tmapFuncIDString;

class CNVProfManager
{
public:
	CNVProfManager()
		: m_NextID(1),
		m_CurrentSlot(0),
		m_RecordSize(0),
		m_bStarted(false),
		m_pRecord(NULL),
		m_hSharedMemory(INVALID_HANDLE_VALUE),
		m_pFunctionNames(NULL),
		m_hFunctionNames(INVALID_HANDLE_VALUE),
		m_hStartEvent(INVALID_HANDLE_VALUE),
		m_hStopEvent(INVALID_HANDLE_VALUE),
		m_hSetupEvent(INVALID_HANDLE_VALUE),
		m_hAckEvent(INVALID_HANDLE_VALUE)
	{

		OpenEvents();
	}

	~CNVProfManager()
	{
		CleanupMemory();
		CleanupEvents();
	}

	unsigned int GetNextID(const char* pszName)
	{
		unsigned int NewID = m_NextID;
		m_mapFuncIDString[NewID] = pszName;
		m_NextID++;
		return NewID;
	}

	void Push(unsigned int ID)
	{
		if (m_bStarted && (m_CurrentSlot < m_RecordSize))
		{
			m_pRecord[m_CurrentSlot].m_ID = ID;
			m_pRecord[m_CurrentSlot].m_Time = ReadTSC_profile();
			m_CurrentSlot++;
		}
	}

	void Pop(unsigned int ID)
	{
		if (m_bStarted && (m_CurrentSlot < m_RecordSize))
		{
			m_pRecord[m_CurrentSlot].m_ID = ID | NVPOP_BIT;
			m_pRecord[m_CurrentSlot].m_Time = ReadTSC_profile();
			m_CurrentSlot++;
		}
	}

	void CheckEvents()
	{
		CheckSetupEvent();
		CheckStartEvent();
		CheckStopEvent();
	}

	void CheckStartEvent()
	{
		if (m_hStartEvent == INVALID_HANDLE_VALUE || m_hSetupEvent == INVALID_HANDLE_VALUE || m_pRecord == NULL)
			return;

		DWORD dwRet = WaitForSingleObject(m_hStartEvent, 0);
		if (dwRet != WAIT_OBJECT_0)
			return;
		
		m_CurrentSlot = 0;
		m_bStarted = true;
		
		SetEvent(m_hAckEvent);
	}

	void CheckStopEvent()
	{
		if (m_hStopEvent == INVALID_HANDLE_VALUE)
			return;

		DWORD dwRet = WaitForSingleObject(m_hStopEvent, 0);
		if (dwRet != WAIT_OBJECT_0)
			return;

		// Stick an invalid event on the end.
		Push(NVPROFID_INVALID);

		m_bStarted = false;

		
		// Remove the function name list
		if (m_pFunctionNames)
			UnmapViewOfFile(m_pFunctionNames);
		if (m_hFunctionNames)
			CloseHandle(m_hFunctionNames);

		// Find out how big the function names are
		// First entry in the function names is the actual size of the buffer.
		DWORD dwFuncSize = sizeof(unsigned int);
		tmapFuncIDString::iterator itrFunctions = m_mapFuncIDString.begin();
		while (itrFunctions != m_mapFuncIDString.end())
		{
			dwFuncSize += itrFunctions->second.size() + 1; // string + the zero
			dwFuncSize += sizeof(unsigned int);	// ID
			itrFunctions++;
		}

		// Create a shared memory buffer for all the functions
		m_hFunctionNames = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, dwFuncSize, NVPROF_FUNCTIONNAMEMEM);
		if (m_hFunctionNames != INVALID_HANDLE_VALUE)
		{
			m_pFunctionNames = (void*)MapViewOfFile(m_hFunctionNames, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, dwFuncSize);
		}
		else
		{
			if (GetLastError() == ERROR_ALREADY_EXISTS)
				assert(!"Filemapping shouldn't exist!");
		}

		if (m_pFunctionNames)
		{
			// Copy the functions into the buffer.
			ZeroMemory(m_pFunctionNames, dwFuncSize);
			BYTE* pCurrentFunction = (BYTE*)m_pFunctionNames;
			
			// Copy the size
			*(unsigned int*)pCurrentFunction = dwFuncSize;
			pCurrentFunction += sizeof(unsigned int);

			itrFunctions = m_mapFuncIDString.begin();
			while (itrFunctions != m_mapFuncIDString.end())
			{
				// Store the ID
				*(unsigned int*)pCurrentFunction = itrFunctions->first;
				pCurrentFunction += sizeof(unsigned int);

				// Store the string
				memcpy(pCurrentFunction, itrFunctions->second.c_str(), itrFunctions->second.size());
				pCurrentFunction += itrFunctions->second.size() + 1;
	
			
				itrFunctions++;
			}
		}

		SetEvent(m_hAckEvent);
	}

	void CleanupMemory()
	{
		if (m_pRecord)
			UnmapViewOfFile(m_pRecord);
		if (m_hSharedMemory != INVALID_HANDLE_VALUE)
			CloseHandle(m_hSharedMemory);
		if (m_pFunctionNames)
			UnmapViewOfFile(m_pFunctionNames);
		if (m_hFunctionNames != INVALID_HANDLE_VALUE)
			CloseHandle(m_hFunctionNames);

		m_pRecord = NULL;
		m_hSharedMemory = NULL;
		m_pFunctionNames = NULL;
		m_hFunctionNames = NULL;

	}

	void CleanupEvents()
	{
		if (m_hStartEvent != INVALID_HANDLE_VALUE)
			CloseHandle(m_hStartEvent);
		if (m_hStopEvent != INVALID_HANDLE_VALUE)
			CloseHandle(m_hStopEvent);
		if (m_hSetupEvent != INVALID_HANDLE_VALUE)
			CloseHandle(m_hSetupEvent);
		if (m_hAckEvent != INVALID_HANDLE_VALUE)
			CloseHandle(m_hAckEvent);

		m_hStartEvent = NULL;
		m_hStopEvent = NULL;
		m_hSetupEvent = NULL;
		m_hAckEvent = NULL;
	}

	void OpenEvents()
	{
		m_hStartEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, NVPROF_STARTEVENT);
		m_hStopEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, NVPROF_STOPEVENT);
		m_hSetupEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, NVPROF_SETUPEVENT);
		m_hAckEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, NVPROF_ACKEVENT);
	}

	void CheckSetupEvent()
	{
		if (m_hSetupEvent == INVALID_HANDLE_VALUE)
			return;

		DWORD dwRet = WaitForSingleObject(m_hSetupEvent, 0);
		if (dwRet == WAIT_OBJECT_0)
		{
			ConnectionEnable();
		}
	}

	void ConnectionEnable()
	{
		CleanupMemory();

		m_hSharedMemory = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, NVPROF_SHAREDMEM);
		if (m_hSharedMemory != INVALID_HANDLE_VALUE)
		{
			// First data item contains the size ofthe record
			m_pRecord = (tNVProfFuncRecord*)MapViewOfFile(m_hSharedMemory, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(unsigned int));
			m_RecordSize = *((unsigned int*)m_pRecord);
			m_pRecord = (tNVProfFuncRecord*)MapViewOfFile(m_hSharedMemory, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, m_RecordSize * sizeof(tNVProfFuncRecord));
		}
		
		m_CurrentSlot = 0;
	}


	tNVProfFuncRecord* GetRecords(unsigned int& RecordSize)
	{
		RecordSize = m_RecordSize;
		return m_pRecord;
	}

private:
	tmapFuncIDString m_mapFuncIDString;
	tNVProfFuncRecord* m_pRecord;
	unsigned int m_NextID;
	unsigned int m_CurrentSlot;
	unsigned int m_RecordSize;
	bool m_bStarted;
	HANDLE m_hStartEvent;
	HANDLE m_hStopEvent;
	HANDLE m_hSetupEvent;
	HANDLE m_hSharedMemory;
	HANDLE m_hAckEvent;

	HANDLE m_hFunctionNames;
	void* m_pFunctionNames;
};

extern CNVProfManager g_NVProfManager;

class CNVProfFuncID
{
public:
	CNVProfFuncID(const char* pszName)
	{
		m_ID = g_NVProfManager.GetNextID(pszName);
	}

	unsigned int m_ID;
};

class CNVProfFuncRegister
{
public:
	CNVProfFuncRegister(unsigned int ID)
		: m_ID(ID)
	{
		g_NVProfManager.Push(ID);
	}
	~CNVProfFuncRegister()
	{
		g_NVProfManager.Pop(m_ID);
	}
private:
	unsigned int m_ID;
};

// Macros
#define NVPROF_FUNC(a)\
static CNVProfFuncID __funcID(a);						\
CNVProfFuncRegister __funcRegister(__funcID.m_ID);

#define DECLARE_NVPROFILE_MANAGER() CNVProfManager g_NVProfManager
#define CHECK_NVPROFILE_EVENTS() g_NVProfManager.CheckEvents()

#else

// Undefined usually
#define NVPROF_FUNC(a)
#define DECLARE_NVPROFILE_MANAGER()
#define CHECK_NVPROFILE_EVENTS()

#endif

#endif // _header