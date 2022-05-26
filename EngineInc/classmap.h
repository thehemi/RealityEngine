#ifndef CLASSMAP_H
#define CLASSMAP_H
#include <map>

ENGINE_API extern map<string, class Factory*> _registry;

#ifndef DOXYGEN_IGNORE
struct HashKey
{
	unsigned long Hash;
	string className;
};
#endif

/// Class factory, for instantiating C++ or C# Actors by classname string
class ENGINE_API Factory {
private:
	static vector<HashKey> HashKeys;
public:
/*	static unsigned long hash(const char *string) 
	{  
		unsigned long val;  
		char *p;  

		for (p=(char*)string;*p;p++)   
			val = (val << 5) | *p;

		return val % 9000000;
	}   */

	// hash algorithm taken from SGI's STL
 static unsigned long hash(const string& _Keyval)
  {
   const char* __s=_Keyval.c_str();
   unsigned long __h = 0;
   for ( ; *__s; ++__s)
    __h = 5*__h + *__s;
   return __h;
  }


	static void RegisterHash(string className)
	{
		int index = HashKeys.size();
		HashKeys.resize(index + 1);
		HashKeys[index].className = className;
		HashKeys[index].Hash = hash(className);
	}

	static void PrintHashKeys()
	{
		for(int index = 0; index < HashKeys.size(); index++)
		{
		LogPrintf("Hash code %s : %i",HashKeys[index].className.c_str(),HashKeys[index].Hash);
		}
	}

	static unsigned long GetHashKey(string className)
	{
		for(int i = 0; i < HashKeys.size(); i++)
		{
			if(HashKeys[i].className == className)
				return HashKeys[i].Hash;
		}
		return 0;
	}

	static string GetClassName(unsigned long classHash)
	{
		for(int i = 0; i < HashKeys.size(); i++)
		{
			if(HashKeys[i].Hash == classHash)
				return HashKeys[i].className;
		}
		return "";
	}


	static Actor* create(unsigned long classHash,World* w) 
	{
		string className = GetClassName(classHash);

		if(!className.length())
			return NULL;

		if(FoundInRegistry(className))
			return _registry[className]->performCreate(w);
		else
			return w->CreateScriptActor(className);
	}

	static Actor* create(string className,World* w) {
		if(FoundInRegistry(className))
			return _registry[className]->performCreate(w);
		else
			return w->CreateScriptActor(className);
	}
	static vector<string> GetClasses()
	{
		vector<string> classList;
		map<string, Factory*>::iterator ppEachItem;
		for ( ppEachItem = _registry.begin();
			ppEachItem != _registry.end(); ppEachItem++ )
		{
			if(ppEachItem->second)
				classList.push_back(ppEachItem->second->m_MyClassName);
		}
		return classList;
	}
	static bool FoundInRegistry(string className)
	{
		map<string, Factory*>::iterator ppEachItem;
		for ( ppEachItem = _registry.begin();
			ppEachItem != _registry.end(); ppEachItem++ )
		{
			if(ppEachItem->second->m_MyClassName == className)
				return true;
		}
		return false;
	}
	static int GetIndex(string ClassName)
	{
		map<string, Factory*>::iterator ppEachItem;
		int i = 0;
		for ( ppEachItem = _registry.begin();
			ppEachItem != _registry.end(); ppEachItem++ )
		{
			if(ppEachItem->second->m_MyClassName == ClassName)
				return i;

			i++;
		}
		return -1;
	}
	static Actor* CreateFromIndex(int index, World* w)
	{
		map<string, Factory*>::iterator ppEachItem;
		int i = 0;
		for ( ppEachItem = _registry.begin();
			ppEachItem != _registry.end(); ppEachItem++ )
		{
			if(i == index)
				return ppEachItem->second->performCreate(w);

			i++;
		}
		return 0;
	}
	string m_MyClassName;
protected:
	Factory(string className )
	{
		Factory::RegisterHash(className);
		m_MyClassName = className;
		_registry[className] = this;
	}
private:
	virtual Actor* performCreate(World* w) = 0;

};

#ifndef DOXYGEN_IGNORE
/// The TypedFactory template creates an instance of
/// class T.
template <class T> 
class TypedFactory : public Factory {
public:
	TypedFactory();

private:
	Actor* performCreate(World* w) { return new T(w); };
	static TypedFactory _instance;
};
#endif

/// The REGISTER_FACTORY macro instantiates a given object
/// factory and ensures that it has been added to the registry
/// of available object factories.
#define REGISTER_FACTORY( T ) \
	template <> TypedFactory<T>::TypedFactory() : Factory( #T ) {} \
	TypedFactory<T> TypedFactory<T>::_instance; 
#endif