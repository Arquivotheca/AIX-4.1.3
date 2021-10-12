static char sccsid[] = "@(#)29  1.12  src/bos/usr/bin/tty/tty.c, cmdstat, bos41B, 9504A 12/21/94 13:41:33";
/*
 * COMPONENT_NAME: (CMDSTAT) status
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 71, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */


/**********************************************************************/
/* Include File                                                       */
/**********************************************************************/
#include  <stdio.h>
#include  <unistd.h>
#include  <locale.h>
#include  "tty_msg.h"

/**********************************************************************/
/* Constant Definition / Macro Function                               */
/**********************************************************************/
#define  ISTERM   0
#define  NOTTERM  1
#define  ERROR    2

#define  MSGSTR(Num,Str)  catgets(catd,MS_TTY,Num,Str)

/**********************************************************************/
/* Function Prototype Declaration                                     */
/**********************************************************************/

/**********************************************************************/
/* Global / External Variables                                        */
/**********************************************************************/
extern int	optind;				/* Used by getopt     */

static int		 sflg = FALSE;			/* s flag             */
static nl_catd	 catd;					/* Catalog descriptor */

/**********************************************************************/
/* Name: main                                                         */
/* Function: Returns user's terminal name.                            */
/* Return Value: 0  Standard input is a terminal.                     */
/*               1  Standard input is not a terminal.                 */
/*              >1  An error occured.                                 */
/**********************************************************************/
int  main(argc, argv)
int  argc;
char *argv[];
{
	register char  *p;
	register int    i;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_TTY,NL_CAT_LOCALE);

	while( (i = getopt(argc, argv, "s?")) != EOF ) {
		switch( i ) {
		  case 's':
			sflg = TRUE;
			break;
		  case '?':
			fprintf( stderr, MSGSTR(USAGE, "Usage: tty [-s]\n") );
			exit(ERROR);
		}
	}

	p = ttyname(0);

	if ( sflg == FALSE )
		printf( "%s\n", p ? p : MSGSTR(NOTTTY,"not a tty") );

	exit( p ? ISTERM : NOTTERM );
}
