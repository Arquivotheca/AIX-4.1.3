/* @(#)28	1.4  src/bos/usr/include/strings.h, libcstr, bos411, 9428A410j 3/8/94 16:03:56 */
#ifndef _H_STRINGS
#define _H_STRINGS
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>

#ifdef _ALL_SOURCE

#ifdef _NO_PROTO

extern int bcmp();
extern void bcopy();
extern void bzero();
extern int  ffs();
extern char *index();
extern char *rindex();
extern int  strcasecmp();
extern int  strncasecmp();

#else /* _NO_PROTO */

extern int bcmp(const void *, const void *, size_t);
extern void bcopy(const void *, void *, size_t);
extern void bzero(void *, size_t);
extern int  ffs(int);
extern char *index(const char *s, int);
extern char *rindex(const char *, int);
extern int  strcasecmp(const char *, const char *);
extern int  strncasecmp(const char *, const char *, size_t);

#endif /* _NO_PROTO */

#endif /* _ALL_SOURCE */

#ifdef _ALL_SOURCE
#include <string.h>
#endif /* _ALL_SOURCE */


#endif /* _H_STRINGS */
