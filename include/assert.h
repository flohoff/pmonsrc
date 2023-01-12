#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
/*
 * assert.h : assertion definitions
 * Copyright (c) 1992 Algorithmics Ltd.
 */
/* assert.h can be included several times
*/
#undef assert
#undef __assert

#ifdef NDEBUG
#define assert(ignore)  ((void)0)
#else

extern void __assfail(char *fmt,...) __attribute__ ((noreturn));

#define assert(expression)  \
  ((void)((expression) ? 0 : (__assert (#expression, __FILE__, __LINE__), 0)))

#define __assert(expression, file, line)  \
  __assfail ("Failed assertion `%s' at line %d of `%s'.\n",	\
	       expression, line, file)
#endif


#ifdef __cplusplus
}
#endif
