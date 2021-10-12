static char sccsid[] = "@(#)94	1.20  src/bos/usr/bin/bsh/echo.c, cmdbsh, bos411, 9428A410j 9/1/93 17:31:18";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: echo
 *
 * ORIGINS: 3, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
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
 *
 * 1.14  com/cmd/sh/sh/echo.c, , bos320, 9134320 8/12/91 14:57:42
 *
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 * 
 *
 * OSF/1 1.1
 */

#include	"defs.h"
#include	<stdlib.h>

#define	echoexit(a)	flushb();return(a)

extern int exitval;

/*	
 *	Echo() argument "argv" is decoded.
 */

echo(argc, argv)
uchar_t **argv;
{
	register uchar_t *cp;
	register int	i, wd;
	int	j;
	
	if(--argc == 0) {
		prc_buff('\n');
		echoexit(0);
	}

	for(i = 1; i <= argc; i++) 
	{
		sigchk();
		for(cp = argv[i]; *cp; cp++) 
		{
                        int     mblength = mblen((char *)cp, MB_CUR_MAX);
                        if (mblength > 1) {
                                while(--mblength){
                                        prc_buff (*cp);
                                        cp++;
                                }
			} else
			if(*cp == '\\')
			switch(*++cp) 
			{
				case 'b':
					prc_buff('\b');
					continue;

				case 'c':
					echoexit(0);

				case 'f':
					prc_buff('\f');
					continue;

				case 'n':
					prc_buff('\n');
					continue;

				case 'r':
					prc_buff('\r');
					continue;

				case 't':
					prc_buff('\t');
					continue;

				case 'v':
					prc_buff('\v');
					continue;

				case '\\':
					prc_buff('\\');
					continue;
				case '0':
					j = wd = 0;
					while ((*++cp >= '0' && *cp <= '7') && j++ < 3) {
						wd <<= 3;
						wd |= (*cp - '0');
					}
					prc_buff(wd);
					--cp;
					continue;

				default:
					cp--;
			}
			prc_buff(*cp);
		}
                /*
                 * space between arguments
                 */
                if (i < argc)
                        prc_buff(' ');
	}
        /*
         * ending newline
         */
        prc_buff('\n');
	echoexit(0);
}

