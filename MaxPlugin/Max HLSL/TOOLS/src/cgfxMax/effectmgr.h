#ifndef __EFFECTMGR_H
#define __EFFECTMGR_H

#include "shaderinfo.h"

class CEffectRecord
{
public:
	CEffectRecord(LPD3DXEFFECT pEffect = NULL)
		: m_pEffect(pEffect)
	{
		if (pEffect)
			m_RefCount = 1;
		else
			m_RefCount = 0;
	}

	void AddRef() { m_RefCount++; }
	unsigned long Release() 
	{
		m_RefCount--; 
		if (m_RefCount == 0)
		{
			SAFE_RELEASE(m_pEffect);
		}
		return m_RefCount;
	}

	LPD3DXEFFECT m_pEffect;
	unsigned int m_RefCount;

};

typedef std::map<std::string, CEffectRecord> tmapFileToEffectRecord;
class CEffectManager : public Singleton<CEffectManager>
{
public:
	CEffectManager()
	{
		NVPROF_FUNC("CEffectManager::CEffectManager");
	}

	LPD3DXEFFECT FindEffect(const char* pszName)
	{
		NVPROF_FUNC("CEffectManager::FindEffect");
		tmapFileToEffectRecord::iterator itrFound = m_mapFileToEffectRecord.find(pszName);
		if (itrFound != m_mapFileToEffectRecord.end())
		{
			itrFound->second.AddRef();
			return itrFound->second.m_pEffect;
		}

		return NULL;
	}

	bool AddEffect(const char* pszName, LPD3DXEFFECT pEffect)
	{
		NVPROF_FUNC("CEffectManager::AddEffect");
		m_mapFileToEffectRecord[pszName] = CEffectRecord(pEffect);

		return true;
	}

	bool RemoveEffect(LPD3DXEFFECT pEffect)
	{
		NVPROF_FUNC("CEffectManager::RemoveEffect");
		tmapFileToEffectRecord::iterator itrFound = m_mapFileToEffectRecord.begin();
		if (itrFound != m_mapFileToEffectRecord.end())
		{
			if (itrFound->second.m_pEffect == pEffect)
			{
				unsigned int Ref = itrFound->second.Release();

				// Kill the effect from the mapping
				if (Ref == 0)
				{
					m_mapFileToEffectRecord.erase(itrFound);
				}
				return true;
			}
			itrFound++;
		}
		return false;

	}


private:
	LPD3DXEFFECT m_pEffect;
	tmapFileToEffectRecord m_mapFileToEffectRecord;
};


#endif __EFFECTMGR_H