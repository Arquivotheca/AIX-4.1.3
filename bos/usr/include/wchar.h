/* @(#)33	1.13  src/bos/usr/include/wchar.h, libcnls, bos411, 9430C411a 7/12/94 15:01:32 */

/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
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

#ifndef _H_WCHAR
#define _H_WCHAR

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_TYPES
#include <sys/types.h>
#endif

#ifdef _XOPEN_SOURCE

#ifndef _H_STDIO
#include <stdio.h>
#endif

#ifndef _H_TIME
#include <time.h>
#endif

#ifndef	WEOF
#define WEOF	(-1)
#endif	/* WEOF	*/

#ifdef	_NO_PROTO

extern wint_t fgetwc();
extern wchar_t *fgetws();
extern wint_t fputwc();
extern int fputws();
extern wint_t getwc();
extern wint_t getwchar();

#if (!(defined(__H_CTYPE) && defined(_ILS_MACROS)))
extern int iswalnum();
extern int iswalpha();
extern int iswcntrl();
extern int iswdigit();
extern int iswgraph();
extern int iswlower();
extern int iswprint();
extern int iswpunct();
extern int iswspace();
extern int iswupper();
extern int iswxdigit();
extern int iswctype();
extern int towlower();
extern int towupper();
#endif /* (!(defined(__H_CTYPE) && defined(_ILS_MACROS))) */

extern wint_t putwc();
extern wint_t putwchar();
extern wint_t ungetwc();
extern wctype_t wctype();
extern wchar_t *wcscat();
extern wchar_t *wcschr();
extern int wcscmp();
extern int wcscoll();
extern wchar_t *wcscpy();
extern size_t wcscspn();
extern size_t wcsftime();
extern size_t wcslen();
extern wchar_t *wcsncat();
extern int wcsncmp();
extern wchar_t *wcsncpy();
extern wchar_t *wcspbrk();
extern wchar_t *wcsrchr();
extern size_t wcsspn();
extern double wcstod();
extern wchar_t *wcstok();
extern long wcstol();
extern unsigned long wcstoul();
extern wchar_t *wcswcs();
extern int wcswidth();
extern size_t wcsxfrm();
extern int wcwidth();

#else

extern wint_t fgetwc(FILE *);
extern wchar_t *fgetws(wchar_t *, int, FILE *);
extern wint_t fputwc(wint_t, FILE *);
extern int fputws(const wchar_t *, FILE *);
extern wint_t getwc(FILE *);
extern wint_t getwchar(void);

#if (!(defined(__H_CTYPE) && defined(_ILS_MACROS)))
extern int iswalnum(wint_t);
extern int iswalpha(wint_t);
extern int iswcntrl(wint_t);
extern int iswdigit(wint_t);
extern int iswgraph(wint_t);
extern int iswlower(wint_t);
extern int iswprint(wint_t);
extern int iswpunct(wint_t);
extern int iswspace(wint_t);
extern int iswupper(wint_t);
extern int iswxdigit(wint_t);
extern int iswctype (wint_t, wctype_t);
extern int towlower(wint_t);
extern int towupper(wint_t);
#endif /* (!(defined(__H_CTYPE) && defined(_ILS_MACROS))) */

extern wint_t putwc(wint_t, FILE *);
extern wint_t putwchar(wint_t);
extern wint_t ungetwc(wint_t, FILE *);
extern wctype_t wctype(const char*);
extern wchar_t *wcscat(wchar_t *, const wchar_t *);
extern wchar_t *wcschr(const wchar_t *,  wchar_t);
extern int wcscmp(const wchar_t *, const wchar_t *);
extern int wcscoll(const wchar_t *, const wchar_t *);
extern wchar_t *wcscpy(wchar_t *, const wchar_t *);
extern size_t wcscspn(const wchar_t *, const wchar_t *);
extern size_t wcsftime(wchar_t *, size_t, const char *, const struct tm *);
extern size_t wcslen(const wchar_t *);
extern wchar_t *wcsncat(wchar_t *, const wchar_t *, size_t);
extern int wcsncmp(const wchar_t *, const wchar_t *, size_t);
extern wchar_t *wcsncpy(wchar_t *, const wchar_t *, size_t);
extern wchar_t *wcspbrk(const wchar_t *, const wchar_t *);
extern wchar_t *wcsrchr(const wchar_t *, wchar_t);
extern size_t wcsspn(const wchar_t *, const wchar_t *);
extern double wcstod(const wchar_t *, wchar_t **);
extern wchar_t *wcstok(wchar_t *, const wchar_t *);
extern long wcstol(const wchar_t *, wchar_t **, int);
extern unsigned long wcstoul(const wchar_t *, wchar_t **, int);
extern wchar_t *wcswcs(const wchar_t *, const wchar_t *);
extern int wcswidth(const wchar_t *, size_t);
extern size_t wcsxfrm(wchar_t *, const wchar_t *, size_t);
extern int wcwidth(wchar_t);

#endif	/* _NO_PROTO	*/

#endif	/* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE

#define getwchar()	getwc(stdin)

#ifdef	_NO_PROTO
extern wchar_t *getws();
extern int putws();
extern int wsprintf();
extern int vwsprintf();
#else
extern wchar_t *getws(wchar_t *);
extern int putws(const wchar_t *);
extern int wsprintf(wchar_t *, const char *, ...);
#ifdef _VA_LIST
extern int vwsprintf(wchar_t *, const char *, va_list);
#else
#define _HIDDEN_VA_LIST
#include <va_list.h>
extern int vwsprintf(wchar_t *, const char *, __va_list);
#endif /* _VA_LIST */

#endif /* _NO_PROTO */

extern wchar_t *strtows(wchar_t *, char *);
extern char    *wstrtos(char *, wchar_t *);

#define wstrcat NCstrcat
#define wstrlen NCstrlen
#define wstrdup NCstrdup
#define wstrncat NCstrncat
#define wstrcmp NCstrcmp
#define wstrncmp NCstrncmp
#define wstrcpy NCstrcpy
#define wstrncpy NCstrncpy

#endif /* _ALL_SOURCE */

/*
 * 64-bit integers, known as long long and unsigned long long
 */
#if (defined(_LONG_LONG) && defined(_ALL_SOURCE))
#ifdef _NO_PROTO
 
extern long long int wcstoll( );
extern unsigned long long int wcstoull( );
 
#else /* ifdef _NO_PROTO */
 
extern long long int wcstoll(
     const wchar_t *,		/* String to convert */
     wchar_t **,		/* Pointer to update */
     int );			/* Base to use */
extern unsigned long long int wcstoull(
     const wchar_t *,		/* String to convert */
     wchar_t **,		/* Pointer to update */
     int );			/* Base to use */
 
#endif /* ifdef _NO_PROTO */
#endif /* if defined(_LONG_LONG) && defined(_ALL_SOURCE) */

#endif /* _H_WCHAR */

