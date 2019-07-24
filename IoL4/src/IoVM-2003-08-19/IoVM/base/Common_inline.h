/*   Copyright (c) 2002, Steve Dekorte
 *   All rights reserved. See _BSDLicense.txt.
 *
 *   You may need to add an entry for your C compiler
 */

/* Trick to get inlining to work with various compilers
 * Kudos to Daniel A. Koepke
 */

#undef IO_DECLARE_INLINES
#undef IOINLINE

#if defined(__cplusplus)
  #ifdef IO_IN_C_FILE
    /* in .c */
  #else 
    /* in .h */
    #define IO_DECLARE_INLINES
    #define IOINLINE extern inline
  #endif
#else
  #ifdef IO_IN_C_FILE
    /* in .c */
    #define IO_DECLARE_INLINES
    #define IOINLINE inline
  #else 
    /* in .h */
    #define IO_DECLARE_INLINES
    #define IOINLINE extern inline
  #endif
#endif

