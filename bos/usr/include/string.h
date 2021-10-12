/* @(#)11	1.28.2.6  src/bos/usr/include/string.h, libcstr, bos411, 9430C411a 7/12/94 15:04:28 */
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27,71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

#ifndef _H_STRING
#define _H_STRING

#ifndef _H_STANDARDS
#include <standards.h>
#endif /* _H_STANDARDS */

#ifndef _H_TYPES
#include <sys/types.h>
#endif /* _H_TYPES */

/*
 *
 *      The ANSI standard requires that certain values be in string.h.
 *      It also requires that if _ANSI_C_SOURCE is defined then ONLY these
 *      values are present. This header includes all the ANSI required entries.
 *      In addition other entries for the XIX system are included.
 *
 */

#ifdef   _ANSI_C_SOURCE

/*
 *	The following definitions (NULL, size_t) are included in <sys/types.h>.
 *	They are also included here to comply with ANSI standards.
 */

#   ifndef NULL
#	define NULL	0
#   endif /* NULL */

#   ifndef _SIZE_T
#	define _SIZE_T
	typedef unsigned long	size_t;
#   endif /* _SIZE_T */

#   ifdef	_NONSTD_TYPES
	extern char	*memchr();
	extern char	*memcpy();
	extern char	*memset();
	extern int	strcspn();
	extern int	strlen();
	extern int	strspn();
#   elif	defined	_NO_PROTO
	extern void	*memchr();
	extern void	*memcpy();
	extern void	*memset();
	extern size_t 	strcspn();
	extern size_t	strlen();
	extern size_t	strspn();
#   else	/* _NONSTD_TYPES, _NO_PROTO */
        extern void     *memchr(const void *, int, size_t);
        extern void     *memcpy(void *, const void *, size_t);
        extern void     *memset(void *, int, size_t);
        extern size_t   strcspn(const char *, const char *);
        extern size_t   strlen(const char *);
        extern size_t   strspn(const char *, const char *);
#   endif	/* _NONSTD_TYPES, _NO_PROTO */

#   ifdef   _NO_PROTO
	extern void 	*memmove();
	extern char 	*strcpy();
	extern char 	*strncpy();
	extern char 	*strcat();
	extern char	*strncat();
	extern int 	memcmp();
	extern int	strcmp();
	extern int	strncollen();
	extern int	strncmp();
	extern char	*strchr();
	extern char	*strpbrk();
	extern char	*strrchr();
	extern char	*strstr();
	extern char 	*strtok();
	extern char 	*strerror();
	extern int	strcoll();
	extern size_t strxfrm();
#	ifdef _THREAD_SAFE
	    extern char *strtok_r();
	    extern int	strerror_r();
#	endif /* _THREAD_SAFE */

#   else  /*_NO_PROTO */
        extern void     *memmove(void *, const void *, size_t);
        extern char     *strcpy(char *, const char *);
        extern char     *strncpy(char *, const char *, size_t);
        extern char     *strcat(char *, const char *);
        extern char     *strncat(char *, const char *, size_t);
        extern int      memcmp(const void *, const void *,size_t);
        extern int      strcmp(const char *, const char *);
        extern int      strncmp(const char *,const char *,size_t);
        extern int      strncollen(const char *, const int );
        extern char     *strchr(const char *, int);
        extern char     *strpbrk(const char *, const char *);
        extern char     *strrchr(const char *, int);
        extern char     *strstr(const char *, const char *);
        extern char     *strtok(char *, const char *);
        extern char     *strerror(int);
        extern int      strcoll(const char *, const char *);
        extern size_t strxfrm(char *, const char *, size_t);
#	ifdef _THREAD_SAFE
	    extern char *strtok_r(char *, const char *, char **);
	    extern int	strerror_r(int, char *, int);
#	endif /* _THREAD_SAFE */

#   endif /*_NO_PROTO */
#endif /*_ANSI_C_SOURCE */

#ifdef	_XOPEN_SOURCE
#   ifdef	_NONSTD_TYPES
	extern char	*memccpy();
#   elif	defined	_NO_PROTO
	extern void	*memccpy();
#   else	/* _NONSTD_TYPES, _NO_PROTO */
        extern void     *memccpy(void *, const void *, int, size_t);
#   endif	/* _NONSTD_TYPES, _NO_PROTO */

#endif	/* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE

#ifdef _NO_PROTO
extern char *strdup();
#else /* _NO_PROTO */
extern char *strdup(const char *);
#endif /* _NO_PROTO */

#endif /* _ALL_SOURCE */

