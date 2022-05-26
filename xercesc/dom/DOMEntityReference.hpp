#ifndef DOMEntityReference_HEADER_GUARD_
#define DOMEntityReference_HEADER_GUARD_

/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2001-2002 The Apache Software Foundation.  All rights
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
 * originally based on software copyright (c) 2001, International
 * Business Machines, Inc., http://www.ibm.com .  For more information
 * on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/*
 * $Id: DOMEntityReference.hpp,v 1.6 2003/03/07 19:59:04 tng Exp $
 */


#include <xercesc/util/XercesDefs.hpp>
#include "DOMNode.hpp"

XERCES_CPP_NAMESPACE_BEGIN


/**
 * <code>DOMEntityReference</code> objects may be inserted into the structure
 * model when an entity reference is in the source document, or when the
 * user wishes to insert an entity reference. Note that character references
 * and references to predefined entities are considered to be expanded by
 * the HTML or XML processor so that characters are represented by their
 * Unicode equivalent rather than by an entity reference. Moreover, the XML
 * processor may completely expand references to entities while building the
 * structure model, instead of providing <code>DOMEntityReference</code>
 * objects. If it does provide such objects, then for a given
 * <code>DOMEntityReference</code> node, it may be that there is no
 * <code>DOMEntity</code> node representing the referenced entity. If such an
 * <code>DOMEntity</code> exists, then the subtree of the
 * <code>DOMEntityReference</code> node is in general a copy of the
 * <code>DOMEntity</code> node subtree. However, this may not be true when an
 * entity contains an unbound namespace prefix. In such a case, because the
 * namespace prefix resolution depends on where the entity reference is, the
 * descendants of the <code>DOMEntityReference</code> node may be bound to
 * different namespace URIs.
 * <p>As for <code>DOMEntity</code> nodes, <code>DOMEntityReference</code> nodes and
 * all their descendants are readonly.
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113'>Document Object Model (DOM) Level 2 Core Specification</a>.
 *
 * @since DOM Level 1
 */

class CDOM_EXPORT DOMEntityReference: public DOMNode {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMEntityReference() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMEntityReference(const DOMEntityReference &);
    DOMEntityReference & operator = (const DOMEntityReference &);
    //@}

public:
    // -----------------------------------------------------------------------
    //  All constructors are hidden, just the destructor is available
    // -----------------------------------------------------------------------
    /** @name Destructor */
    //@{
    /**
     * Destructor
     *
     */
    virtual ~DOMEntityReference() {};
    //@}

};

XERCES_CPP_NAMESPACE_END

#endif


