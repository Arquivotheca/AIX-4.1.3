/* @(#)88	1.24  src/bos/usr/include/nl_types.h, libcmsg, bos411, 9428A410j 5/26/94 16:25:52 */

/*
 * COMPONENT_NAME: LIBCMSG
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_NL_TYPES
#define _H_NL_TYPES

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifdef _XOPEN_SOURCE
typedef int nl_item;
typedef struct _catalog_descriptor *nl_catd;

#define NL_SETD 	1  				
#define NL_CAT_LOCALE 	1  				

#ifdef _NO_PROTO
extern nl_catd catopen();
extern char  *catgets();
extern int catclose();
#else
extern nl_catd catopen(const char *, int);
extern char  *catgets(nl_catd, int, int, const char *);
extern int catclose(nl_catd);
#endif /* _NO_PROTO */
#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE
typedef struct _catalog_descriptor CATD;

#ifdef	_NO_PROTO
nl_catd NLcatopen();
char *	NLgetamsg();
char *NLcatgets();
char *catgetmsg();
#else
nl_catd NLcatopen(char*, int);
extern char *NLgetamsg(char*, int, int, char*);
extern char *NLcatgets(nl_catd, int, int, char *);
extern char *catgetmsg(nl_catd, int, int, char*, int);
#endif	/* _NO_PROTO */

#ifndef _H_MESG
#include <mesg.h>
#endif

#endif /* _ALL_SOURCE */

#endif /* _H_NL_TYPES */
