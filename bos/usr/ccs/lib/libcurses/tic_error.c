static char sccsid[] = "@(#)98  1.2  src/bos/usr/ccs/lib/libcurses/tic_error.c, libcurses, bos411, 9435A411a 8/25/94 15:02:26";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: Error message routines
 *
 *   ORIGINS: 4, 27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)curses:screen/tic_error.c	1.4" */
/*********************************************************************
*                         COPYRIGHT NOTICE                           *
**********************************************************************
*        This software is copyright (C) 1982 by Pavel Curtis         *
*                                                                    *
*        Permission is granted to reproduce and distribute           *
*        this file by any means so long as no fee is charged         *
*        above a nominal handling fee and so long as this            *
*        notice is always included in the copies.                    *
*                                                                    *
*        Other rights are reserved except as explicitly granted      *
*        by written permission of the author.                        *
*                Pavel Curtis                                        *
*                Computer Science Dept.                              *
*                405 Upson Hall                                      *
*                Cornell University                                  *
*                Ithaca, NY 14853                                    *
*                                                                    *
*                Ph- (607) 256-4934                                  *
*                                                                    *
*                Pavel.Cornell@Udel-Relay   (ARPAnet)                *
*                decvax!cornell!pavel       (UUCPnet)                *
*********************************************************************/

/*
 *	tic_error.c -- Error message routines
 *
 *  $Log:	RCS/tic_error.v $
 * Revision 2.1  82/10/25  14:45:31  pavel
 * Added Copyright Notice
 * 
 * Revision 2.0  82/10/24  15:16:32  pavel
 * Beta-one Test Release
 * 
 * Revision 1.3  82/08/23  22:29:31  pavel
 * The REAL Alpha-one Release Version
 * 
 * Revision 1.2  82/08/19  19:09:44  pavel
 * Alpha Test Release One
 * 
 * Revision 1.1  82/08/12  18:36:02  pavel
 * Initial revision
 * 
 *
 */

#include "compiler.h"
#include <varargs.h>
#include <locale.h>
#include "tic_msg.h"

#define MSGSTR(num,str) catgets(catd,MS_TIC,num,str)
extern nl_catd catd;

extern char *string_table;
extern short term_names;

/* VARARGS1 */
warning(va_alist)
va_dcl
{
    va_list args;
    char *fmt;

    va_start(args);
    fmt = va_arg(args, char *);
    fprintf(stderr, MSGSTR(WARNING,
	"tic: Warning near line %d in terminal '%s':\n"),
	curr_line, string_table+term_names);
    vfprintf (stderr, fmt, args);
    va_end(args);
}


/* VARARGS1 */
err_abort(va_alist)
va_dcl
{
    va_list args;
    char *fmt;
    
    va_start(args);
    fmt = va_arg(args, char *);
    fprintf(stderr, MSGSTR(ERRABORT, "tic: Line %d in terminal '%s':\n"),
	curr_line, string_table+term_names);
    vfprintf (stderr, fmt, args);
    va_end(args);
    exit(1);
}


/* VARARGS1 */
syserr_abort(va_alist)
va_dcl
{
    va_list args;
    char *fmt;
    
    va_start(args);
    fmt = va_arg(args, char *);
    fprintf(stderr, MSGSTR(SYSABORT,
	"tic: PROGRAM ERROR: Line %d in terminal '%s':\n"),
	curr_line, string_table+term_names);
    vfprintf (stderr, fmt, args);
    fprintf(stderr, MSGSTR(CORRUPT, 
	"*** Possibly corrupted terminfo file ***\n"));
    va_end(args);
    exit(1);
}
