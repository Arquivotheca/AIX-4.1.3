/* @(#)48	1.6  src/bos/usr/include/ras.h, cmderrlg, bos411, 9428A410j 9/28/93 13:34:09 */

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: common header file for CMDERRLG, CMDTRACE, CMDDUMP
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_RAS
#define _H_RAS

/*
 * Progname is a convention that is the base name of the program as exec-ed.
 * setprogname() appears at the start of each program in main.c .
 */
extern char *Progname;
extern char *mcs_catalog;			/* contains name of message catalog */
extern char *mcs_catalog_a;			/* contains name of "raslib.cat" */
extern char *Yes,*No;				/* NLS yes and no */
#ifdef MCS_CATALOG
#define setprogname() \
	Progname = basename(argv[0]); \
	jcatprint_chk(argc,argv,MCS_CATALOG);
#else
#define setprogname() \
	Progname = basename(argv[0]); \
	jcatprint_chk(argc,argv,0);
#endif

/*
 * utility routines.
 */
extern char *vset();		/* prepend HOME directory to filename */
extern char *basename();	/* return the basename of the string */
extern char *dirname();		/* return the dirname of the string */
extern char *errstr();		/* return the perror error string */
extern char *stracpy();		/* malloc and copy a string to a pointer */
extern char *jalloc();		/* malloc, with clear. exit if fails */
extern char *jcatgets();	/* return message catalog string */
extern void jsignal();		/* common signal handling routine */

/*
 * MIN(x,y)
 *   minimum value of x and y
 * MALLOC(n,struct c_entry)
 *   allocate n c_entry structures and return a properly type-casted
 *   pointer to the area.
 * OFF(structure_name,member_name)
 *   will give the byte offset of a particular member in the structure.
 * OFFC(structure_name,member_name)
 *   like OFF(), but for array[] members.
 * ALIGN(N,x)
 *   Round 'x' up to the next 'N' byte boundary.
 * STREQ(s1,s2)
 *   true if the strings are equal. The test of the first character
 *   often avoids the function call when the strings are different.
 */
#undef  MIN
#define MIN(x,y)       ( (x) < (y) ? (x) : (y) )
#define MALLOC(n,s)    ( (s *)jalloc(   (n) * sizeof(s)) )
#define REALLOC(p,n,s) ( (s *)realloc(p,(n) * sizeof(s)) )
#define OFF(s,m)       ( (int)&(((struct s *)0)->m) )	/* member offset */
#define OFFC(s,m)      ( (int)&(((struct s *)0)->m[0]) )
#define ALIGN(N,x)     ( (x)%(N) ? (x)+(N)-(x)%(N) : (x) )	/* N byte bndry */
#define STREQ(s1,s2)   ( ((s1)[0] == (s2)[0]) && streq(s1,s2) )

/*
 * Debug macro.
 * This macro has varialble number of args like printf, but
 * does a test of a global Debugflg before the call.
 */
#define ERR_DEBUG 1
extern Debugflg;
#define Debug  ERR_DEBUG && (Debugflg == 0) ? 0 : _Debug
#define Debug2 ERR_DEBUG && (Debugflg <  2) ? 0 : _Debug

/*
 * hex dump to Btrace debug file.
 */
#define JDUMP(STR) \
	ERR_DEBUG && (Debugflg == 0) ? 0 : _jdump("STR",STR,sizeof(*(STR)))

/*
 * Termination codes used by the 2-pass programs such as
 * errupdate, errinstall, errmsg, and trcrpt.
 * The setjmp variables are used to do the global longjmp.
 */
#define EXCP_EOF      1
#define EXCP_UNEXPEOF 2
#define EXCP_INT      3
extern setjmpinitflg;	/* setjmp variable */
extern eofjmp[];		/* setjmp variable */

typedef void (*sigtype)();

typedef unsigned short Boolean;

#define M(x) x

#include <locale.h>
#include <ctype.h>

/*
 * NLPUTC(c,cp)  is   *cp++ = c;

* #define NLPUTC(nlc,cp) \
* { \
*	int n; \
* \
*	n = _NCe2((nlc),(cp)[0],(cp)[1]); \
*	(cp) += n; \
* }
*/
#endif /* _H_RAS */

