#ifndef lint
static char sccsid[] = "@(#)11	1.3 src/bos/usr/ccs/bin/make/getopt.c, cmdmake, bos41B, 9504A 12/23/94 12:44:00";
#endif /* lint */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: ERR
 *		getopt
 *		
 *
 *   ORIGINS: 27,85
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: getopt.c,v $ $Revision: 2.8.4.4 $ (OSF) $Date: 1992/12/11 19:22:02 $";
#endif

#include <stdio.h>
#include <string.h>
#include <nl_types.h>
#include "libc_msg.h"

#define ERR(s, c)	if (opterr){\
	(void) fputs(argv[0], stderr);\
	(void) fputs(s, stderr);\
	(void) fputc(c, stderr);\
	(void) fputc('\n', stderr);}

static int	opterr = 1;		/* print errors?			*/
int	optind = 1;		/* index of next argv			*/
static int	optopt;			/* current option we're looking at	*/
char	*optarg;		/* argument for current option		*/


/*
 * NAME:	getopt
 *
 * FUNCTION:	get flag letters from the argument vector
 *
 * NOTES:	Getopt scans the command line looking for specified flags.
 *		The flags may or may not require arguments.
 * 		If option followed by a ';', argument has optional param.
 *
 * 		Make uses it own version of getopt() because the AIX libc
 *		version does not support the optional parameter feature. This
 *		getopt() is from the OSF 1.2 libc. Once AIX supports the
 *		optional parameter, this module can be removed from make.
 *
 * return  VALUE DESCRIPTION:	returns '?' on an error (unrecognized flag or
 *		a recognized flag requiring an argument and no argument given),
 *              -1 on successful completion of the scan of the command line,
 *		else the recognized flag just encountered
 */  

int getopt( int	 argc,		/* number of command line arguments */
	    char * const *argv,		/* pointer to command line arguments */
	    char const *optstring)	/* string describing valid flags */
{
	int	c;
	char	*cp;
	nl_catd	catd;
	static int stringind = 1;

	optarg = "";		/* Defend against VSX4 test problems */

	if (stringind == 1) {
		if (optind >= argc || argv[optind] == NULL ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0') {
			return (-1);
		} else if (strcmp(argv[optind], "--") == 0) {
			optind++;
			return (-1);
		}
	}
	optopt = c = argv[optind][stringind];
	if (c == ':' || (cp = strchr(optstring, c)) == NULL) {
		/* check for illegal options */
		if (optstring[0] != ':') {
			catd = catopen(MF_LIBC, NL_CAT_LOCALE);
			ERR(catgets(catd, MS_LIBC, M_OPTILL, 
			    ": illegal option -- "), c);
			catclose (catd);
		}
		if (argv[optind][++stringind] == '\0') {
			optind++;
			stringind = 1;
		}
		return ('?');
	}
	if (*++cp == ':') {		/* parameter is needed */
		/* no blanks to separate option and parameter */
		if (argv[optind][stringind+1] != '\0')
			optarg = (char *) &argv[optind++][stringind+1];
		else if (++optind >= argc) {
			optarg = (char *) argv[optind];
			stringind = 1;
			if (optstring[0] != ':') {
				catd = catopen(MF_LIBC, NL_CAT_LOCALE);
				ERR(catgets(catd, MS_LIBC, M_OPTARG,
				    ": option requires an argument -- "), c);
				catclose(catd);
				return ('?');
			}
			return (':');
		} else
			optarg = (char *) argv[optind++];

		stringind = 1;
	/*
	 * OSF extention - ';' means optional parameter
	 * must follow option with no whitespace.
	 * To support obsolete syntax in the commands
	 */
	} else if (*cp == ';') {		/* optional parameter */
		if (argv[optind][stringind+1] != '\0')
			optarg = (char *) &argv[optind][stringind+1];
		else 
			optarg = NULL;
		stringind = 1;
		optind++;
	} else {			/* parameter not needed */
		/* if c is the last option update optind */
		if (argv[optind][++stringind] == '\0') {
			stringind = 1;
			optind++;
		}
		optarg = NULL;
	}
	return (c);
}



