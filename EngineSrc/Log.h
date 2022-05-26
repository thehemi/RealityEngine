//=========== Copyright (c) 2003, Tim Johnson. All rights reserved. ===========
/// \brief Global logging class.
/// 	Use LogPrintf() defined in Base.h to print to the log
//=============================================================================
#pragma once

class ENGINE_API EngineLog {
public:
	void Init(char* Name);
	void Exit();
	void Out(const char *fmt, ...);
	void PushPrefix(char* aPrefix);
	void PopPrefix();
};

