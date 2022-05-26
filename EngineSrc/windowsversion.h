TCHAR OSComputerName[MAX_COMPUTERNAME_LENGTH + 1];
TCHAR OSUserName[MAX_COMPUTERNAME_LENGTH + 1];
TCHAR OSVersionText[32]; 
WORD OSVersionLanguage;     /// Operating System language code (407=german,409=english, ...)
ULONG OSVersionMajor;      /// Operating System Major Version
ULONG OSVersionMinor;      /// Operating System Minor Version


string getOSString ()
{
	LPVOID version;       /// Storage for the version resource
	DWORD  versionSIZE;      /// The size of the version structure
	UINT  languageLENGTH;     /// Length of actual version
	LPWORD languageINFO;     /// version struct
	OSVERSIONINFO vi={0};     /// version info struct
	TCHAR versionFILE[MAX_PATH]={0};   /// e.g. c:\winnt\system32\user.exe (allways user.exe)
	int err=0;
	DWORD dummy=MAX_COMPUTERNAME_LENGTH;

	GetComputerName(OSComputerName, &dummy);            /// OSComputerName
	dummy=MAX_COMPUTERNAME_LENGTH;
	GetUserName(OSUserName, &dummy);              /// OSUserName

	vi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx(&vi);                  /// get os version info

	if (vi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS){
		if  ( vi.dwMajorVersion > 4 || (vi.dwMajorVersion == 4) &&(vi.dwMinorVersion > 0) )
			_tcscpy (OSVersionText, _T("Windows 98")); 
		else
			_tcscpy (OSVersionText, _T("Windows 95"));
	}
	else{
		if (vi.dwPlatformId==VER_PLATFORM_WIN32_NT)
			if (vi.dwMajorVersion == 4)
				_tcscpy (OSVersionText, _T("Windows NT"));  
			else if (vi.dwMajorVersion == 5 && vi.dwMinorVersion == 0)
				_tcscpy (OSVersionText, _T("Windows 2000"));         
			else if(vi.dwMajorVersion == 5 && vi.dwMinorVersion == 1)
				_tcscpy (OSVersionText, _T("Windows XP"));          
			else
				_tcscpy (OSVersionText, _T("Unknown OS"));   
	}

	OSVersionMajor=vi.dwMajorVersion;              /// OSVersionMajor
	OSVersionMinor=vi.dwMinorVersion;              /// OSVersionMinor

	/// we do not set dwBuildNumber and szCSDVersion

	GetSystemDirectory (versionFILE, MAX_PATH);
	_tcscat (versionFILE, _T("\\user.exe"));            /// set path to .../user.exe

	versionSIZE = GetFileVersionInfoSize( versionFILE, &dummy );
	if ( versionSIZE > 0 ) {
		version = malloc (versionSIZE);            /// alloc memory for versioninfo
		if ( version ) {
			if (GetFileVersionInfo(versionFILE,0,versionSIZE,version)){
				/// Get file version information
				if (VerQueryValue( version, _T("\\VarFileInfo\\Translation"),
					(LPVOID*)&languageINFO, &languageLENGTH) !=0) {  /// get language code

						OSVersionLanguage=languageINFO[0];        /// OSVersionLanguage

					} else  err=13;
			} else err=12;
			free( version );              /// free memory
		} else err=11;
	} else err=10;

	if (err) {
		switch (err) {
   case 10: MessageBox (0, _T("failed to get version size"),   _T("error"), 10);
	   break;
   case 11: MessageBox (0, _T("failed to allocate memory"),   _T("error"), 10);
	   break;
   case 12: MessageBox (0, _T("failed to get version information"), _T("error"),
				10); break;
   case 13: MessageBox (0, _T("failed to get languagecode"),   _T("error"), 10);
	   break;
		}
	} 


	return OSVersionText + 
		string(_T("(")) + ToStr(OSVersionMajor) + _T(".") + ToStr(OSVersionMinor) + string(_T(")"))
		+ string(_T(" User:")) + OSUserName;
}
