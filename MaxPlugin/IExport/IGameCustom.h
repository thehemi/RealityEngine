//-----------------------------------------------------------------------------
// Macros to make using XML easier
//-----------------------------------------------------------------------------
#define CreateNode(name, parent,y) CComPtr <IXMLDOMNode> name; CreateXMLNode(pXMLDoc,parent,_T(y),&name);
#define Node(parent,y) tempNode = NULL; CreateXMLNode(pXMLDoc,parent,_T(y),&tempNode);
#define Attrib(name,data) AddXMLAttribute(tempNode,_T(name),data);
#define AttribTo(node,name,data) AddXMLAttribute(node,_T(name),data);