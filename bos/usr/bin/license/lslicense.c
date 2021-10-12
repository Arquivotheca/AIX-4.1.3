static char sccsid[] = "@(#)69	1.1.1.2  src/bos/usr/bin/license/lslicense.c, cmdlicense, bos411, 9428A410j 4/4/94 15:50:30";
/*
 * COMPONENT_NAME: CMDLICENSE
 *
 * FUNCTIONS: main
 *
 * ORIGIN: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/license.h>
#include "license_msg.h"

extern	char	*command_name;
extern	nl_catd	scmc_catd;       /* for messages */

extern	int	list_max_users(int);


/*
 * NAME:     lslicense (main)
 *
 * FUNCTION: Displays the number of fixed licenses and the status of
 *           floating licensing.
 *
 * EXECUTION ENVIRONMENT:
 *
 *           Independent program with command line arguments,
 *           usually invoked by a user from a shell or smit.
 *
 * NOTES:    This program can be invoked as:
 *
 *                   lslicense [-c]
 *
 *           The -c option will produce a colon output.  This output is
 *           needed for the use with the SMIT command.
 *		  
 * RETURNS:  0 if command completes successfully. A positive value
 *           is returned if an error is encountered.
 */
int
main(int argc, char *argv[], char *envp)
{
	int	flag;           /* Flag from getopt                           */
	int	flag_error;     /* If the user passed in a bad flag           */
	int	do_colon_output;/* TRUE = colon output, FALSE = normal output */

#ifndef R5A
	setlocale(LC_ALL, "");
	scmc_catd = catopen(CATFILE, NL_CAT_LOCALE);
#endif

	command_name    = argv[0];
	flag_error      = 0;
	do_colon_output = FALSE;

	if (argc > 2) flag_error++;

	while ((flag = getopt(argc, argv, "c")) != EOF)
	{
		switch(flag)
		{
		    case 'c':
			/*
			 * Display in colon output format.
			 */
			do_colon_output = TRUE;
			break;
		    default:
			flag_error++;
		}
	}

	if (flag_error)
	{
		/*
		 * Print the correct syntax.
		 */
		fprintf(stderr, catgets(scmc_catd, LIST_SET, LIST_USAGE,
			"Usage: lslicense [-c]\n\
\tDisplays the number of fixed licenses and the status\n\
\tof floating licensing.\n"));
		exit(1);
	}

	/*
	 * Print the current number of fixed licenses.
	 */
	if (list_max_users(do_colon_output) < 0)
		exit(2);

	exit(0);
}
