static char sccsid[] = "@(#)68  1.1  src/bldenv/sbtools/libsb/vfprintf.c, bldprocess, bos412, GOLDA411a 4/29/93 12:25:41";
/*
 * Copyright (c) 1990, 1991, 1992  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * ODE 2.1.1
 */
/*
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <varargs.h>

#if	!_DOPRNT_IS_VISIBLE
/* 
 *  No new architectures make _doprnt() visible.
 */
#define	_doprnt	_doprnt_va
#endif

int
vfprintf(iop, fmt, ap)
	FILE *iop;
	char *fmt;
	va_list ap;
{
	int len;
#ifdef OS_HP_UX
	unsigned char localbuf[BUFSIZ];
#else
	char localbuf[BUFSIZ];
#endif

	if (iop->_flag & _IONBF) {
		iop->_flag &= ~_IONBF;
		iop->_ptr = iop->_base = localbuf;
		len = _doprnt(fmt, ap, iop);
		(void) fflush(iop);
		iop->_flag |= _IONBF;
		iop->_base = NULL;
#ifndef NO_FILE_BUFSIZE
		iop->_bufsiz = 0;
#endif
		iop->_cnt = 0;
	} else
		len = _doprnt(fmt, ap, iop);

	return (ferror(iop) ? EOF : len);
}
