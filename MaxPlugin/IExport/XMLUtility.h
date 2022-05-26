/**********************************************************************
 *<
	FILE: XMLUtility.h

	DESCRIPTION:	XML utilites

	CREATED BY:		Neil Hazzard	

	HISTORY:		summer 2002

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
#include "stdafx.h"
#include "atlbase.h"
#include "msxml2.h"


//BOOL InitialiseXML(IXMLDOMNode ** root, IXMLDOMDocument ** doc);
BOOL CreateXMLNode(IXMLDOMDocument * doc, IXMLDOMNode * node, TCHAR *nodeName, IXMLDOMNode ** newNode);

BOOL AddXMLText(IXMLDOMDocument * doc, IXMLDOMNode * node, TCHAR * text);

BOOL AddXMLAttribute(IXMLDOMNode * node, TCHAR * name, const TCHAR * value);
BOOL AddXMLAttribute(IXMLDOMNode * node, TCHAR * name, string value);
BOOL AddXMLAttribute(IXMLDOMNode * node, TCHAR * name, DWORD value);
BOOL AddXMLAttribute(IXMLDOMNode * node, TCHAR * name, int value);
BOOL AddXMLAttribute(IXMLDOMNode * node, TCHAR * name, bool value);
BOOL AddXMLAttribute(IXMLDOMNode * node, TCHAR * name, Vector2 value);
BOOL AddXMLAttribute(IXMLDOMNode * node, TCHAR * name, Vector value);
BOOL AddXMLAttribute(IXMLDOMNode * node, TCHAR * name, Point3 value);
BOOL AddXMLAttribute(IXMLDOMNode * node, TCHAR * name, Point2 value);
BOOL AddXMLAttribute(IXMLDOMNode * node, TCHAR * name, Vector4 value);
BOOL AddXMLAttribute(IXMLDOMNode * node, TCHAR * name, float value);

