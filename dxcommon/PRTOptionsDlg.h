//--------------------------------------------------------------------------------------
/// File: PRTOptionsDlg.cpp
//
/// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once

//--------------------------------------------------------------------------------------
class CPRTOptionsDlg
{
public:
    CPRTOptionsDlg(void);
    ~CPRTOptionsDlg(void);

    SIMULATOR_OPTIONS* GetOptions();
    HRESULT LoadOptions( WCHAR* strFile = NULL );
    HRESULT SaveOptions( WCHAR* strFile = NULL );
    void ResetSettings();

    bool Show(HWND hwnd);

public:
    HWND m_hDlg;
    HWND  m_hToolTip;
    HHOOK m_hMsgProcHook;
    WCHAR m_strExePath[MAX_PATH];
    BOOL m_bShowTooltips;
    bool m_bComboBoxSelChange;

    static INT_PTR CALLBACK StaticDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    INT_PTR CALLBACK DlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );

	bool GetSettings(); /// TIM: Updates SIMULATOR_OPTIONS
    void GetToolTipText( int nDlgId, NMTTDISPINFO* pNMTDI );

    static LRESULT CALLBACK GetMsgProc( int nCode, WPARAM wParam, LPARAM lParam );
    static BOOL CALLBACK EnumChildProc( HWND hwnd, LPARAM lParam );

    void UpdateControlsWithSettings(SIMULATOR_OPTIONS* pOptions,  HWND hDlg );

};

class CPRTLoadDlg
{
public:
    CPRTLoadDlg(void);
    ~CPRTLoadDlg(void);

    SIMULATOR_OPTIONS* GetOptions();

    bool Show();

protected:
    HWND  m_hDlg;
    HWND  m_hToolTip;
    HHOOK m_hMsgProcHook;

    static INT_PTR CALLBACK StaticDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    INT_PTR CALLBACK DlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );

    void GetToolTipText( int nDlgId, NMTTDISPINFO* pNMTDI );

    static LRESULT CALLBACK GetMsgProc( int nCode, WPARAM wParam, LPARAM lParam );
    static BOOL CALLBACK EnumChildProc( HWND hwnd, LPARAM lParam );
};

class CPRTAdaptiveOptionsDlg
{
public:
    CPRTAdaptiveOptionsDlg(void);
    ~CPRTAdaptiveOptionsDlg(void);

    bool Show( HWND hDlg );

protected:
    HWND  m_hDlg;
    HWND  m_hToolTip;
    HHOOK m_hMsgProcHook;

    static INT_PTR CALLBACK StaticDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    INT_PTR CALLBACK DlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );

    void UpdateUI( HWND hDlg );
    void GetToolTipText( int nDlgId, NMTTDISPINFO* pNMTDI );

    static LRESULT CALLBACK GetMsgProc( int nCode, WPARAM wParam, LPARAM lParam );
    static BOOL CALLBACK EnumChildProc( HWND hwnd, LPARAM lParam );
};

class COptionsFile
{
public:
    COptionsFile();
    ~COptionsFile();

    WCHAR m_strFile[MAX_PATH];
    SIMULATOR_OPTIONS m_Options;

    HRESULT LoadOptions( WCHAR* strFile = NULL );
    HRESULT SaveOptions( WCHAR* strFile = NULL  );
    void ResetSettings();
};

SIMULATOR_OPTIONS& GetGlobalOptions();
COptionsFile& GetGlobalOptionsFile();
extern COptionsFile g_OptionsFile;;


class CXMLHelper
{
public:
    static void CreateNewValue( IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, WCHAR* strName, WCHAR* strValue );
    static void CreateNewValue( IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, WCHAR* strName, DWORD nValue );
    static void CreateNewValue( IXMLDOMDocument* pDoc, IXMLDOMNode* pNode, WCHAR* strName, float fValue );
    static void CreateChildNode( IXMLDOMDocument* pDoc, IXMLDOMNode* pParentNode, WCHAR* strName, int nType, IXMLDOMNode** ppNewNode );
    static void GetValue( IXMLDOMNode* &pNode, WCHAR* strName, WCHAR* strValue );
    static void GetValue( IXMLDOMNode* &pNode, WCHAR* strName, int* pnValue );
    static void GetValue( IXMLDOMNode* &pNode, WCHAR* strName, bool* pbValue );
    static void GetValue( IXMLDOMNode* &pNode, WCHAR* strName, float* pfValue );
    static void GetValue( IXMLDOMNode* &pNode, WCHAR* strName, D3DXCOLOR* pclrValue );
    static void GetValue( IXMLDOMNode* &pNode, WCHAR* strName, DWORD* pdwValue );
};




