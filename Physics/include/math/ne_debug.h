#ifndef NE_DEBUG_H
#define NE_DEBUG_H

#include <assert.h>

#ifdef VERIFY
#undef VERIFY
#endif

#ifdef VERIFYS
#undef VERIFYS
#endif

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef ASSERTS
#undef ASSERTS
#endif

#ifdef BREAK
#undef BREAK
#endif


#ifdef _DEBUG
    #define ASSERT(E)    assert(E)
#else
    #define ASSERT(E)    //assert(E)
#endif


#endif//NE_DEBUG_H
