//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
/// Quake-style console engine, including logic for binding commands to functions
///
///
/// Author: Tim Johnson
//====================================================================================

#ifndef CONSOLE_INCLUDED
#define CONSOLE_INCLUDED


//--------------------------------------------------------------------------------------
/// Command shell for console
//--------------------------------------------------------------------------------------
enum ShellType {
	ST_ILLEGAL  ,
	ST_POINTER  ,
	ST_FUNCTION ,
	ST_ARRAY    ,
	ST_VOID     ,
	ST_INT	  ,
	ST_VECTOR   ,
	ST_FLOAT    ,
	ST_STRING   ,
	ST_BOOL,
};


//--------------------------------------------------------------------------------------
/// Console Interface
//--------------------------------------------------------------------------------------
class GAME_API GUIConsole{
private:
	/// A symbol defined for use in shell.
	struct ShellSymbol {
		/// type of the symbol
		ShellType sType;   
		/// symbol name
		string strName;  
		/// symbol value
		void *pvValue;     
	};
	struct historyLine
	{
		string text;
		COLOR color;
	};
	//
	/// Shell is a class for storing and executing data and functions
	//
	class Shell {
	private:
		vector<ShellSymbol> shellSymbols;
		// Finds a specified command symbol within the execution shell, returns the index on the ShellSymbol list
		int FindCmdName(string s);
		// Returns symbol value if a var, converted to string
		string GetSymbolValue(ShellSymbol symbol);
	public:
		// registerd a var or function for use with the console exceution system
		void DeclareSymbol(ShellType type, string name, void* pSymbol);
		// Parses & executes command, whether a function with params or setting of a var
		string ExecuteSymbol(string cmd);
		// Returns string name of symbol, function or var, on the array
		string Con_GetSymbolName(int index){ return shellSymbols[index].strName; }
		int Con_GetNumSymbols(){ return shellSymbols.size(); }
	};

	// Private Data
	/// Previous Lines
	vector<historyLine> strConsoleHistory; 
	/// Old editing line while user scrolls up and down
	string strOldEditingLine;	
	/// Recent lines to draw directly to viewport
	int iRecentLines;				  
	/// animated fade factor for 'disappearing' history line
	float fadeLineOpacityFactor; 
	/// Current history line on (for up/down arrows)
	int iHistoryLine;		
	/// State of console
	bool bConsoleOn;	
	/// Textures
	Texture ConsoleTex[2];			  

	/// Return key is pressed, line is processed
	void KeyReturn();				
	/// Up key; previous line is shown
	void KeyArrowUp();				
	/// Down key; scrolls to most recent line
	void KeyArrowDown();			
	/// Delete key; deletes character at cursor pos
	void KeyBackspace();	
	/// Tab key; shows commands
	void KeyTab();	
	// in dedicated mode, stores all console text for display in the command line
	string	historyString;
	// adds entry to the console's history list of previously displayed lines
	void AddHistoryLine(string line, COLOR color = COLOR_RGBA(0,220,0,255));	
	bool bChatLineOn;

public:
	/// Previous TYPED lines
	vector<string> strTypedHistory; 
	// clears all console text
	void Clear();
	void Hide()
	{
		bConsoleOn = false;
		bChatLineOn = false;
	}
	void Show(bool ChatLineOnly = false)
	{
		bConsoleOn = true;
		bChatLineOn = ChatLineOnly;
	}
	bool IsChatLineOn(){return bChatLineOn;}
	void ClearEditingLine(){strEditingLine="";iCursorPos=0;}
	/// Current Line
	string strEditingLine;
	/// Command shell for console
	Shell shell;	
	/// Position of blinking text cursor
	int iCursorPos;					  
	GUIConsole();
	~GUIConsole();

	/* Windows Messages */
	void MsgKeyDown(unsigned int wParam);
	void MsgChar(unsigned int wParam);

	// Draws the console system onto the canvas
	void Render(class Canvas* canvas);
	// initialized during game init process
	void Initialize(string frontTex, string backTex);
	bool IsOn(){ return bConsoleOn; }

	// prints a line of text to the console
	void Printf(const char *fmt, ...);
	// prints a colored line of text to the console
	void PrintfColor(COLOR color,const char *fmt, ...);

	// Registers a variable or function for access within the console
	void DeclareSymbol(ShellType type, string name, void* pSymbol) { shell.DeclareSymbol(type,name,pSymbol); }
};
#endif