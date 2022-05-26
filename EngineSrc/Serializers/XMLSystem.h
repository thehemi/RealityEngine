//=============================================================================
/// XMLSystem	- XML Wrapper for file loading/saving
//
//
//=============================================================================
#pragma once


typedef unsigned short XMLCh;
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>
using namespace xercesc_2_5;

//-----------------------------------------------------------------------------
/// Because I'm lazy, and in case the XML platform changes
//-----------------------------------------------------------------------------
/// Loading
//-----------------------------------------------------------------------------
#define AttribExists(x,y) (((DOMElement*)x)->getAttributeNode(L##y) != NULL)
#define ReadColor(x,y)	m_XML.GetColor(((DOMElement*)x)->getAttributeNode(L##y))
#define ReadFloatColor(x,y)	m_XML.GetFloatColor(((DOMElement*)x)->getAttributeNode(L##y))
#define ReadFloat(x,y)	m_XML.GetFloat(((DOMElement*)x)->getAttributeNode(L##y))
#define ReadString(x,y)	m_XML.GetString(((DOMElement*)x)->getAttributeNode(L##y))
#define ReadVector(x,y)	m_XML.GetVector(((DOMElement*)x)->getAttributeNode(L##y))
#define ReadVector2(x,y)	m_XML.GetVector2(((DOMElement*)x)->getAttributeNode(L##y))
#define ReadMatrix(x,y)	m_XML.GetMatrix(((DOMElement*)x)->getAttributeNode(L##y))
#define ReadVector4(x,y)m_XML.GetVector4(((DOMElement*)x)->getAttributeNode(L##y))
#define ReadInt(x,y)	m_XML.GetInt(((DOMElement*)x)->getAttributeNode(L##y))
#define ReadBool(x,y)	m_XML.GetBool(((DOMElement*)x)->getAttributeNode(L##y))
#define ReadTime(x,y)	m_XML.GetTime(((DOMElement*)x)->getAttributeNode(L##y))

/// Low-Level XML Serializer class
class XMLSystem
{
private:

	/// The path to the file to parser. Set via command line.
	string                   m_XmlFile;		
	/// Indicates whether namespace processing should be done.
	bool                     m_DoNamespaces;	
	/// Indicates whether schema processing should be done.
	bool                     m_DoSchema;		
	/// Indicates whether full schema constraint checking should be done.
	bool                     m_SchemaFullChecking;	
	/// Indicates whether entity reference nodes needs to be created or not. Default false
	bool                     m_DoCreate;		
	string                   m_outputfile;
	// options for DOMWriter's features
	/// The encoding we are to output in. It defaults to the encoding of the input XML file.
	XMLCh*					 m_OutputEncoding;	
	/// Indicates whether split-cdata-sections is to be enabled or not.
	bool                     m_SplitCdataSections;	
	/// Indicates whether default content is discarded or not.
	bool                     m_DiscardDefaultContent;
	/// Indicates if user wants to plug in the DOMPrintFilter.
	bool                     m_UseFilter;			
	bool                     m_FormatPrettyPrint;
	bool                     m_WriteBOM;
	
	class DOMTreeErrorReporter *	m_ErrReporter;

	class xercesc_2_5::DOMElement*		m_LastNodeCreated;
	class xercesc_2_5::DOMDocument*		m_Document;

public:
	class XercesDOMParser *			m_Parser;

	XMLSystem()
	{
		m_LastNodeCreated	= NULL;
		m_Document			= NULL;
		m_DoNamespaces = false;
		m_DoSchema              = false;
		m_SchemaFullChecking    = false;
		m_DoCreate              = false;
		m_OutputEncoding        = 0;
		m_SplitCdataSections    = true;
		m_DiscardDefaultContent = true;
		m_UseFilter             = false;
		m_FormatPrettyPrint     = true;
		m_WriteBOM              = false;
		//gValScheme       = XercesDOMParser::Val_Auto
		m_Parser			   = NULL;
		m_ErrReporter		   = NULL;
	}

	void Destroy();

	DOMElement*		FindFirstNode(DOMElement* pNode, string name);
	xercesc_2_5::DOMDocument*	CreateDocument(string name);		/// Creates a fresh document
	bool			Load(string filename);
	bool			Save(string name);
    class xercesc_2_5::DOMDocument*		GetDocument(){ return m_Parser->getDocument(); }
	class			DOMTreeWalker* GetWalker();

	string GetName(DOMNode* pCurrent);

	/// Get Methods
	string GetString(DOMNode* pCurrent, bool textNode = false);
	float  GetFloat(DOMNode* pCurrent, bool textNode = false);
	int	   GetInt(DOMNode* pCurrent, bool textNode = false);
	bool   GetBool(DOMNode* pCurrent, bool textNode = false);
	Vector GetVector(DOMNode* pCurrent, bool textNode = false);
	Vector2 GetVector2(DOMNode* pCurrent, bool textNode = false);
	Vector4 GetVector4(DOMNode* pCurrent, bool textNode = false);
	Matrix GetMatrix(DOMNode* pCurrent, bool textNode = false);
	DWORD  GetColor(DOMNode* pCurrent, bool textNode = false);
	FloatColor  GetFloatColor(DOMNode* pCurrent, bool textNode = false);
	SYSTEMTIME  GetTime(DOMNode* pCurrent, bool textNode = false);

    void    ReadParameter(int index, DOMNode* node);
	bool	ReadParameter(EditorVar::Type& type, BYTE* data, DOMNode* node, int& size, string& name);
	void	WriteParameter(TCHAR* name, EditorVar::Type type, BYTE* data, DOMNode* node);
	
	DOMElement* CreateNode(DOMNode* node, TCHAR * name);
	DOMElement*  CreateTextNode(DOMNode* node, TCHAR * name);
	BOOL Attrib(TCHAR * name, const TCHAR * value, DOMNode* node = NULL);
	BOOL Attrib(TCHAR * name, string value, DOMNode* node = NULL);
    BOOL Attrib(TCHAR * name, Matrix& value, DOMNode* node = NULL);
	BOOL Attrib(TCHAR * name, DWORD value, DOMNode* node = NULL);
	BOOL Attrib(TCHAR * name, int value, DOMNode* node = NULL);
	BOOL Attrib(TCHAR * name, bool value, DOMNode* node = NULL);
	BOOL Attrib(TCHAR * name, Vector2& value, DOMNode* node = NULL);
	BOOL Attrib(TCHAR * name, Vector& value, DOMNode* node = NULL);
	BOOL Attrib(TCHAR * name, Vector4& value, DOMNode* node = NULL);
	BOOL Attrib(TCHAR * name, float value, DOMNode* node = NULL);
	BOOL Attrib(TCHAR * name, SYSTEMTIME time, DOMNode* node = NULL);
	
};

XercesDOMParser * LoadFile(char* name);

