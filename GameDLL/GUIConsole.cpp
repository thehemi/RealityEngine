//=========== (C) Copyright 2004, Artificial Studios. All rights reserved. ===========
// 
//====================================================================================

#include "stdafx.h"
#include "Texture.h"
#include "RenderDevice.h"
#include "GUIConsole.h"
#include "GameEngine.h"

float LINE_TIME = 6; // Each line may remain for LINE_TIME seconds
int MAX_LINES = 5;
float lastLineTime = LINE_TIME; // How much time left for the oldest line

//--------------------------------------------------------------------------------------
//  prints a line of text to the console
//--------------------------------------------------------------------------------------
void GUIConsole::Printf(const char *fmt, ...){
	va_list		argptr;
	char		msg[1024];
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);
	AddHistoryLine(msg);
	if(g_Game.IsDedicated())historyString = historyString +"\r\n"+ msg;
}

//--------------------------------------------------------------------------------------
// prints a colored line of text to the console 
//--------------------------------------------------------------------------------------
void GUIConsole::PrintfColor(COLOR color,const char *fmt, ...){
	va_list		argptr;
	char		msg[1024];
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);
	AddHistoryLine(msg,color);
	if(g_Game.IsDedicated())historyString = historyString +"\r\n"+ msg;
}

GUIConsole::GUIConsole(){ }
GUIConsole::~GUIConsole(){ }

//--------------------------------------------------------------------------------------
// adds entry to the console's history list of previously displayed lines
//--------------------------------------------------------------------------------------
void GUIConsole::AddHistoryLine(string line,COLOR color){
	LogPrintf(("Con: "+line).c_str());
	historyLine histLine;
	histLine.text = line;
	histLine.color = color;
	strConsoleHistory.push_back(histLine);
	iRecentLines++;
}

//--------------------------------------------------------------------------------------
// clears all console text
//--------------------------------------------------------------------------------------
void GUIConsole::Clear()
{
	iRecentLines = 0;
	strConsoleHistory.resize(0);
}

