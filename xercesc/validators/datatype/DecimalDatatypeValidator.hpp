/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2001-2003 The Apache Software Foundation.  All rights
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
 * $Id: DecimalDatatypeValidator.hpp,v 1.14 2004/01/29 11:51:22 cargilld Exp $
 * $Log: DecimalDatatypeValidator.hpp,v $
 * Revision 1.14  2004/01/29 11:51:22  cargilld
 * Code cleanup changes to get rid of various compiler diagnostic messages.
 *
 * Revision 1.13  2003/12/23 21:50:36  peiyongz
 * Absorb exception thrown in getCanonicalRepresentation and return 0,
 * only validate when required
 *
 * Revision 1.11  2003/12/17 00:18:38  cargilld
 * Update to memory management so that the static memory manager (one used to call Initialize) is only for static data.
 *
 * Revision 1.10  2003/11/28 18:53:07  peiyongz
 * Support for getCanonicalRepresentation
 *
 * Revision 1.9  2003/11/12 20:32:03  peiyongz
 * Statless Grammar: ValidationContext
 *
 * Revision 1.8  2003/10/02 19:21:06  peiyongz
 * Implementation of Serialization/Deserialization
 *
 * Revision 1.7  2003/08/14 03:00:11  knoaman
 * Code refactoring to improve performance of validation.
 *
 * Revision 1.6  2003/05/22 02:10:52  knoaman
 * Default the memory manager.
 *
 * Revision 1.5  2003/05/15 18:53:26  knoaman
 * Partial implementation of the configurable memory manager.
 *
 * Revision 1.4  2002/12/18 14:17:55  gareth
 * Fix to bug #13438. When you eant a vector that calls delete[] on its members you should use RefArrayVectorOf.
 *
 * Revision 1.3  2002/11/04 14:53:28  tng
 * C++ Namespace Support.
 *
 * Revision 1.2  2002/10/17 20:49:05  peiyongz
 * Patch to Bug#13640: Getter methods not public in DecimalDatatypeValidator
 *
 * Revision 1.1.1.1  2002/02/01 22:22:41  peiyongz
 * sane_include
 *
 * Revision 1.10  2001/11/22 20:23:20  peiyongz
 * _declspec(dllimport) and inline warning C4273
 *
 * Revision 1.9  2001/10/09 20:52:43  peiyongz
 * init():removed
 *
 * Revision 1.8  2001/10/01 21:04:40  peiyongz
 * DTV Reorganization:fix to memory leak in compare() method.
 *
 * Revision 1.7  2001/10/01 16:16:38  peiyongz
 * DTV Reorganization:derived from AbstractNumericValidator
 *
 */

#if !defined(DECIMAL_DATATYPEVALIDATOR_HPP)
#define DECIMAL_DATATYPEVALIDATOR_HPP

#include <xercesc/validators/datatype/AbstractNumericValidator.hpp>
#include <xercesc/util/RefVectorOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLBigDecimal;

class VALIDATORS_EXPORT DecimalDatatypeValidator : public AbstractNumericValidator
{
public:

    // -----------------------------------------------------------------------
    //  Public ctor/dtor
    // -----------------------------------------------------------------------
	/** @name Constructors and Destructor */
    //@{

    DecimalDatatypeValidator
    (
        MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    DecimalDatatypeValidator
    (
        DatatypeValidator* const baseValidator
        , RefHashTableOf<KVStringPair>* const facets
        , RefArrayVectorOf<XMLCh>* const enums
        , const int finalSet
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    virtual ~DecimalDatatypeValidator();

	//@}

    // -----------------------------------------------------------------------
    // Compare methods
    // -----------------------------------------------------------------------
    /** @name Compare Function */
    //@{

    /**
     * Compare two boolean data types
     *
     * @param content1
     * @param content2
     * @return
     */
    virtual int compare(const XMLCh* const, const XMLCh* const
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
        );

    //@}

    /**
      * Returns an instance of the base datatype validator class
	  * Used by the DatatypeValidatorFactory.
      */
    virtual DatatypeValidator* newInstance
    (
        RefHashTableOf<KVStringPair>* const facets
        , RefArrayVectorOf<XMLCh>* const enums
        , const int finalSet
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    virtual const XMLCh* getCanonicalRepresentation
                        (
                          const XMLCh*         const rawData
                        ,       MemoryManager* const memMgr = 0
                        ,       bool                 toValidate = false
                        ) const;

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(DecimalDatatypeValidator)

protected:

// -----------------------------------------------------------------------
// ctor provided to be used by derived classes
// -----------------------------------------------------------------------
    DecimalDatatypeValidator
    (
        DatatypeValidator* const baseValidator
        , RefHashTableOf<KVStringPair>* const facets
        , const int finalSet
        , const ValidatorType type
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

// -----------------------------------------------------------------------
// Abstract interface from AbstractNumericFacetValidator
// -----------------------------------------------------------------------

    virtual void assignAdditionalFacet(const XMLCh* const key
                                     , const XMLCh* const value
                                     , MemoryManager* const manager);

    virtual void inheritAdditionalFacet();

    virtual void checkAdditionalFacetConstraints(MemoryManager* const manager) const;

    virtual void checkAdditionalFacetConstraintsBase(MemoryManager* const manager) const;

    virtual int  compareValues(const XMLNumber* const lValue
                             , const XMLNumber* const rValue);

    virtual void  setMaxInclusive(const XMLCh* const);

    virtual void  setMaxExclusive(const XMLCh* const);

    virtual void  setMinInclusive(const XMLCh* const);

    virtual void  setMinExclusive(const XMLCh* const);

    virtual void  setEnumeration(MemoryManager* const manager);

// -----------------------------------------------------------------------
// Abstract interface from AbstractNumericValidator
// -----------------------------------------------------------------------

    virtual void checkContent(const XMLCh*             const content
                            ,       ValidationContext* const context
                            , bool                           asBase
                            ,       MemoryManager*     const manager);
public:

// -----------------------------------------------------------------------
// Getter methods
// -----------------------------------------------------------------------

    inline unsigned int                    getTotalDigits() const;

    inline unsigned int                    getFractionDigits() const;

private:
// -----------------------------------------------------------------------
// Setter methods
// -----------------------------------------------------------------------

    inline void  setTotalDigits(unsigned int);

    inline void  setFractionDigits(unsigned int);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    // -----------------------------------------------------------------------
    unsigned int         fTotalDigits;
    unsigned int         fFractionDigits;


    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DecimalDatatypeValidator(const DecimalDatatypeValidator&);
    DecimalDatatypeValidator& operator=(const DecimalDatatypeValidator&);
};

// -----------------------------------------------------------------------
// Getter methods
// -----------------------------------------------------------------------

inline unsigned int DecimalDatatypeValidator::getTotalDigits() const
{
    return fTotalDigits;
}

inline unsigned int DecimalDatatypeValidator::getFractionDigits() const
{
    return fFractionDigits;
}

// -----------------------------------------------------------------------
// Setter methods
// -----------------------------------------------------------------------

inline void DecimalDatatypeValidator::setTotalDigits(unsigned int newTotalDigits)
{
    fTotalDigits = newTotalDigits;
}

inline void DecimalDatatypeValidator::setFractionDigits(unsigned int newFractionDigits)
{
    fFractionDigits = newFractionDigits;
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file DecimalDatatypeValidator.hpp
  */
