//----------------------------------------------------------------------------------
// XImport.cpp - Reads modified X files and converts the data
// to C++ structures for use in Compiler.cpp
//
// TODO: Mixmap support
//----------------------------------------------------------------------------------
#include "stdafx.h"
#include <xercesc/util/PlatformUtils.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMWriter.hpp>

#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>

#include "DOMTreeErrorReporter.hpp"
#include "DOMPrintFilter.hpp"
#include "DOMPrintErrorHandler.hpp"
#include "XMLImport.h"

XercesDOMParser * LoadFile(char* name);

string GetNodeName(DOMNode* pCurrent){
	char* strValue = XMLString::transcode(pCurrent->getNodeName());
	string str = strValue;
	XMLString::release(&strValue);
	return str;
}

string GetValue(DOMNode* pCurrent){
	char* strValue = XMLString::transcode(pCurrent->getNodeValue());
	string str = strValue;
	XMLString::release(&strValue);
	return str;
}


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
void XMLSystem::ParseNode(DOMNode* pCurrent)
{
	int type = pCurrent->getNodeType();
	switch (type) {
		case DOMNode::ELEMENT_NODE:
			{
			OutputDebugString(GetNodeName(pCurrent).c_str());

			DOMNamedNodeMap* attrs = pCurrent->getAttributes();
			int len = attrs->getLength();
			for (int i=0; i<len; i++) {
				DOMAttr* attr = (DOMAttr*)attrs->item(i);
				OutputDebugString(GetNodeName(attr).c_str());
				OutputDebugString(GetValue(attr).c_str());
			}

			DOMNodeList* children = pCurrent->getChildNodes();
			len = children->getLength();
			for (int i=0; i<len; i++)
				ParseNode(children->item(i));

			break;
			}
		case DOMNode::ENTITY_REFERENCE_NODE:
			//out.print("&" + node.getNodeName() + ";");
			break;
		case DOMNode::CDATA_SECTION_NODE:
			//out.print("<![CDATA[" + node.getNodeValue() + "]]>");
			break;
		case DOMNode::TEXT_NODE:
			//out.print(escapeXML(node.getNodeValue()));
			break;
		case DOMNode::PROCESSING_INSTRUCTION_NODE:
			/*out.print("<?" + node.getNodeName());
			String data = node.getNodeValue();
			if (data!=null && data.length()>0)
				out.print(" " + data);
			out.println("?>");*/
			break;
	}
}


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool XMLSystem::Load(string filename)
{
	m_Parser = LoadFile((char*)filename.c_str());

	//DOMNode         *doc = m_Parser->getDocument();
	//DOMDocument *testdoc = m_Parser->getDocument(); 
	DOMNode *pRoot = m_Parser->getDocument()->getDocumentElement();

	DOMNode* pCurrent = NULL;
	DOMTreeWalker* walker = m_Parser->getDocument()->createTreeWalker(pRoot, DOMNodeFilter::SHOW_ALL, NULL, true);

	// use the tree walker to print out the text nodes.
	for ( pCurrent = walker->nextNode(); pCurrent != 0; pCurrent = walker->nextNode() )
	{
		ParseNode(pCurrent);
	}


	return true;
}