/* @(#)37	1.6  src/bos/usr/include/mbstr.h, libcnls, bos411, 9428A410j 2/10/93 16:09:16 */
/*
 *   COMPONENT_NAME: LIBCNLS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 3 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   Copyright (c) 1984 AT&T
 *   All Rights Reserved
 *
 *   THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 *   The copyright notice above does not evidence any
 *   actual or intended publication of suc
 */

/*
 * NAME: mbstr.h
 */                                                                   
#ifndef _H_MBSTR
#define _H_MBSTR
#include <sys/types.h>

#define MBMAXLEN	4

typedef unsigned int mbchar_t;

#ifdef   _NO_PROTO
    extern char	*mbsadvance();
    extern char	*mbscat();
    extern char	*mbschr();
    extern int	mbscmp();
    extern char	*mbscpy();
    extern char	*mbsinvalid();
    extern size_t	mbslen();
    extern char	*mbsncat();
    extern int	mbsncmp();
    extern char	*mbsncpy();
    extern char	*mbspbrk();
    extern char	*mbsrchr();
    extern mbchar_t	mbstomb();
    extern int	mbswidth();
    extern int	mbstoint();
#else  /*_NO_PROTO */
    extern char	*mbsadvance(const char *s);
    extern char	*mbscat(char *, char *);
    extern char	*mbschr(const char *, const mbchar_t);
    extern int	mbscmp(char *, char *);
    extern char	*mbscpy(char *, char *);
    extern char	*mbsinvalid(const char *s);
    extern size_t	mbslen(const char *);
    extern char	*mbsncat(char *, const char *, size_t);
    extern int	mbsncmp(char *, char *, size_t);
    extern char	*mbsncpy(char *, char *, size_t);
    extern char	*mbspbrk(char *, char *);
    extern char	*mbsrchr(char *, mbchar_t);
    extern mbchar_t	mbstomb(const char *mbs);
    extern int	mbswidth(const char *s, size_t n);
    extern int	mbstoint(char *);
#endif /*_NO_PROTO */
#endif /* _H_MBSTR */
