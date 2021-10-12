/* @(#)75	1.2  src/bos/usr/bin/bterm/X/Xfuncs.h, libbidi, bos411, 9428A410j 6/9/94 10:39:54 */
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 16,27,40,42
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * $XConsortium: Xfuncs.h,v 1.8 91/04/17 09:27:52 rws Exp $
 * 
 * Copyright 1990 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 */

#ifndef _XFUNCS_H_
#define _XFUNCS_H_

#include "X/Xosdefs.h"

#ifdef X_USEBFUNCS
void bcopy();
void bzero();
int bcmp();
#else
#if (__STDC__ && !defined(X_NOT_STDC_ENV) && !defined(sun) && !defined(macII)) || defined(SVR4) || defined(hpux) || defined(_AIX)
#include <string.h>
#define bcopy(b1,b2,len) memmove(b2, b1, (size_t)(len))
#define bzero(b,len) memset(b, 0, (size_t)(len))
#define bcmp(b1,b2,len) memcmp(b1, b2, (size_t)(len))
#else
#ifdef sgi
#include <bstring.h>
#else
#ifdef SYSV
#include <memory.h>
#if defined(_XBCOPYFUNC) && !defined(macII)
#define bcopy _XBCOPYFUNC
#define _XNEEDBCOPYFUNC
#endif
void bcopy();
#define bzero(b,len) memset(b, 0, len)
#define bcmp(b1,b2,len) memcmp(b1, b2, len)
#else /* bsd */
void bcopy();
void bzero();
int bcmp();
#endif /* SYSV */
#endif /* sgi */
#endif /* __STDC__ and relatives */
#endif /* X_USEBFUNCS */

#endif /* _XFUNCS_H_ */
