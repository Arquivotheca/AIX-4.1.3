/* @(#)03	1.29  src/bos/kernel/sys/NLchar.h, libcnls, bos411, 9428A410j 4/6/94 22:00:50 */
#ifndef _H_NLCHAR
#define _H_NLCHAR
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <jctype.h>
#include <sys/types.h>
#include <sys/localedef.h> 

loc_t *_locp = NULL;

/*  BASIC DEFINITIONS FOR USING NLCHARS.
 */

/*  The big type itself.
 */
typedef unsigned short NLchar;

extern char
	*NLstrcpy(),
	*NLstrncpy(),
	*NLstrcat(),
	*NLstrncat(),
	*NLstrchr(),
	*NLstrrchr(),
	*NLstrpbrk(),
	*NLstrtok();
extern NLchar
	*NCstrcpy(),
	*NCstrncpy(),
	*NCstrcat(),
	*NCstrncat(),
	*NCstrchr(),
	*NCstrrchr(),
	*NCstrpbrk(),
	*NCstrtok();

/*  Number of distinct NLchars.
 */
#define NLCOLMAX        16384     /* maximum # elements in collation table */
#define NLCHARMAX       256

#ifdef lint
extern NLchar NCdechr();
#else

/*  Single-shift character definition (both NLchar and char representation).
 */

#define _NCtop(nlc)     ((nlc) >> 8 & 0xff)
#define _NCbot(nlc)     ((nlc) & 0xff)

/*  SINGLE-CHARACTER CONVERSION MACROS.
 */
/* The use of the following macros will require recompilation of programs
 * in future versions of AIX. There are no function counterparts.
 */

/*  Internal macro to test for multi-byte NLS code point.
 */
#define _NCis2(c0,c1)   NCisshift(c0)

/*  Internal macro to convert multi-byte NLS code point to NLchar.
 */
#define _NCd2(c0,c1)    (((c0) << 8) | (c1))

/*  Internal macro to convert NLchar to NLS code point.
 */

#define _NCe2(nlc, c0, c1)      (((nlc) > 0xff) ? \
					((c0) = _NCtop(nlc), \
						(c1) = _NCbot(nlc), 2) : \
					((c0) = (nlc), 1))

/*  Convert c0 (and c1, if need be) into NLchar nlc; return # chars used.
 *  This is the only conversion macro that accepts an NLchar, rather than
 *  a pointer to one, as an argument.
 */
#define _NCdec2(c0, c1, nlc) ((_NCis2((c0), (c1))) ? \
				((nlc) = _NCd2((c0), (c1)), 2) : \
				((nlc) = (c0), 1))

/*  Defs for accessing collating data.
 */
/* The use of the following macros will require recompilation of programs
 * in future versions of AIX. There are no function counterparts.
 */
#define _NCmap(nlc)     ( ((nlc) <= 0xff) ? (nlc) : \
				( (nlc) < 0xdfff ? ( (nlc) - 0x8000 ) : \
						   ( (nlc) - 0xc000 ) ))
#define _NCunmap(nlc)   ( ((nlc) <= 0xff)? (nlc) : \
				( (nlc) < 0x2000 ? ( (nlc) + 0x8000 ) : \
						   ( (nlc) + 0xc000 ) ))

/* 
 * NCeqvmap is no longer supported because the definition of an equivalence 
 * class is changed to mean "all characters or collating elements with the 
 * same primary collation value". If used, the macro will always return 1 
 * ("no equivalence class").
 */

#define NCeqvmap(ucval) ( 1 )
#endif /* lint */

/*  MISCELLANY.
 */

/*  Internal defs for string(3) macros using charsets.
 */
#define NLCSETMAX	33	/* malloc charsets over this length - 1 */

#ifdef lint
extern NLchar *_NCbufstr();
extern void _NCfreebuf();
#else

/*  Decode a string into an NLchar array big enough to hold it.  Use array
 *  s if possible, else malloc one with _NLgetbuf.  First element flags
 *  whether malloc was used.
 */
#define _NCbufstr(s, d, l)	((d)[0] = 0, ((l) - 2 <= \
					NCdecstr((s), &(d)[1], (l) - 1)) ? \
					_NCgetbuf((s), (l)) : &(d)[1])


/*  Free a buffer filled by _NCbufstr, if it was malloc'd originally.
 */
#ifndef _KERNEL
#define _NCfreebuf(s)		if (*((s) - 1)) free((void *)((s) - 1));
#endif /* _KERNEL */
#endif /* LINT */

extern NLchar *_NCgetbuf();
#endif /* _H_NLCHAR */ 
