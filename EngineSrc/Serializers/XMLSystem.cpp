//=============================================================================
// XMLSystem	- XML Wrapper for file loading/saving
//
//
//=============================================================================
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
#include "XMLSystem.h"

// ---------------------------------------------------------------------------
//  This is a simple class that lets us do easy (though not terribly efficient)
//  trancoding of char* data to XMLCh data.
// ---------------------------------------------------------------------------
class XStr
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    XStr(const char* const toTranscode)
    {
        // Call the private transcoding method
        fUnicodeForm = XMLString::transcode(toTranscode);
    }

    ~XStr()
    {
        XMLString::release(&fUnicodeForm);
    }


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const XMLCh* unicodeForm() const
    {
        return fUnicodeForm;
    }

private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fUnicodeForm
    //      This is the Unicode XMLCh format of the string.
    // -----------------------------------------------------------------------
    XMLCh*   fUnicodeForm;
};

#define X(str) XStr(str).unicodeForm()

//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
DOMElement* XMLSystem::FindFirstNode(DOMElement* pNode, string name){
	DOMNodeList*	pChildren =  pNode->getChildNodes();
	DOMElement*		pChild	  = NULL;
	for(int i=0;i<pChildren->getLength();i++){
		if(GetName(pChildren->item(i)) == name)
			return (DOMElement*)pChildren->item(i);
	} 
	return NULL;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void XMLSystem::Destroy(){
	//  Clean up the error handler. The m_Parser does not adopt handlers
	//  since they could be many objects or one object installed for multiple
	//  handlers.
	delete m_ErrReporter;
	//  Delete the m_Parser itself.  Must be done prior to calling Terminate, below.
	delete m_Parser;
	// And call the termination method
	XMLPlatformUtils::Terminate();
	XMLString::release(&m_OutputEncoding);
}


//-----------------------------------------------------------------------------
// Creates a file
//-----------------------------------------------------------------------------
xercesc_2_5::DOMDocument* XMLSystem::CreateDocument(string name){
	XMLPlatformUtils::Initialize();
	DOMImplementation* impl =  DOMImplementationRegistry::getDOMImplementation(X("Core"));
	if(impl == NULL)
		Error("getDOMImplementation() failed!");


	xercesc_2_5::DOMDocument* doc = impl->createDocument(
		0,                    // root element namespace URI.
		X(name.c_str()),           // root element name
		0);                   // document type object (DTD).

	if(!doc)
		Error("createDocument() failed!");

	m_Document = doc;
	m_LastNodeCreated = doc->getDocumentElement();
	return doc;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
bool XMLSystem::Load(string name)
{
	int retval = 0;

	// Initialize the XML4C2 system
	try
	{
		XMLPlatformUtils::Initialize();
	}

	catch(const XMLException &toCatch)
	{
		XERCES_STD_QUALIFIER cerr << "Error during Xerces-c Initialization.\n"
			<< "  Exception message:"
			<< StrX(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
		return 0;
	}

	m_XmlFile = name.c_str();

	//
	//  Create our m_Parser, then attach an error handler to the m_Parser.
	//  The m_Parser will call back to methods of the ErrorHandler if it
	//  discovers errors during the course of parsing the XML document.
	//
	m_Parser = new XercesDOMParser;
	m_Parser->setValidationScheme(XercesDOMParser::Val_Never);//Auto);
	m_Parser->setDoNamespaces(m_DoNamespaces);
	m_Parser->setDoSchema(m_DoSchema);
	m_Parser->setValidationSchemaFullChecking(m_SchemaFullChecking);
	m_Parser->setCreateEntityReferenceNodes(m_DoCreate);

	m_ErrReporter = new DOMTreeErrorReporter();
	m_Parser->setErrorHandler(m_ErrReporter);

	//
	//  Parse the XML file, catching any XML exceptions that might propogate
	//  out of it.
	//
	bool errorsOccured = false;
	try
	{
		m_Parser->parse(m_XmlFile.c_str());
	}

	catch (const XMLException& e)
	{
		XERCES_STD_QUALIFIER cerr << "An error occurred during parsing\n   Message: "
			<< StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
		errorsOccured = true;
	}

	catch (const DOMException& e)
	{
		const unsigned int maxChars = 2047;
		XMLCh errText[maxChars + 1];

		XERCES_STD_QUALIFIER cerr << "\nDOM Error during parsing: '" << m_XmlFile << "'\n"
			<< "DOMException code is:  " << e.code << XERCES_STD_QUALIFIER endl;

		if (DOMImplementation::loadDOMExceptionMsg(e.code, errText, maxChars))
			XERCES_STD_QUALIFIER cerr << "Message is: " << StrX(errText) << XERCES_STD_QUALIFIER endl;

		errorsOccured = true;
	}

	catch (...)
	{
		XERCES_STD_QUALIFIER cerr << "An error occurred during parsing\n " << XERCES_STD_QUALIFIER endl;
		errorsOccured = true;
	}

	bool errors = (errorsOccured || m_ErrReporter->getSawErrors());
	if(errors){
		Destroy();
	}
	return !errors;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
bool XMLSystem::Save(string name){
	DOMPrintFilter   *myFilter = 0;

	try
	{
		// get a serializer, an instance of DOMWriter
		XMLCh tempStr[100];
		XMLString::transcode("LS", tempStr, 99);
		DOMImplementation *impl          = DOMImplementationRegistry::getDOMImplementation(tempStr);
		DOMWriter         *theSerializer = ((DOMImplementationLS*)impl)->createDOMWriter();

		// set user specified output encoding
		theSerializer->setEncoding(m_OutputEncoding);

		// plug in user's own filter
		if (m_UseFilter)
		{
			// even we say to show attribute, but the DOMWriter
			// will not show attribute nodes to the filter as
			// the specs explicitly says that DOMWriter shall
			// NOT show attributes to DOMWriterFilter.
			//
			// so DOMNodeFilter::SHOW_ATTRIBUTE has no effect.
			// same DOMNodeFilter::SHOW_DOCUMENT_TYPE, no effect.
			//
			myFilter = new DOMPrintFilter(DOMNodeFilter::SHOW_ELEMENT   |
				DOMNodeFilter::SHOW_ATTRIBUTE |
				DOMNodeFilter::SHOW_DOCUMENT_TYPE);
			theSerializer->setFilter(myFilter);
		}

		// plug in user's own error handler
		DOMErrorHandler *myErrorHandler = new DOMPrintErrorHandler();
		theSerializer->setErrorHandler(myErrorHandler);

		// set feature if the serializer supports the feature/mode
		if (theSerializer->canSetFeature(XMLUni::fgDOMWRTSplitCdataSections, m_SplitCdataSections))
			theSerializer->setFeature(XMLUni::fgDOMWRTSplitCdataSections, m_SplitCdataSections);

		if (theSerializer->canSetFeature(XMLUni::fgDOMWRTDiscardDefaultContent, m_DiscardDefaultContent))
			theSerializer->setFeature(XMLUni::fgDOMWRTDiscardDefaultContent, m_DiscardDefaultContent);

		if (theSerializer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, m_FormatPrettyPrint))
			theSerializer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, m_FormatPrettyPrint);

		if (theSerializer->canSetFeature(XMLUni::fgDOMWRTBOM, m_WriteBOM))
			theSerializer->setFeature(XMLUni::fgDOMWRTBOM, m_WriteBOM);

		//
		// Plug in a format target to receive the resultant
		// XML stream from the serializer.
		//
		// StdOutFormatTarget prints the resultant XML stream
		// to stdout once it receives any thing from the serializer.
		//
		XMLFormatTarget *myFormTarget;
		myFormTarget = new LocalFileFormatTarget(name.c_str());

		//
		// do the serialization through DOMWriter::writeNode();
		//
		theSerializer->writeNode(myFormTarget, *m_Document);

		delete theSerializer;

		//
		// Filter, formatTarget and error handler
		// are NOT owned by the serializer.
		//
		delete myFormTarget;
		delete myErrorHandler;

		if (m_UseFilter)
			delete myFilter;

	}
	catch (XMLException& e)
	{
		XERCES_STD_QUALIFIER cerr << "An error occurred during creation of output transcoder. Msg is:"
			<< XERCES_STD_QUALIFIER endl
			<< StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
		return false;
	}

	// Put dir back to correct place
	ResetCurrentDirectory();
	return true;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
string XMLSystem::GetName(DOMNode* pCurrent){
	char* strValue = XMLString::transcode(pCurrent->getNodeName());
	string str = strValue;
	XMLString::release(&strValue);
	return str;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
string XMLSystem::GetString(DOMNode* pCurrent, bool textNode){
	assert(pCurrent);
    if(!pCurrent)
        return "";
	char* strValue;

	if(textNode)
		strValue = XMLString::transcode(pCurrent->getTextContent());
	else
		strValue = XMLString::transcode(pCurrent->getNodeValue());
	string str = strValue;
	XMLString::release(&strValue);
	return str;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
DWORD XMLSystem::GetColor(DOMNode* pCurrent, bool textNode){
	string value = GetString(pCurrent,textNode);
	if(GetWord(value,4).length())
		return COLOR_RGBA(atoi(GetWord(value,0).c_str())*255,atoi(GetWord(value,1).c_str())*255,atoi(GetWord(value,2).c_str())*255,atoi(GetWord(value,3).c_str())*255);
	else
		return COLOR_RGBA(atoi(GetWord(value,0).c_str())*255,atoi(GetWord(value,1).c_str())*255,atoi(GetWord(value,2).c_str())*255,1*255);
}


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
FloatColor XMLSystem::GetFloatColor(DOMNode* pCurrent, bool textNode){
	string value = GetString(pCurrent,textNode);

    string w1 = GetWord(value,0);
    string w2 = GetWord(value,1);
    string w3 = GetWord(value,2);

	return FloatColor(atof(w1.c_str()),atof(w2.c_str()),atof(w3.c_str()),1);
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
int XMLSystem::GetInt(DOMNode* pCurrent, bool textNode){
	string value = GetString(pCurrent,textNode);
	char* s;
	errno = 0;
	int number = strtol(value.c_str(),&s,10);
	if(s == value.c_str()) { // Something went wrong!
		Error("XMLSystem::GetInt: The value for the node was invalid: '%s'",value.c_str());
	}
	return number;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
SYSTEMTIME XMLSystem::GetTime(DOMNode* pCurrent, bool textNode){
	string value = GetString(pCurrent,textNode);
	SYSTEMTIME time;

	vector<string> t;
	for(int i=0;i<8;i++)
		t.push_back(GetWord(value,i));

	char *err1 = 0;
	time.wYear		= strtod(t[0].c_str(),&err1);
	time.wMonth		= strtod(t[1].c_str(),&err1);
	time.wDayOfWeek = strtod(t[2].c_str(),&err1);
	time.wDay		= strtod(t[3].c_str(),&err1);
	time.wHour		= strtod(t[4].c_str(),&err1);
	time.wMinute	= strtod(t[5].c_str(),&err1);
	time.wSecond	= strtod(t[6].c_str(),&err1);
	time.wMilliseconds = strtod(t[7].c_str(),&err1);

	return time;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
float XMLSystem::GetFloat(DOMNode* pCurrent, bool textNode){
	string value = GetString(pCurrent,textNode);
	char* s;
	errno = 0;
	float number = strtod(value.c_str(),&s);
	if(s == value.c_str()) { // Something went wrong!
		Error("XMLSystem::GetInt: The value for the node was invalid: '%s'",value.c_str());
	}
	return number;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
bool XMLSystem::GetBool(DOMNode* pCurrent, bool textNode){
	string value = GetString(pCurrent,textNode);
	if(value == "true"||value=="1")
		return true;
	if(value == "false"||value=="0")
		return false;

	// Something went wrong!
	Error("XMLSystem::GetBool: The value for the node was invalid: '%s'",value.c_str());
	return false;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector XMLSystem::GetVector(DOMNode* pCurrent, bool textNode){
	string value = GetString(pCurrent,textNode);

	// strtod puts errror codes in these if something goes wrong, like trying to convert alpha
	char *err1 = 0,*err2 = 0,*err3 = 0;

	// Format: x y z
	Vector thevector;
	// GetWord gets the nTh word from the string, words are ascii separated by spaces
	string v1 = GetWord(value,0).c_str();
	string v2 = GetWord(value,1).c_str();
	string v3 = GetWord(value,2).c_str();
	thevector.x = strtod(v1.c_str(),&err1);
	thevector.y = strtod(v2.c_str(),&err2);
	thevector.z = strtod(v3.c_str(),&err3);

	if (err1 == v1.c_str() || err2 == v1.c_str() || err3 == v1.c_str()){ // Something went wrong!
		Warning("XMLSystem::GetVector: The value for the node was invalid: '%s'",value.c_str());
		thevector.Set(0,0,0);
	}

	return thevector;
}


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector2 XMLSystem::GetVector2(DOMNode* pCurrent, bool textNode){
	string value = GetString(pCurrent,textNode);

	// strtod puts errror codes in these if something goes wrong, like trying to convert alpha
	char *err1 = 0,*err2 = 0,*err3 = 0;

	// Format: x y z
	Vector2 thevector;
	// GetWord gets the nTh word from the string, words are ascii separated by spaces
	string v1 = GetWord(value,0).c_str();
	string v2 = GetWord(value,1).c_str();
	thevector.x = strtod(v1.c_str(),&err1);
	thevector.y = strtod(v2.c_str(),&err2);

	return thevector;
}
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Matrix XMLSystem::GetMatrix(DOMNode* pCurrent, bool textNode){
	Error("GetMatrix() is not yet implemented");
	return Matrix();
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Vector4 XMLSystem::GetVector4(DOMNode* pCurrent, bool textNode){
	string value = GetString(pCurrent,textNode);

	// strtod puts errror codes in these if something goes wrong, like trying to convert alpha
	char *err1 = 0,*err2 = 0,*err3 = 0;

	// Format: x y z w
	Vector4 thevector;
	// GetWord gets the nTh word from the string, words are ascii separated by spaces
	string v1 = GetWord(value,0).c_str();
	string v2 = GetWord(value,1).c_str();
	string v3 = GetWord(value,2).c_str();
	string v4 = GetWord(value,2).c_str();
	thevector.x = strtod(v1.c_str(),&err1);
	thevector.y = strtod(v2.c_str(),&err2);
	thevector.z = strtod(v3.c_str(),&err3);
	thevector.w = strtod(v4.c_str(),NULL);

	if (err1 == v1.c_str() || err2 == v1.c_str() || err3 == v1.c_str()){ // Something went wrong!
		Warning("XMLSystem::GetVector4: The value for the node was invalid: '%s'",value.c_str());
		//thevector.Set(0,0,0,0);
	}

	return thevector;
}

//----------------------------------------------------------------------------------
DOMElement*  XMLSystem::CreateTextNode(DOMNode* node, TCHAR * name)
{
	DOMText* e = m_Document->createTextNode(X(name));
	node->appendChild(e);
	return (DOMElement*)e;
}
//----------------------------------------------------------------------------------
DOMElement* XMLSystem::CreateNode(DOMNode* node, TCHAR * name)
{
	DOMElement* e = m_Document->createElement(X(name));
	node->appendChild(e);
	m_LastNodeCreated = e;
	return e;
}
//----------------------------------------------------------------------------------
BOOL XMLSystem::Attrib(TCHAR * name, const TCHAR * value, DOMNode* node)
{
	if(!node)
		node = m_LastNodeCreated;
	((DOMElement*)node)->setAttribute(X(name), X(value));
	return true;
}
//----------------------------------------------------------------------------------
BOOL XMLSystem::Attrib(TCHAR * name, string value, DOMNode* node)
{
	return XMLSystem::Attrib(name,value.c_str(),node);
}
//----------------------------------------------------------------------------------
BOOL XMLSystem::Attrib(TCHAR * name, DWORD value, DOMNode* node)
{
	TCHAR buf[64];
	_stprintf(buf,"%d",value);
	return XMLSystem::Attrib(name,buf,node);
}
//----------------------------------------------------------------------------------
BOOL XMLSystem::Attrib(TCHAR * name, int value, DOMNode* node)
{
	TCHAR buf[64];
	_stprintf(buf,"%d",value);
	return XMLSystem::Attrib(name,buf,node);
}
//----------------------------------------------------------------------------------
BOOL XMLSystem::Attrib(TCHAR * name, bool value, DOMNode* node)
{
	TCHAR buf[64];
	_stprintf(buf,"%s",(value?"true":"false"));
	return XMLSystem::Attrib(name,buf,node);
}
//----------------------------------------------------------------------------------
BOOL XMLSystem::Attrib(TCHAR * name, SYSTEMTIME time, DOMNode* node)
{
	TCHAR buf[128];
	_stprintf(buf,"%d %d %d %d %d %d %d %d",time.wYear,time.wMonth,time.wDayOfWeek,time.wDay,time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
	return XMLSystem::Attrib(name,buf,node);
}
//----------------------------------------------------------------------------------
BOOL XMLSystem::Attrib(TCHAR * name, float value, DOMNode* node)
{
	TCHAR buf[64];
	_stprintf(buf,"%f",value);
	return XMLSystem::Attrib(name,buf,node);
}
//----------------------------------------------------------------------------------
BOOL XMLSystem::Attrib(TCHAR * name, Vector& value, DOMNode* node)
{
	TCHAR buf[64];
	_stprintf(buf,"%f %f %f",value.x,value.y,value.z);
	return XMLSystem::Attrib(name,buf,node);
}
//----------------------------------------------------------------------------------
BOOL XMLSystem::Attrib(TCHAR * name, Vector4& value, DOMNode* node)
{
	TCHAR buf[64];
	_stprintf(buf,"%f %f %f %f",value.x,value.y,value.z,value.w);
	return XMLSystem::Attrib(name,buf,node);
}
//----------------------------------------------------------------------------------
BOOL XMLSystem::Attrib(TCHAR * name, Vector2& value, DOMNode* node)
{
	TCHAR buf[64];
	_stprintf(buf,"%f %f",value.x,value.y);
	return XMLSystem::Attrib(name,buf,node);
}

//----------------------------------------------------------------------------------
void    XMLSystem::ReadParameter(int index, DOMNode* node)
{
	string name = GetString(((DOMElement*)node)->getAttributeNode(L"Name"));
	DOMNode* e = ((DOMElement*)node)->getAttributeNode(L"Value");
    string typeS = GetString(((DOMElement*)node)->getAttributeNode(L"Type"));
    
    	if(!e){
		Warning("XMLSystem: No Value for '%s'",name.c_str());
		return;
	}
        EditorVar::Type type = EditorVar::FromString(typeS);
    char * data=0;
    bool isString=false;
    string stringValue;
	if(type == EditorVar::BOOL)
	{
        data = new char[2];
        bool value=GetBool(e);
        memcpy(data,&value,2);
	}
	else if(type == EditorVar::INT)
	{
        data = new char[sizeof(int)];
        int value=GetInt(e);
		memcpy(data,&value,sizeof(int));
	}
	else if(type == EditorVar::FLOAT)
	{
		data = new char[sizeof(float)];
        float value=GetFloat(e);
		 memcpy(data,&value,sizeof(float));
	}
	else if(type == EditorVar::FLOAT2)
	{
		//(Vector2*)data = &GetVector2(e);
		//bytes = sizeof(Vector2);
	}
	else if(type == EditorVar::FLOAT3)
	{
        data = new char[sizeof(Vector)];
        Vector value=GetVector(e);
		 memcpy(data,&value,sizeof(Vector));
	}
	else if(type == EditorVar::FLOAT4)
	{
	//	(Vector4*)data = &GetVector4(e);
		//bytes = sizeof(Vector4);
	}
	else if(type == EditorVar::STRING)
	{
        stringValue= GetString(e);
		data =(char*)stringValue.c_str();
        isString=true;
	}
	else if(type == EditorVar::FILENAME)
	{
		//(string*)data = &GetString(e);
		//bytes = (*(string*)data).length()+2;
	}
	else if(type == EditorVar::COLOR)
	{
		//(FloatColor*)data = &GetFloatColor(e);
		//bytes = sizeof(FloatColor);
	}
	else if(type == EditorVar::TEXTURE)
	{
		//Texture* t = (Texture*)data;
	//	t->filename = GetString(e);
		//bytes = sizeof(Texture);
	}

    if(e)
        SSystem_ActorDeserialize(index,data,name.c_str(),typeS.c_str());
    if (data && !isString)
       delete data;
}
//----------------------------------------------------------------------------------
bool	XMLSystem::ReadParameter(EditorVar::Type& type, BYTE* data, DOMNode* node, int& bytes, string& name)
{
	name = GetString(((DOMElement*)node)->getAttributeNode(L"Name"));
	DOMNode* e = ((DOMElement*)node)->getAttributeNode(L"Value");
	if(!e){
		Warning("XMLSystem: No Value for '%s'",name.c_str());
		return false;
	}
	type = EditorVar::FromString(GetString(((DOMElement*)node)->getAttributeNode(L"Type")));
	bytes = 0;
	if(type == EditorVar::BOOL)
	{
		*(bool*)data = GetBool(e);
		bytes = sizeof(bool);
	}
	else if(type == EditorVar::INT)
	{
		*(int*)data = GetInt(e);
		bytes = sizeof(int);
	}
	else if(type == EditorVar::FLOAT)
	{
		*(float*)data = GetFloat(e);
		bytes = sizeof(float);
	}
	else if(type == EditorVar::FLOAT2)
	{
		*(Vector2*)data = GetVector2(e);
		bytes = sizeof(Vector2);
	}
	else if(type == EditorVar::FLOAT3)
	{
		*(Vector*)data = GetVector(e);
		bytes = sizeof(Vector);
	}
	else if(type == EditorVar::FLOAT4)
	{
		*(Vector4*)data = GetVector4(e);
		bytes = sizeof(Vector4);
	}
	else if(type == EditorVar::STRING)
	{
		*(string*)data = GetString(e);
		bytes = (*(string*)data).length()+2;
	}
	else if(type == EditorVar::FILENAME)
	{
		*(string*)data = GetString(e);
		bytes = (*(string*)data).length()+2;
	}
	else if(type == EditorVar::COLOR)
	{
		*(FloatColor*)data = GetFloatColor(e);
		bytes = sizeof(FloatColor);
	}
	else if(type == EditorVar::TEXTURE)
	{
		Texture* t = (Texture*)data;
		t->filename = GetString(e);
		bytes = sizeof(Texture);
	}

	return true;
}

//----------------------------------------------------------------------------------
void XMLSystem::WriteParameter(TCHAR* name, EditorVar::Type type, BYTE* data, DOMNode* node)
{
	node = CreateNode(node,"Param");
	Attrib("Name",name,node);
	Attrib("Type",EditorVar::ToString(type));
	if(type == EditorVar::BOOL)
	{
		Attrib("Value",*(bool*)data,node);
	}
	else if(type == EditorVar::INT)
	{
		Attrib("Value",*(int*)data,node);
	}
	else if(type == EditorVar::FLOAT)
	{
		Attrib("Value",*(float*)data,node);
	}
	else if(type == EditorVar::FLOAT2)
	{
		Attrib("Value",*(Vector2*)data,node);
	}
	else if(type == EditorVar::FLOAT3)
	{
		Attrib("Value",*(Vector*)data,node);
	}
	else if(type == EditorVar::FLOAT4)
	{
		Attrib("Value",*(Vector4*)data,node);
	}
	else if(type == EditorVar::STRING)
	{
		Attrib("Value",*(string*)data,node);
	}
	else if(type == EditorVar::FILENAME)
	{
		Attrib("Value",*(string*)data,node);
	}
	else if(type == EditorVar::COLOR)
	{
		Attrib("Value",*(Vector4*)data,node);
	}
	else if(type == EditorVar::TEXTURE)
	{
		Attrib("Value",((Texture*)data)->filename,node);
	}
}
	

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
DOMTreeWalker* XMLSystem::GetWalker()
{
	DOMNode *pRoot = m_Parser->getDocument()->getDocumentElement();
	DOMTreeWalker* walker = m_Parser->getDocument()->createTreeWalker(pRoot, DOMNodeFilter::SHOW_ALL, NULL, true);
	return walker;
}
