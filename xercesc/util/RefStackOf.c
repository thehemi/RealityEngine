/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 1999-2000 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Xerces" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache\@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation, and was
 * originally based on software copyright (c) 1999, International
 * Business Machines, Inc., http://www.ibm.com .  For more information
 * on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/**
 * $Log: RefStackOf.c,v $
 * Revision 1.4  2003/12/17 00:18:35  cargilld
 * Update to memory management so that the static memory manager (one used to call Initialize) is only for static data.
 *
 * Revision 1.3  2003/05/16 06:01:52  knoaman
 * Partial implementation of the configurable memory manager.
 *
 * Revision 1.2  2002/11/04 15:22:04  tng
 * C++ Namespace Support.
 *
 * Revision 1.1.1.1  2002/02/01 22:22:12  peiyongz
 * sane_include
 *
 * Revision 1.3  2000/03/02 19:54:44  roddey
 * This checkin includes many changes done while waiting for the
 * 1.1.0 code to be finished. I can't list them all here, but a list is
 * available elsewhere.
 *
 * Revision 1.2  2000/02/06 07:48:03  rahulj
 * Year 2K copyright swat.
 *
 * Revision 1.1.1.1  1999/11/09 01:05:02  twl
 * Initial checkin
 *
 * Revision 1.2  1999/11/08 20:45:13  rahul
 * Swat for adding in Product name and CVS comment log variable.
 *
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/util/RefStackOf.hpp>
#endif

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  RefStackOf: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem>
RefStackOf<TElem>::RefStackOf(const unsigned int initElems,
                              const bool adoptElems,
                              MemoryManager* const manager) :

    fVector(initElems, adoptElems, manager)
{
}

template <class TElem> RefStackOf<TElem>::~RefStackOf()
{
}


// ---------------------------------------------------------------------------
//  RefStackOf: Element management methods
// ---------------------------------------------------------------------------
template <class TElem> const TElem* RefStackOf<TElem>::
elementAt(const unsigned int index) const
{
    if (index > fVector.size())
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Stack_BadIndex, fVector.getMemoryManager());
    return fVector.elementAt(index);
}

template <class TElem> void RefStackOf<TElem>::push(TElem* const toPush)
{
    fVector.addElement(toPush);
}

template <class TElem> const TElem* RefStackOf<TElem>::peek() const
{
    const int curSize = fVector.size();
    if (curSize == 0)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::Stack_EmptyStack, fVector.getMemoryManager());

    return fVector.elementAt(curSize-1);
}

template <class TElem> TElem* RefStackOf<TElem>::pop()
{
    const int curSize = fVector.size();
    if (curSize == 0)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::Stack_EmptyStack, fVector.getMemoryManager());

    // Orphan off the element from the last slot in the vector
    return fVector.orphanElementAt(curSize-1);
}

template <class TElem> void RefStackOf<TElem>::removeAllElements()
{
    fVector.removeAllElements();
}


// ---------------------------------------------------------------------------
//  RefStackOf: Getter methods
// ---------------------------------------------------------------------------
template <class TElem> bool RefStackOf<TElem>::empty()
{
    return (fVector.size() == 0);
}

template <class TElem> unsigned int RefStackOf<TElem>::curCapacity()
{
    return fVector.curCapacity();
}

template <class TElem> unsigned int RefStackOf<TElem>::size()
{
    return fVector.size();
}




// ---------------------------------------------------------------------------
//  RefStackEnumerator: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem> RefStackEnumerator<TElem>::
RefStackEnumerator(         RefStackOf<TElem>* const    toEnum
                    , const bool                        adopt) :
    fAdopted(adopt)
    , fCurIndex(0)
    , fToEnum(toEnum)
    , fVector(&toEnum->fVector)
{
}

template <class TElem> RefStackEnumerator<TElem>::~RefStackEnumerator()
{
    if (fAdopted)
        delete fToEnum;
}


// ---------------------------------------------------------------------------
//  RefStackEnumerator: Enum interface
// ---------------------------------------------------------------------------
template <class TElem> bool RefStackEnumerator<TElem>::hasMoreElements() const
{
    if (fCurIndex >= fVector->size())
        return false;
    return true;
}

template <class TElem> TElem& RefStackEnumerator<TElem>::nextElement()
{
    return *fVector->elementAt(fCurIndex++);
}

template <class TElem> void RefStackEnumerator<TElem>::Reset()
{
    fCurIndex = 0;
}

XERCES_CPP_NAMESPACE_END
