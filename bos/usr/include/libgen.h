/* @(#)92	1.1  src/bos/usr/include/libgen.h, libPW, bos411, 9428A410j 3/4/94 11:06:52 */
#ifndef _H_LIBGEN
#define _H_LIBGEN

/*
 *   COMPONENT_NAME: LIBPW
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

extern char * __loc1;

#ifdef _NO_PROTO
extern char *basename();
extern char *dirname();
extern char *regcmp();
extern char *regex();
#else /* _NO_PROTO */
extern char *basename(char *);
extern char *dirname(char *);
extern char *regcmp(const char *, ...);
extern char *regex(const char *, const char *, ...);
#endif /* _NO_PROTO */

#endif /* _H_LIBGEN */