//--------------------------------------------------------------------------------------
// Draws the console system onto the canvas
//--------------------------------------------------------------------------------------
void GUIConsole::Render(Canvas* canvas){
	// Get the height of the font set using a dummy call
	int font_height = canvas->GetTextSize(SmallFont,"T").cy;

	// Console up, check/draw any recent lines to viewport
	if(!bConsoleOn || bChatLineOn)
	{
		int startY = 40;

		if(bChatLineOn)
		{
			int yOff = startY+font_height*6;
			int xOff = 10;
			string displayString = string("> ") + strEditingLine;
			canvas->Textf(SmallFont,COLOR_RGBA(255,255,50,200),xOff,yOff,displayString.c_str());
			int x_pos = canvas->GetTextSize(SmallFont,displayString.substr(0,iCursorPos+2).c_str()).cx;
			canvas->Textf(SmallFont,COLOR_RGBA(255,255,50,200),xOff+x_pos,yOff,"_");
		}

		int fadeLineNum = strConsoleHistory.size() - iRecentLines - 1;
		if(fadeLineNum > -1 && fadeLineNum < strConsoleHistory.size() && fadeLineOpacityFactor > 0)
		{
			COLOR textColor = strConsoleHistory[fadeLineNum].color;
			canvas->Textf(SmallFont,COLOR_RGBA(COLOR_GETRED(textColor),COLOR_GETGREEN(textColor),COLOR_GETBLUE(textColor),(int)(((float)COLOR_GETALPHA(textColor)) * fadeLineOpacityFactor)),10,startY,strConsoleHistory[fadeLineNum].text.c_str());
			fadeLineOpacityFactor -= GDeltaTime*1.2;
		}

		// Check for recent lines to draw directly to view
		if(iRecentLines<=0)
			return;

		if(iRecentLines>MAX_LINES)
		{
			fadeLineOpacityFactor = 1;
			iRecentLines = MAX_LINES;
		}

		// Draw remaining lines
		for(int i=0;i<iRecentLines;i++)
		{
			int yUp = startY+font_height+(font_height*i);
			canvas->Textf(SmallFont,strConsoleHistory[(strConsoleHistory.size()-iRecentLines)+i].color,10,yUp,strConsoleHistory[(strConsoleHistory.size()-iRecentLines)+i].text.c_str());
		}

		// Remove oldest line when expired
		// Update timer
		lastLineTime -= GDeltaTime;
		if(lastLineTime <= 0)
		{

			iRecentLines--;
			fadeLineOpacityFactor = 1;
			if(iRecentLines>=0)
				lastLineTime = LINE_TIME;
		}
		return;
	}
	/* The console is on, so draw all of its text and graphics */
	
	//set the viewport to front of Z buffer for canvas draws
	int minZ = RenderDevice::Instance()->MinViewportZ;
	int maxZ = RenderDevice::Instance()->MaxViewportZ;
	D3DVIEWPORT9 viewport;
	viewport.X = 0;
	viewport.Y = 0;
	viewport.MinZ = 0;
	viewport.MaxZ = .05;
	viewport.Height = Canvas::Instance()->Height;
	viewport.Width = Canvas::Instance()->Width;
	RenderWrap::dev->SetViewport(&viewport);

	int xOff = 15;
	int yOff = 238;

	// Draw the console Quads
	{
		float conWidth =  1026;
		float conHeight = 768/3;
		float x = -1, y = -1; 

		// Crude texture animation
		static float t = 0;
		ConsoleTex[0].uOff += GDeltaTime * 0.1f;
		ConsoleTex[0].vOff += GDeltaTime * 0.1f;

		// Alpha from diffuse
		RenderWrap::SetTSS(0,D3DTSS_ALPHAOP , D3DTOP_BLENDDIFFUSEALPHA);
		RenderWrap::SetTSS(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE );

		canvas->Box(COLOR_ARGB(200,255, 255, 255),x,y,conWidth,conHeight,&ConsoleTex[0],BLEND_SRCALPHA,BLEND_INVSRCALPHA);

		// Normal texture coordinates
		ConsoleTex[1].uTile = 2;

		// Alpha from texture
		RenderWrap::SetTSS(0,D3DTSS_ALPHAOP ,D3DTOP_SELECTARG1);
		RenderWrap::SetTSS(0,D3DTSS_ALPHAARG1 ,D3DTA_TEXTURE   );

		canvas->Box(COLOR_ARGB(200,255, 255, 255),x,y,conWidth,conHeight,&ConsoleTex[1],BLEND_INVDESTCOLOR,BLEND_DESTCOLOR);

		RenderWrap::SetTSS(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		RenderWrap::SetTSS(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE );
		RenderWrap::SetTSS(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE );
	}

	// Insertion point
	canvas->Textf(SmallFont,COLOR_ARGB(255,0,220,0),xOff-13,yOff,">");

	// Current Line
	canvas->Textf(SmallFont,COLOR_ARGB(255,200,255,200),xOff,yOff,strEditingLine.c_str());

	// History Lines
	for(int i=0;i<strConsoleHistory.size();i++){
		int yUp = yOff-(font_height*(i+2));
		// We've gone too high up, draw ... and stop
		if(yUp <= 20){
			canvas->Textf(SmallFont,COLOR_ARGB(255,0,220,0),xOff,yUp,"...");
			break;
		}
		canvas->Textf(SmallFont,strConsoleHistory[strConsoleHistory.size()-(i+1)].color,xOff,yUp,strConsoleHistory[strConsoleHistory.size()-(i+1)].text.c_str());
	}

	static int count = 0;
	count++;

	// Blinking cursor
	if(count>50){
		int x_pos = canvas->GetTextSize(SmallFont,strEditingLine.substr(0,iCursorPos).c_str()).cx;
		canvas->Textf(SmallFont,COLOR_ARGB(255,0,255,0),xOff+x_pos,yOff,"_");
	}
	if(count>100)
		count=0;

	viewport.MinZ = minZ;
	viewport.MaxZ = maxZ;
	RenderWrap::dev->SetViewport(&viewport);
}

//--------------------------------------------------------------------------------------
// Delete key; deletes character at cursor pos 
//--------------------------------------------------------------------------------------
void GUIConsole::KeyBackspace(){
	// Return if there's nothing to delete
	if(strEditingLine == "" || iCursorPos == 0)
		return;

	// Erase one character
	strEditingLine.erase(iCursorPos-1,1);
	iCursorPos--;
}

//--------------------------------------------------------------------------------------
// Down key; scrolls to most recent line
//--------------------------------------------------------------------------------------
void GUIConsole::KeyArrowDown(){

	// If this line is 1 beyond the end of history, it's the current line
	if(iHistoryLine == strTypedHistory.size()-1){
		strEditingLine = strOldEditingLine;
		iCursorPos = strEditingLine.length();
		return;
	}

	// Can't go past most recent line (failsafe)
	if(iHistoryLine > strTypedHistory.size()-1)
		return;

	iHistoryLine++;

	strEditingLine = strTypedHistory[iHistoryLine];
	iCursorPos = strEditingLine.length();
}

//--------------------------------------------------------------------------------------
// Up key; previous line is shown
//--------------------------------------------------------------------------------------
void GUIConsole::KeyArrowUp(){
	// We can't go further back
	if(iHistoryLine == 0)
		return;

	iHistoryLine--;

	strEditingLine = strTypedHistory[iHistoryLine];
	iCursorPos = strEditingLine.length();
}

//--------------------------------------------------------------------------------------
// initialized during game init process
//--------------------------------------------------------------------------------------
void GUIConsole::Initialize(string frontTex, string backTex){
	iCursorPos = 0;
	fadeLineOpacityFactor = 1;
	bConsoleOn = false;
	iRecentLines = 0;
	iHistoryLine = 0;
	ConsoleTex[0].Load(backTex);
	ConsoleTex[1].Load(frontTex);
}

//--------------------------------------------------------------------------------------
// /* Windows Messages */
//--------------------------------------------------------------------------------------
void GUIConsole::MsgChar(unsigned int wParam){
	char chrKey = wParam;

	// Toggle console if ` key
	if(chrKey == '`' || wParam == 192 || wParam == 223) // 226 or 192 is code from realitybuilder
	{
		if(bChatLineOn)
			bChatLineOn = false;
		else
			bConsoleOn = !bConsoleOn;
			
		g_Game.SetCursorVisible(bConsoleOn || g_Game.HasAnyGUIopen());
	}

	// Do nothing if the console is not on
	if(!bConsoleOn)
		return;

	// If key with letter pressed
	if( chrKey > 0 && isprint(chrKey) && chrKey!='`') 
	{
		// insert it to editing line
		string strChr = "";strChr+=chrKey;
		strEditingLine.insert( iCursorPos, strChr);
		strOldEditingLine = strEditingLine;
		iCursorPos++;
	}
}

//--------------------------------------------------------------------------------------
// /* Windows Messages */
//--------------------------------------------------------------------------------------
void GUIConsole::MsgKeyDown(unsigned int wParam){
	// Do nothing if the console is not on
	if(!bConsoleOn)
		return;

	bool bShift = GetKeyState(VK_SHIFT) & 0x8000;
	switch( wParam) {
		case VK_RETURN:  
			{
				if(bConsoleOn)KeyReturn();
				break;
			}
		case VK_UP:      if(bConsoleOn)KeyArrowUp();     break;
		case VK_DOWN:    if(bConsoleOn)KeyArrowDown();   break;
		case VK_TAB:     if(bConsoleOn)KeyTab();   break;
		case VK_BACK:    KeyBackspace();   break;
		case VK_DELETE:  KeyBackspace();   break;
		case VK_LEFT:    if(bConsoleOn)if( iCursorPos > 0)                      iCursorPos--;  break;
		case VK_RIGHT:   if(bConsoleOn)if( iCursorPos < strEditingLine.length()) iCursorPos++;  break;
	}
}

//--------------------------------------------------------------------------------------
// Return key is pressed, line is processed
//--------------------------------------------------------------------------------------
void GUIConsole::KeyReturn(){
	// Remove any excess white space
	trimLeft(strEditingLine);
	trimRight(strEditingLine);

	// Ignore blank lines
	if(strEditingLine == ""){
		iCursorPos = 0;
		return;
	}

	// Add to typed history
	strTypedHistory.push_back(strEditingLine);

	if(!bChatLineOn)
	{
	// Add line to console history
	AddHistoryLine(strEditingLine);
	// Handle line
	// If / send command, respond if bad command
	// else if Networked send chat
	//if(strEditingLine.find("/")==0)strEditingLine = strEditingLine.substr(1,strEditingLine.length());
	string rValue = shell.ExecuteSymbol(strEditingLine);
	if(rValue.size() > 0)AddHistoryLine(rValue);
	else 
		Client::Instance()->Say(strEditingLine.c_str());
	}
	else
		Client::Instance()->Say(strEditingLine.c_str());

	// Reset line and cursor pos and history line (for up/down searching)
	strEditingLine = "";
	strOldEditingLine = strEditingLine;
	iCursorPos = 0;
	iHistoryLine = strTypedHistory.size();

	if(bChatLineOn)
	{
		bChatLineOn = false;
		bConsoleOn = false;
	}
}

//--------------------------------------------------------------------------------------
// Tab key; shows commands
//--------------------------------------------------------------------------------------
void GUIConsole::KeyTab(){
	// Implement command scanning
	// Just show all commands
	AddHistoryLine("---------- Console Commands and Variables: ----------");
	// Max 60 chrs per line
	string details;
	for(int i=0;i<shell.Con_GetNumSymbols();i++){
		details += shell.Con_GetSymbolName(i) + "      ";
		if(details.length()>60){
			AddHistoryLine(details);
			details = "";
		}
	}
	if(details!="")
		AddHistoryLine(details);
}


//--------------------------------------------------------------------------------------
// registerd a var or function for use with the console exceution system
//--------------------------------------------------------------------------------------
void GUIConsole::Shell::DeclareSymbol(ShellType type, string name, void* pSymbol){
	// Build new symbol
	ShellSymbol newSymbol;
	newSymbol.sType = type;
	newSymbol.pvValue = pSymbol;
	newSymbol.strName = name;

	// Add to list
	shellSymbols.push_back(newSymbol);
}

//--------------------------------------------------------------------------------------
// Finds a specified command symbol within the execution shell, returns the index on the ShellSymbol list
//--------------------------------------------------------------------------------------
int GUIConsole::Shell::FindCmdName(string s){
	ToLowerCase(s);

	vector<ShellSymbol>::iterator ppEachItem;
	int n = 0;
	for ( ppEachItem = shellSymbols.begin();
		ppEachItem != shellSymbols.end(); ppEachItem++ )
	{
		// Compare the name, ignoring any parenthesis
		string cmp = (*ppEachItem).strName;
		if(cmp.find("(")!=-1)
			cmp = cmp.substr(0,cmp.find("("));
		ToLowerCase(cmp);
		if(cmp == s)
			return n;
		n++;
	}

	return -1;
}

//--------------------------------------------------------------------------------------
// Returns symbol value if a var, converted to string
//--------------------------------------------------------------------------------------
string GUIConsole::Shell::GetSymbolValue(ShellSymbol symbol){
	char val[128];
	// Figure out the data value
	switch(symbol.sType){
	case ST_INT:
		sprintf(val,"%d",*(int*)symbol.pvValue);
		break;
	case ST_STRING:
		sprintf(val,"%s",(char*)symbol.pvValue);
		break;
	case ST_FLOAT:
		sprintf(val,"%f",*(float*)symbol.pvValue);
		break;
	case ST_BOOL:
		sprintf(val,"%s",((*(bool*)symbol.pvValue)? "true":"false"));
		break;
	case ST_VECTOR:
		sprintf(val,"%f %f %f",(*(Vector*)symbol.pvValue).x,(*(Vector*)symbol.pvValue).y,(*(Vector*)symbol.pvValue).z);
		break;
	}
	return val;
}

//--------------------------------------------------------------------------------------
// Parses & executes command, whether a function with params or setting of a var
//--------------------------------------------------------------------------------------
string GUIConsole::Shell::ExecuteSymbol(string cmd)
{
	string rvalue;
	string name;

	if(cmd.find("set ")!=-1)
		cmd = cmd.substr(cmd.find("set ")+4);

	// Get the pure name
	if(cmd.find("(")!=-1)
		name = cmd.substr(0,cmd.find("("));
	else
		name = cmd.substr(0,cmd.find(" "));

	// Search for the pure name
	int index = FindCmdName(name);
	if(index == -1)return "";

	ShellSymbol symbol = shellSymbols[index];

	// If it's a function execute it
	if(symbol.sType == ST_FUNCTION)
	{
		if(cmd.find("=")!=-1)
			return "That's a command, you can't give it a value, silly!";
		// Extract the params, either in parenthesis or not, what do I care?
		string param;
		if(cmd.find("(")!=-1)
			param = cmd.substr(cmd.find("(")+1,cmd.find_last_of(")")-(cmd.find("(")+1));
		else
			param = cmd.substr(cmd.find(" ")+1,cmd.length());

		// No params
		if(param == "")
			( (void (*)(void))symbol.pvValue)();
		// params
		else{
			// TIM: Removed - so functions get numbers as strings
			// See if it's a number
			//char* s;
			//errno = 0;
			//int number = strtol(param.c_str(),&s,10);
			//if(!isdigit((int)param[0])) // It's not a number
			( (void (*)(char*))symbol.pvValue)((char*)param.c_str());
			//else			// It's a number
			//	( (void (*)(int))symbol.pvValue)(number);
		}
		return "Command Executed";
	}

	// If it's a var without assignment display value
	if(cmd.find(" ")==-1){
		return name + " = " + GetSymbolValue(symbol);
	}
	// This is an assignment
	// Filter out the value part
	string value;
	if(cmd.find(" ")!=-1){
		value = cmd.substr(cmd.find(" ")+1,cmd.length());
	}
	else{
		value = cmd.substr(cmd.find("=")+1,cmd.length());
		trimLeft(value);
	}
	// Figure out the data type and then assign it
	switch(symbol.sType){
		case ST_INT:
			*(int*)symbol.pvValue = atoi(value.c_str());
			break;
		case ST_STRING:
			// This is SOOO EVIL
			((string*)symbol.pvValue)->resize(256);
			strcpy((char*)symbol.pvValue,value.c_str());
			break;
		case ST_FLOAT:
			*(float*)symbol.pvValue = atof(value.c_str());
			break;
		case ST_VECTOR:
			{
				string x = value.substr(0,value.find(" "));
				value = value.substr(value.find(" ")+1);
				string y = value.substr(0,value.find(" "));
				value = value.substr(value.find(" ")+1);
				string z = value.substr(0,value.find(" "));

				*(Vector*)symbol.pvValue = Vector(atof(x.c_str()),atof(y.c_str()),atof(z.c_str()));
				break;
			}

		case ST_BOOL:{
			if(value.find("true")!=-1||value.find("1")!=-1)
				*(bool*)symbol.pvValue = true;
			else if(value.find("false")!=-1||value.find("0")!=-1)
				*(bool*)symbol.pvValue = false;
			else 
				return "Invalid value for bool. Use true/false or 1/0";
					 }
					 break;
	}
	return name +" set to: "+GetSymbolValue(symbol);
}