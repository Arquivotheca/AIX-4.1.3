/* @(#)98	1.31.1.6  src/bos/usr/include/ctype.h, libcchr, bos411, 9428A410j 7/6/94 16:46:57 */

/*
 * COMPONENT_NAME: LIBCCHR
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
 *
 */

#ifndef __H_CTYPE
#define __H_CTYPE

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifdef _ANSI_C_SOURCE

#define _ISALPHA	0x001
#define _ISALNUM	0x002
#define _ISBLANK	0x004
#define _ISCNTRL	0x008
#define _ISDIGIT	0x010
#define _ISGRAPH	0x020
#define _ISLOWER	0x040
#define _ISPRINT	0x080
#define _ISPUNCT	0x100
#define _ISSPACE	0x200
#define _ISUPPER	0x400
#define _ISXDIGIT	0x800

#ifdef _NO_PROTO
	extern int	isalpha();
	extern int	isalnum();
	extern int	iscntrl();
	extern int	isdigit();
	extern int	isgraph();
	extern int	islower();
	extern int	isprint();
	extern int	ispunct();
	extern int	isspace();
	extern int	isupper();
	extern int	isxdigit();
	extern int	toupper();
	extern int	tolower();
#else /* _NO_PROTO */
	extern int	isalpha(int);
	extern int	isalnum(int);
	extern int	iscntrl(int);
	extern int	isdigit(int);
	extern int	isgraph(int);
	extern int	islower(int);
	extern int	isprint(int);
	extern int	ispunct(int);
	extern int	isspace(int);
	extern int	isupper(int);
	extern int	isxdigit(int);
	extern int	toupper(int);
	extern int	tolower(int);
#endif /* _NO_PROTO */

#endif /* _ANSI_C_SOURCE */

#ifdef _XOPEN_SOURCE

#   ifndef _WCHAR_T
#	define _WCHAR_T
	typedef unsigned short	wchar_t;
#   endif /* _WCHAR_T */

#   ifndef _WCTYPE_T
#	define _WCTYPE_T
	typedef unsigned int	wctype_t;
#   endif /* _WCTYPE_T */

#   ifndef _WINT_T
#	define _WINT_T
	typedef int	wint_t;
#   endif /* _WINT_T */

#ifdef _NO_PROTO
	extern int	isascii();
	extern int	toascii();
#else /* _NO_PROTO */
	extern int	isascii(int);
	extern int	toascii(int);
#endif /* _NO_PROTO */

#define _toupper(__a) toupper(__a)
#define _tolower(__a) tolower(__a)

#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE

#ifndef __H_LOCALEDEF
#include <sys/localedef.h>
#endif

#ifdef _NO_PROTO
	extern int	is_wctype();
	extern wctype_t	get_wctype();
#else
	extern int	is_wctype(wint_t, wctype_t);
	extern wctype_t	get_wctype(char *);
#endif /* NO_PROTO */

#ifdef _ILS_MACROS	

/* macros for ANSI functions */
#define _VALC(__c)		((__c)>=0&&(__c)<=256)
#define _IS(__c,__m)		(__OBJ_DATA(__lc_ctype)->mask[__c] & __m)
#define isalpha(__a)	(_VALC(__a)?_IS(__a,_ISALPHA):0)
#define isalnum(__a)	(_VALC(__a)?_IS(__a,_ISALNUM):0)
#define iscntrl(__a)	(_VALC(__a)?_IS(__a,_ISCNTRL):0)
#define isdigit(__a)	(_VALC(__a)?_IS(__a,_ISDIGIT):0)
#define isgraph(__a)	(_VALC(__a)?_IS(__a,_ISGRAPH):0)
#define islower(__a)	(_VALC(__a)?_IS(__a,_ISLOWER):0)
#define isprint(__a)	(_VALC(__a)?_IS(__a,_ISPRINT):0)
#define ispunct(__a)	(_VALC(__a)?_IS(__a,_ISPUNCT):0)
#define isspace(__a)	(_VALC(__a)?_IS(__a,_ISSPACE):0)
#define isupper(__a)	(_VALC(__a)?_IS(__a,_ISUPPER):0)
#define isxdigit(__a)	(_VALC(__a)?_IS(__a,_ISXDIGIT):0)
#define _XUPPER(__a)	(__OBJ_DATA(__lc_ctype)->upper[__a])
#define _XLOWER(__a)	(__OBJ_DATA(__lc_ctype)->lower[__a])
#define toupper(__a)	(_VALC(__a)?_XUPPER(__a):__a)
#define tolower(__a)	(_VALC(__a)?_XLOWER(__a):__a)
#define isascii(c)	(!((c) & ~0177))

/* macros for XOPEN functions */
#define _SBCK(__a,__m)	(__OBJ_DATA(__lc_ctype)->mask[__a] & __m)
#define _MBCK(__a,__m)	(__OBJ_DATA(__lc_ctype)->qmask[__OBJ_DATA(__lc_ctype)->qidx[(__a)-256]] & __m)
#define _MBVALCK(__a,__m) ((__a)>__OBJ_DATA(__lc_ctype)->max_wc?0:_MBCK(__a,__m))
#define _VALCK(__a,__m)	((__a)<256?_SBCK(__a,__m):_MBVALCK(__a,__m))
#define iswalpha(__a)	((__a)>=0?_VALCK(__a,_ISALPHA):0)
#define iswalnum(__a)	((__a)>=0?_VALCK(__a,_ISALNUM):0)
#define iswcntrl(__a)	((__a)>=0?_VALCK(__a,_ISCNTRL):0)
#define iswdigit(__a)	((__a)>=0?_VALCK(__a,_ISDIGIT):0)
#define iswgraph(__a)	((__a)>=0?_VALCK(__a,_ISGRAPH):0)
#define iswlower(__a)	((__a)>=0?_VALCK(__a,_ISLOWER):0)
#define iswprint(__a)	((__a)>=0?_VALCK(__a,_ISPRINT):0)
#define iswpunct(__a)	((__a)>=0?_VALCK(__a,_ISPUNCT):0)
#define iswspace(__a)	((__a)>=0?_VALCK(__a,_ISSPACE):0)
#define iswupper(__a)	((__a)>=0?_VALCK(__a,_ISUPPER):0)
#define iswxdigit(__a)	((__a)>=0?_VALCK(__a,_ISXDIGIT):0)
#define is_wctype(__a,__b)	((__a)>=0?_VALCK(__a,__b):0)
#define iswctype(__a,__b)	((__a)>=0?_VALCK(__a,__b):0)
#define towupper(__a)	(((__a) >= __OBJ_DATA(__lc_ctype)->min_wc && \
			  (__a) <= __OBJ_DATA(__lc_ctype)->max_wc) ? \
				      __OBJ_DATA(__lc_ctype)->upper[__a] : __a)
#define towlower(__a)	(((__a) >= __OBJ_DATA(__lc_ctype)->min_wc && \
			  (__a) <= __OBJ_DATA(__lc_ctype)->max_wc) ? \
				      __OBJ_DATA(__lc_ctype)->lower[__a] : __a)
#endif /* _ILS_MACROS */

#undef _toupper
#undef _tolower

#define _toupper(__a)	(__OBJ_DATA(__lc_ctype)->upper[__a])
#define _tolower(__a)	(__OBJ_DATA(__lc_ctype)->lower[__a])

#endif /* _ALL_SOURCE */

#endif /* __H_CTYPE */
