/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 * This is a header that all other source files should include.
 */

/*
 * These defines are helpful for doing OS specific checks in the code
 */
 
#ifndef IOCOMMON_DEFINED
#define IOCOMMON_DEFINED 1

#include "IoConfig.h"
#define LOW_MEMORY_SYSTEM 1
#include <stdlib.h>
#include <string.h>

void *cpalloc(void *p, size_t size);

/* Windows stuff */

#if defined(WIN32) || defined(__WINS__) || defined(__MINGW32__) || defined(_MSC_VER)
  #define inline __inline
  
  #if !defined(__MINGW32__)
    /* disable compile warnings which are always treated 
       as errors in my dev settings */

    #pragma warning( disable : 4244 )
    /* warning C4244: 'function' : conversion from 'double ' to 'int ', possible loss of data */
  
    /*#pragma warning( disable : 4090 ) */
    /* warning C4090: 'function' : different 'const' qualifiers  */
  
    /*#pragma warning( disable : 4024 )*/ 
    /* warning C4024: different types for formal and actual parameter  */
  
    /*#pragma warning( disable : 4761 ) */
    /* warning C4761: integral size mismatch in argument; conversion supplied  */
  
    /*#pragma warning( disable : 4047 ) */
    /* warning C4047: '=' : 'char *' differs in levels of indirection from 'int '  */
  #endif

  /* malloc, realloc, free undefined */
  #if !defined(__SYMBIAN32__)
    #include <memory.h> 
  #endif
    
  /* strlen undefined */
  #include <string.h> 
  #include <malloc.h> /* for calloc */
#endif

#endif