#ifdef	_ALL_SOURCE
#   ifdef	_NO_PROTO
	extern char     *index();
	extern char     *rindex();
	extern void	swab();
	extern wchar_t	*wcscat();
	extern wchar_t	*wcschr();
	extern int	wcscmp();
	extern wchar_t	*wcscpy();
	extern size_t	wcscspn();
	extern size_t	wcslen();
	extern wchar_t	*wcsncat();
	extern int	wcsncmp();
	extern wchar_t	*wcsncpy();
	extern wchar_t	*wcspbrk();
	extern wchar_t	*wcsrchr();
	extern size_t	wcsspn();
	extern wchar_t	*wcstok();
	extern wchar_t	*wcswcs();
	extern int	wcswidth();
	extern int	wcwidth();
	extern int	wcscoll();
	extern size_t wcsxfrm();
#ifdef _THREAD_SAFE
	extern wchar_t	*wcstok_r();
	extern int 	dirname_r();
#endif   /* _THREAD_SAFE */
#   else	/*_NO_PROTO */
        extern char     *index(const char *, int);
        extern char     *rindex(const char *, int);
        extern void     swab(const void *, void *, ssize_t); 
        extern wchar_t  *wcscat(wchar_t *, const wchar_t *);
        extern wchar_t  *wcschr(const wchar_t *, wchar_t);
        extern int      wcscmp(const wchar_t *, const wchar_t *);
        extern wchar_t  *wcscpy(wchar_t *, const wchar_t *);
        extern size_t   wcscspn(const wchar_t *, const wchar_t *);
        extern size_t   wcslen(const wchar_t *);
        extern wchar_t  *wcsncat(wchar_t *, const wchar_t *, size_t);
        extern int      wcsncmp(const wchar_t *, const wchar_t *, size_t);
        extern wchar_t  *wcsncpy(wchar_t *, const wchar_t *, size_t);
        extern wchar_t  *wcspbrk(const wchar_t *, const wchar_t *);
        extern wchar_t  *wcsrchr(const wchar_t *, wchar_t);
        extern size_t   wcsspn(const wchar_t *, const wchar_t *);
        extern wchar_t  *wcstok(wchar_t *, const wchar_t *);
        extern wchar_t  *wcswcs(const wchar_t *, const wchar_t *);
        extern int      wcswidth(const wchar_t *, size_t);
        extern int      wcwidth(wchar_t);
        extern int      wcscoll(const wchar_t *, const wchar_t *);
        extern size_t wcsxfrm(wchar_t *, const wchar_t *, size_t);
#ifdef _THREAD_SAFE
	extern wchar_t  *wcstok_r(wchar_t *ws1, const wchar_t *ws2, wchar_t **savept);
	extern int 	dirname_r(char *path, char *buf, int size);
#endif   /* _THREAD_SAFE */
#   endif	/* _NO_PROTO */

#ifdef __STR__
#   ifndef __STR31__
#       define index(__s1,__c) __strchr(__s1,__c)
#       define rindex(__s1,__c) __strrchr(__s1,__c)
#   endif /* ifndef __STR31__ */
#endif /* __STR__ */

#endif	/* _ALL_SOURCE */

/*
 *   The following macro definitions cause the XLC compiler to inline
 *   these functions whenever possible.
 */
 
#ifdef __STR__
#   define strcpy(__s1,__s2) __strcpy(__s1,__s2)
#   define strcmp(__s1,__s2) __strcmp(__s1,__s2)

#   ifndef __STR31__
#       define strlen(__s1) __strlen(__s1)
#       define strchr(__s1,__c) __strchr(__s1,__c)
#       define strrchr(__s1,__c) __strrchr(__s1,__c)
#       define strcat(__s1,__s2) __strcat(__s1,__s2)
#       define memchr(__s1,__c,__n) __memchr(__s1,__c,__n)
#       define memcpy(__s1,__s2,__n) __memcpy(__s1,__s2,__n)
#       define memmove(__s1,__s2,__n) __memmove(__s1,__s2,__n)
#       define memcmp(__s1,__s2,__n) __memcmp(__s1,__s2,__n)
#       define memset(__s1,__c,__n) __memset(__s1,__c,__n)
/*
 *   __XLC121__ is automatically defined by the XLC 1.2.1 compiler so that
 *   the compiler can inline the following functions when possible.
 */
#     ifdef __XLC121__
#       define memccpy(__target,__source,__c,__n)  __memccpy(__target,__source,__c,__n)
#       define strncat(__s1, __s2, __n)    __strncat(__s1, __s2, __n)
#       define strncmp(__s1, __s2, __n)    __strncmp(__s1, __s2, __n)
#       define strncpy(__s1, __s2, __n)    __strncpy(__s1, __s2, __n)
#     endif /*__XLC121__ */
#   endif /* ifndef __STR31__ */
#endif /* __STR__ */

#endif /* _H_STRING */
