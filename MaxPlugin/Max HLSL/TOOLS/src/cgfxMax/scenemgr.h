#ifndef __SCENEMGR_H
#define __SCENEMGR_H

#include "shaderinfo.h"
#include <set>

typedef enum tagSCENEMGR_MESSAGE
{
	SCENEMGR_ADDNODE = 0,
	SCENEMGR_REMOVENODE = 1,
	SCENEMGR_PRENEWSCENE = 2

} eSCENEMGR_MESSAGE;

class INotificationCallback
{
public:
	virtual bool NotifyAddNode(void* pValue) = 0;
	virtual bool NotifyRemoveNode(void* pValue) = 0;
	virtual bool PreNewScene(void* pValue) = 0;
};

typedef std::set<INotificationCallback*> tsetNotification;
class CSceneManager : public Singleton<CSceneManager>
{
public:
	CSceneManager()
	{
		NVPROF_FUNC("CSceneManager::CSceneManager");
	}

	bool AddSink(INotificationCallback* pNotify)
	{
		NVPROF_FUNC("CSceneManager::AddSink");
		m_setNotification.insert(pNotify);
		return true;
	}

	bool RemoveSink(INotificationCallback* pNotify)
	{
		NVPROF_FUNC("CSceneManager::RemoveSink");
		tsetNotification::iterator itrFound = m_setNotification.find(pNotify);
		m_setNotification.erase(itrFound);
		return true;
	}

	bool Notify(eSCENEMGR_MESSAGE Message, void* Value)
	{
		NVPROF_FUNC("CSceneManager::BroadcastNodeRemove");
		tsetNotification::iterator itrCurrent = m_setNotification.begin();
		while (itrCurrent != m_setNotification.end())
		{
			switch(Message)
			{
				case SCENEMGR_ADDNODE:
				{
					(*itrCurrent)->NotifyAddNode(Value);
				}
				break;

				case SCENEMGR_REMOVENODE:
				{
					(*itrCurrent)->NotifyRemoveNode(Value);
				}
				break;
				case SCENEMGR_PRENEWSCENE:
				{
					(*itrCurrent)->PreNewScene(Value);
				}
				break;

			}
			itrCurrent++;
		}
		return true;
	}

private:
	tsetNotification m_setNotification;
};


#endif __EFFECTMGR_H