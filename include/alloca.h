/*
 * alloca.h : alloca definitions
 * Copyright (c) 1992 Algorithmics Ltd.
 */

#ifndef __ALLOCA_H
#ifdef __cplusplus
extern "C" {
#endif
#define __ALLOCA_H

#if defined(__GNUC__) || defined(__sparc__)
#define alloca(x)	__builtin_alloca(x)
#else
void	*alloca (size_t);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __ALLOCA_H */
