#define STRICT
#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#include <windows.h>
#include <shellapi.h>


// --------------------------------------------------------------------------------
// SetDlgItemUrl(hwnd,IDC_MYSTATIC,"http://www.wischik.com/lu");
//   This routine turns a dialog's static text control into an underlined hyperlink.
//   You can call it in your WM_INITDIALOG, or anywhere.
//   It will also set the text of the control... if you want to change the text
//   back, you can just call SetDlgItemText() afterwards.
// --------------------------------------------------------------------------------
void SetDlgItemUrl(HWND hdlg,int id,const char *url); 

// Implementation notes:
// We have to subclass both the static control (to set its cursor, to respond to click)
// and the dialog procedure (to set the font of the static control). Here are the two
// subclasses:
LRESULT CALLBACK UrlCtlProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK UrlDlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
// When the user calls SetDlgItemUrl, then the static-control-subclass is added
// if it wasn't already there, and the dialog-subclass is added if it wasn't
// already there. Both subclasses are removed in response to their respective
// WM_DESTROY messages. Also, each subclass stores a property in its window,
// which is a HGLOBAL handle to a GlobalAlloc'd structure:
typedef struct {char *url; WNDPROC oldproc; HFONT hf; HBRUSH hb;} TUrlData;
// I'm a miser and only defined a single structure, which is used by both
// the control-subclass and the dialog-subclass. Both of them use 'oldproc' of course.
// The control-subclass only uses 'url' (in response to WM_LBUTTONDOWN),
// and the dialog-subclass only uses 'hf' and 'hb' (in response to WM_CTLCOLORSTATIC)
// There is one sneaky thing to note. We create our underlined font *lazily*.
// Initially, hf is just NULL. But the first time the subclassed dialog received
// WM_CTLCOLORSTATIC, we sneak a peak at the font that was going to be used for
// the control, and we create our own copy of it but including the underline style.
// This way our code works fine on dialogs of any font.

// SetDlgItemUrl: this is the routine that sets up the subclassing.
void SetDlgItemUrl(HWND hdlg,int id,const char *url) 
{ // nb. vc7 has crummy warnings about 32/64bit. My code's perfect! That's why I hide the warnings.
#pragma warning( push )
#pragma warning( disable: 4312 4244 )
	// First we'll subclass the edit control
	HWND hctl = GetDlgItem(hdlg,id);
	//if(!desc) desc = url;
	//SetWindowText(hctl,desc);
	HGLOBAL hold = (HGLOBAL)GetProp(hctl,"href_dat");
	if (hold!=NULL) // if it had been subclassed before, we merely need to tell it the new url
	{ TUrlData *ud = (TUrlData*)GlobalLock(hold);
	delete[] ud->url;
	ud->url=new char[strlen(url)+1]; strcpy(ud->url,url);
	}
	else
	{ HGLOBAL hglob = GlobalAlloc(GMEM_MOVEABLE,sizeof(TUrlData));
	TUrlData *ud = (TUrlData*)GlobalLock(hglob);
	ud->oldproc = (WNDPROC)GetWindowLongPtr(hctl,GWLP_WNDPROC);
	ud->url=new char[strlen(url)+1]; strcpy(ud->url,url);
	ud->hf=0; ud->hb=0;
	GlobalUnlock(hglob);
	SetProp(hctl,"href_dat",hglob);
	SetWindowLongPtr(hctl,GWLP_WNDPROC,(LONG_PTR)UrlCtlProc);
	}
	//
	// Second we subclass the dialog
	hold = (HGLOBAL)GetProp(hdlg,"href_dlg");
	if (hold==NULL)
	{ HGLOBAL hglob = GlobalAlloc(GMEM_MOVEABLE,sizeof(TUrlData));
	TUrlData *ud = (TUrlData*)GlobalLock(hglob);
	ud->url=0;
	ud->oldproc = (WNDPROC)GetWindowLongPtr(hdlg,GWLP_WNDPROC);
	ud->hb=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

	ud->hf=0; // the font will be created lazilly, the first time WM_CTLCOLORSTATIC gets called
	GlobalUnlock(hglob);
	SetProp(hdlg,"href_dlg",hglob);
	SetWindowLongPtr(hdlg,GWLP_WNDPROC,(LONG_PTR)UrlDlgProc);
	}
#pragma warning( pop )
}

