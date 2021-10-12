static char sccsid[] = "@(#)32	1.5  src/bos/usr/ccs/lib/libPW/fdfopen.c, libPW, bos411, 9428A410j 6/16/90 00:56:10";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: fdfopen
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

# include	"stdio.h"
# include	"sys/types.h"
# include	"macros.h"

/*
 * FUNCTION: Return FILE * for file descriptor argument
 *
 * RETURN VALUE DESCRIPTIONS:
 *	Returns file pointer on success,
 *	NULL on failure (fd not open or no file structures available).
 */
/*
	Interface to stdio
	First arg is file descriptor, second is read/write mode (0/1).
*/


FILE *
fdfopen(fd, mode)
register int fd, mode;
{
	register FILE *iop;

	if (fstat(fd, &Statbuf) < 0)
		return(NULL);
	for (iop = _iob; iop->_flag&(_IOREAD|_IOWRT); iop++)
		if (iop >= &_iob[_NFILE-1])
			return(NULL);
	iop->_flag &= ~(_IOREAD|_IOWRT);
	iop->_file = fd;
	if (mode)
		iop->_flag |= _IOWRT;
	else
		iop->_flag |= _IOREAD;
	return(iop);
}
