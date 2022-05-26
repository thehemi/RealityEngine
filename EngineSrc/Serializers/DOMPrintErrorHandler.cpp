/*
 * $Id: DOMPrintErrorHandler.cpp,v 1.1 2004/08/12 10:13:52 cvsuser Exp $
 * $Log: DOMPrintErrorHandler.cpp,v $
 * Revision 1.1  2004/08/12 10:13:52  cvsuser
 * *** empty log message ***
 *
 * Revision 1.6  2003/05/30 09:36:35  gareth
 * Use new macros for iostream.h and std:: issues.
 *
 * Revision 1.5  2003/02/05 18:53:22  tng
 * [Bug 11915] Utility for freeing memory.
 *
 * Revision 1.4  2002/12/10 15:36:36  tng
 * DOMPrint minor update: print error message to XERCES_STD_QUALIFIER cerr.
 *
 * Revision 1.3  2002/06/13 14:55:01  peiyongz
 * Fix to UNIX compilation failure
 *
 * Revision 1.2  2002/06/11 19:46:28  peiyongz
 * Display error message received from the serializer.
 *
 * Revision 1.1  2002/05/29 21:19:50  peiyongz
 * DOM3 DOMWriter/DOMWriterFilter
 *
 *
 */
#include <stdafx.h>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMError.hpp>
#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif

#include "DOMPrintErrorHandler.hpp"

bool DOMPrintErrorHandler::handleError(const DOMError &domError)
{
    // Display whatever error message passed from the serializer
    if (domError.getSeverity() == DOMError::DOM_SEVERITY_WARNING)
        XERCES_STD_QUALIFIER cerr << "\nWarning Message: ";
    else if (domError.getSeverity() == DOMError::DOM_SEVERITY_ERROR)
        XERCES_STD_QUALIFIER cerr << "\nError Message: ";
    else
        XERCES_STD_QUALIFIER cerr << "\nFatal Message: ";

    char *msg = XMLString::transcode(domError.getMessage());
    XERCES_STD_QUALIFIER cerr<< msg <<XERCES_STD_QUALIFIER endl;
    XMLString::release(&msg);

	LogPrintf("Error saving file: %s",msg);

    // Instructs the serializer to continue serialization if possible.
    return true;
}