// UrlCtlProc: this is the subclass procedure for the static control
LRESULT CALLBACK UrlCtlProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{ HGLOBAL hglob = (HGLOBAL)GetProp(hwnd,"href_dat");
if (hglob==NULL) return DefWindowProc(hwnd,msg,wParam,lParam);
TUrlData *oud=(TUrlData*)GlobalLock(hglob); TUrlData ud=*oud;
GlobalUnlock(hglob); // I made a copy of the structure just so I could GlobalUnlock it now, to be more local in my code
switch (msg)
{ case WM_DESTROY:
{ RemoveProp(hwnd,"href_dat"); GlobalFree(hglob);
if (ud.url!=0) delete[] ud.url;
// nb. remember that ud.url is just a pointer to a memory block. It might look weird
// for us to delete ud.url instead of oud->url, but they're both equivalent.
} break;
case WM_LBUTTONDOWN:
	{ HWND hdlg=GetParent(hwnd); if (hdlg==0) hdlg=hwnd;
	ShellExecute(hdlg,"open",ud.url,NULL,NULL,SW_SHOWNORMAL);
	} break;
case WM_SETCURSOR:
	{ SetCursor(LoadCursor(NULL,MAKEINTRESOURCE(IDC_HAND)));
	return TRUE;
	} 
case WM_NCHITTEST:
	{ return HTCLIENT; // because normally a static returns HTTRANSPARENT, so disabling WM_SETCURSOR
	} 
}
return CallWindowProc(ud.oldproc,hwnd,msg,wParam,lParam);
}

// UrlDlgProc: this is the subclass procedure for the dialog
LRESULT CALLBACK UrlDlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{ HGLOBAL hglob = (HGLOBAL)GetProp(hwnd,"href_dlg");
if (hglob==NULL) return DefWindowProc(hwnd,msg,wParam,lParam);
TUrlData *oud=(TUrlData*)GlobalLock(hglob); TUrlData ud=*oud;
GlobalUnlock(hglob);
switch (msg)
{ case WM_DESTROY:
{ RemoveProp(hwnd,"href_dlg"); GlobalFree(hglob);
if (ud.hb!=0) DeleteObject(ud.hb);
if (ud.hf!=0) DeleteObject(ud.hf);
} break;
case WM_CTLCOLORSTATIC:
	{ HDC hdc=(HDC)wParam; HWND hctl=(HWND)lParam;
	// To check whether to handle this control, we look for its subclassed property!
	HANDLE hprop=GetProp(hctl,"href_dat");
	if (hprop==NULL) return CallWindowProc(ud.oldproc,hwnd,msg,wParam,lParam);
	// There has been a lot of faulty discussion in the newsgroups about how to change
	// the text colour of a static control. Lots of people mess around with the
	// TRANSPARENT text mode. That is incorrect. The correct solution is here:
	// (1) Leave the text opaque. This will allow us to re-SetDlgItemText without it looking wrong.
	// (2) SetBkColor. This background colour will be used underneath each character cell.
	// (3) return HBRUSH. This background colour will be used where there's no text.
	SetTextColor(hdc,RGB(0,0,255));
	SetBkColor(hdc,GetSysColor(COLOR_BTNFACE));
	if (ud.hf==0)
	{ // we use lazy creation of the font. That's so we can see font was currently being used.
		TEXTMETRIC tm; GetTextMetrics(hdc,&tm);
		LOGFONT lf;
		lf.lfHeight=tm.tmHeight;
		lf.lfWidth=0;
		lf.lfEscapement=0;
		lf.lfOrientation=0;
		lf.lfWeight=tm.tmWeight;
		lf.lfItalic=tm.tmItalic;
		lf.lfUnderline=TRUE;
		lf.lfStrikeOut=tm.tmStruckOut;
		lf.lfCharSet=tm.tmCharSet;
		lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
		lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;
		lf.lfQuality=DEFAULT_QUALITY;
		lf.lfPitchAndFamily=tm.tmPitchAndFamily;
		GetTextFace(hdc,LF_FACESIZE,lf.lfFaceName);
		ud.hf=CreateFontIndirect(&lf);
		TUrlData *oud = (TUrlData*)GlobalLock(hglob); oud->hf=ud.hf; GlobalUnlock(hglob);
	}
	SelectObject(hdc,ud.hf);
	// Note: the win32 docs say to return an HBRUSH, typecast as a BOOL. But they
	// fail to explain how this will work in 64bit windows where an HBRUSH is 64bit.
	// I have supressed the warnings for now, because I hate them...
#pragma warning( push )
#pragma warning( disable: 4311 )
	return (BOOL)ud.hb;
#pragma warning( pop )
	}  
}
return CallWindowProc(ud.oldproc,hwnd,msg,wParam,lParam);
}

