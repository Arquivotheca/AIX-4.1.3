/* @(#)09	1.11  src/bos/usr/include/iconv.h, libiconv, bos411, 9428A410j 6/29/94 13:53:51
 *
 *   COMPONENT_NAME:	LIBICONV
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_ICONV_H
#define	_ICONV_H

#ifndef	_H_STANDARDS
#include <standards.h>
#endif
#ifndef	_H_TYPES
#include <sys/types.h>
#endif
#ifndef	__H_LC_CORE
#include <sys/lc_core.h>
#endif
#ifdef	_ALL_SOURCE
#ifndef	_H_ERRNO
#include <errno.h>
#endif
#endif

#ifdef _XOPEN_SOURCE

#define _LC_ICONV     10

/*
 *	Definition of iconv_t type.
 */

typedef	struct __iconv_rec	*iconv_t;
struct	__iconv_rec	{
#ifdef	_ALL_SOURCE
	_LC_object_t	hdr;
	iconv_t	(*open) (const uchar_t*, const uchar_t*);
	size_t	(*exec) (iconv_t, uchar_t**, size_t*, uchar_t**, size_t*);
	int	(*close)(iconv_t);
#else
	_LC_object_t	__hdr;
	iconv_t	(*__open) (const uchar_t*, const uchar_t*);
	size_t	(*__exec) (iconv_t, uchar_t**, size_t *, uchar_t**, size_t *);
	int	(*__close)(iconv_t);
#endif /*_ALL_SOURCE*/
};

typedef	struct _LC_core_iconv_type	_LC_core_iconv_t;
struct	_LC_core_iconv_type {
#ifdef	_ALL_SOURCE
	_LC_object_t 		hdr;
	_LC_core_iconv_t	*(*init)();
	size_t			(*exec)();
	int			(*close)();
#else
	_LC_object_t		 __hdr;
	_LC_core_iconv_t	*(*__init)();
	size_t			(*__exec)();
	int			(*__close)();
#endif /*_ALL_SOURCE*/
};

/*
 *	methods.
 */

extern	iconv_t	iconv_open(const char*, const char*);
extern	size_t	iconv(iconv_t, const char**, size_t*, char**, size_t*);
extern	int	iconv_close(iconv_t);

#endif /*_XOPEN_SOURCE*/

#ifdef _ALL_SOURCE

typedef struct __iconv_rec	iconv_rec;
typedef unsigned int		CCSID;

#endif /*_ALL_SOURCE*/
#endif /*!_ICONV_H*/
