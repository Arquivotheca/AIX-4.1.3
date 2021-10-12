static char sccsid[] = "@(#)97  1.1  src/bos/usr/ccs/lib/libcurses/tic_captab.c, libcurses, bos411, 9428A410j 9/3/93 15:13:43";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS:  Capabilities table declarations.
 *
 *   ORIGINS: 4
 *
 */
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)curses:screen/tic_captab.c	1.3" */
/*
 *	tic_captab.c -- The names of the capabilities in a form ready for
 *		        the making of a hash table for the compiler.
 *
 */


#include "curses_inc.h"
#include "compiler.h"


struct name_table_entry	cap_table[512];

struct name_table_entry *cap_hash_table[360];

int	Hashtabsize = 360;
int	Captabsize = 0;
int	BoolCount = 0;
int	NumCount = 0;
int	StrCount = 0;
