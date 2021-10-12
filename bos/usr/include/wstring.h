/* @(#)35	1.6  src/bos/usr/include/wstring.h, libcnls, bos411, 9428A410j 5/16/91 09:45:10 */
#ifndef _H_WSTRING
#define _H_WSTRING
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <wchar.h>

extern double	 wstrtod(wchar_t *, wchar_t **);
extern double	 watof(wchar_t *);
extern int	 watoi(wchar_t *);
extern long	 watol(wchar_t *);
extern long	 wstrtol(wchar_t *, wchar_t **, int);

extern wchar_t
	*wstrdup(wchar_t *),
	*wstrcpy(wchar_t *, wchar_t *),
	*wstrncpy(wchar_t *, wchar_t *, int),
	*wstrcat(wchar_t *, wchar_t *),
	*wstrncat(wchar_t *, wchar_t *, int),
	*wstrchr(wchar_t *, int),
	*wstrrchr(wchar_t *, int),
	*wstrpbrk(wchar_t *, wchar_t *),
	*wstrtok(wchar_t *, wchar_t *);

extern size_t
	wstrlen(wchar_t *),
	wstrspn(wchar_t *, wchar_t *),
	wstrcspn(wchar_t *, wchar_t *);

extern int
	wstrcmp(wchar_t *, wchar_t *),
	wstrncmp(wchar_t *, wchar_t *, int);
#endif /* _H_WSTRING */
