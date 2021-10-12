/* @(#)59       1.1  src/bos/usr/ccs/lib/libcurses/print.h, libcurses, bos411, 9428A410j 9/3/93 15:11:04 */
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 4
 *
 *                    SOURCE MATERIALS
 */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)curses:screen/print.h	1.2" */

/* externs from iexpand.c, cexpand.c */
extern void tpr();
extern int cpr();
extern char *cexpand(), *iexpand(), *cconvert(), *rmpadding();

/* externs from print.c */
enum printtypes
    {
    pr_none,
    pr_terminfo,		/* print terminfo listing */
    pr_cap,			/* print termcap listing */
    pr_longnames		/* print C variable name listing */
    };

extern void pr_onecolumn();
extern void pr_caprestrict();
extern void pr_width();
extern void pr_init();
extern void pr_heading();
extern void pr_bheading();
extern void pr_boolean();
extern void pr_bfooting();
extern void pr_nheading();
extern void pr_number();
extern void pr_nfooting();
extern void pr_sheading();
extern void pr_string();
extern void pr_sfooting();
