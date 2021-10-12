/* @(#)49	1.10  src/bos/usr/include/dbm.h, libdbm, bos411, 9428A410j 2/22/94 09:43:38 */
/* 
 * COMPONENT_NAME: (LIBDBM) Data Base Management Library
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	(#)dbm.h	5.1 (Berkeley) 3/27/86
 */

#ifndef _H_DBM
#define _H_DBM

#ifndef NULL
/*
 * this is lunacy, we no longer use it (and never should have
 * unconditionally defined it), but, this whole file is for
 * backwards compatability - someone may rely on this.
 */
#define	NULL    0
#endif

#include <ndbm.h>

datum	fetch();
datum	firstkey();
datum	nextkey();
#if 0
datum	makdatum();
datum	firsthash();
long	calchash();
long	hashinc();
#endif
#endif /* _H_DBM */
