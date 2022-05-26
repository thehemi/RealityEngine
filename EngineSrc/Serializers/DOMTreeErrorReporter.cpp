/*
 * $Id: DOMTreeErrorReporter.cpp,v 1.1 2004/08/12 10:13:52 cvsuser Exp $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/sax/SAXParseException.hpp>
#include "stdafx.h"
#include "DOMTreeErrorReporter.hpp"
#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif
#include <stdlib.h>
#include <string.h>


void DOMTreeErrorReporter::warning(const SAXParseException&)
{
    //
    // Ignore all warnings.
    //
}

void DOMTreeErrorReporter::error(const SAXParseException& toCatch)
{
    fSawErrors = true;
	LogPrintf("Error at file \"%s\", line %d, column %d, Message: \"%s\"",StrX(toCatch.getSystemId()).localForm(),
		toCatch.getLineNumber(),
		toCatch.getColumnNumber(),
		StrX(toCatch.getMessage()).localForm()
		);

}

void DOMTreeErrorReporter::fatalError(const SAXParseException& toCatch)
{
    fSawErrors = true;
	LogPrintf("Error at file \"%s\", line %d, column %d, Message: \"%s\"",StrX(toCatch.getSystemId()).localForm(),
		toCatch.getLineNumber(),
		toCatch.getColumnNumber(),
		StrX(toCatch.getMessage()).localForm()
		);
}

void DOMTreeErrorReporter::resetErrors()
{
    fSawErrors = false;
}



