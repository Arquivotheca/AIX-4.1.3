static char sccsid[] = "@(#)61	1.6  src/bos/usr/ccs/lib/libc/getopt.c, libcenv, bos411, 9428A410j 3/21/94 12:48:42";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: getopt
 *
 * ORIGINS: 3 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994 
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

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */

#include <stdio.h>
#include <string.h>
#include <nl_types.h>
#include "libc_msg.h"

#define ERR(s, c)	if(opterr){\
	(void) fputs(argv[0], stderr);\
	(void) fputs(s, stderr);\
	(void) fputc(c, stderr);\
	(void) fputc('\n', stderr);}

#ifdef _THREAD_SAFE
#include <errno.h>
#define OPTIND			(*optind)
#define OPTOPT			(*optopt)
#define OPTARG			(*optarg)
#define SP			(*sp)
#define RETURN(val,rval)	{(*retopt) = (val); return(rval);}

#else /* _THREAD_SAFE */

int	opterr = 1;		/* print errors?			*/
int	optind = 1;		/* index of next argv			*/
int	optopt;			/* current option we're looking at	*/
char	*optarg;		/* argument for current option		*/

#define OPTIND			optind
#define OPTOPT			optopt
#define OPTARG			optarg
#define SP			sp
#define RETURN(val,rval)	return(val)

#endif /* _THREAD_SAFE */

/*
 * NAME:	getopt
 *                                                                    
 * FUNCTION:	get flag letters from the argument vector
 *                                                                    
 * NOTES:	Getopt scans the command line looking for specified flags.
 *		The flags may or may not require arguments.
 *
 * RETURN VALUE DESCRIPTION:	returns '?' on an error (unrecognized flag or
 *		a recognized flag requiring an argument and no argument given),
 *		-1 on successful completion of the scan of the command line,
 *		else the recognized flag just encountered
 */  
#ifdef _THREAD_SAFE
int getopt_r(int argc, char *const argv[], const char *optstring, int *optopt, char **optarg,
	   int *optind, int opterr, int *sp, int *retopt)
#else /* _THREAD_SAFE */
int getopt(int argc, char *const argv[], const char *optstring)
#endif /* _THREAD _SAFE */
{
	int c;
	char *cp;
	nl_catd	catd;
#ifndef _THREAD_SAFE
	static int sp = 1;
#else /* _THREAD_SAFE */
	if (optopt == NULL || optarg == NULL || optind == NULL ||
	    sp == NULL || retopt == NULL) {
	    errno = EINVAL;
	    RETURN(-1,-1);
	}

	/* First Call */
	if (SP == 0)
	    SP = 1;
	if (OPTIND == 0)
	    OPTIND = 1;
#endif /* _THREAD_SAFE */	

	if(SP == 1) {
		if(OPTIND >= argc || argv[OPTIND] == NULL ||
		   argv[OPTIND][0] != '-' || argv[OPTIND][1] == '\0') {
			RETURN(-1,0);
		}
		else if(strcmp(argv[OPTIND], "--") == 0) {
			OPTIND++;
			RETURN(-1,0);
		}
	}
	OPTOPT = c = argv[OPTIND][SP];
	if(c == ':' || (cp=strchr(optstring, c)) == NULL) {  
						/* check for illegal options */
		if (optstring[0] != ':') {
			catd=catopen("libc.cat", NL_CAT_LOCALE);
			ERR(catgets (catd, MS_LIBC, M_OPTILL,
			    ": illegal option -- "), c);
			catclose (catd);
		}
		if(argv[OPTIND][++SP] == '\0') {
			OPTIND++;
			SP = 1;
		}
		RETURN('?',-1);
	}
	if(*++cp == ':') {			 /* parameter is needed */
		if(argv[OPTIND][SP+1] != '\0')   /* no blanks to separate
						    option and parameter */
			OPTARG = &argv[OPTIND++][SP+1];
		else if(++OPTIND >= argc) {      /* see if parameter missing */
			SP = 1;
			OPTARG = NULL;
			if (optstring[0] != ':') {
				catd=catopen("libc.cat", NL_CAT_LOCALE);
				ERR(catgets (catd, MS_LIBC, M_OPTARG,
				": option requires an argument -- "), c);
				catclose(catd);
				RETURN('?',-1);
			}
			RETURN(':',0);
		} else
			OPTARG = argv[OPTIND++];
		SP = 1;
	} else { 				 /* parameter not needed */
		if(argv[OPTIND][++SP] == '\0') { /* if c is the last option
						    update optind        */
			SP = 1;
			OPTIND++;
		}
		OPTARG = NULL;
	}
	RETURN(c,0);
}
