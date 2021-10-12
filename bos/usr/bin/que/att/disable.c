static char sccsid[] = "@(#)88	1.17  src/bos/usr/bin/que/att/disable.c, cmdque, bos411, 9428A410j 12/16/93 15:40:02";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: disable
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <IN/standard.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include "common.h"
#include "frontend.h"


#include "disabl_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_DISABL,num,str)

/*----Misc. */
#define QMSGUSE1 "[-c] [-rReason] PrinterName ..."
#define QMSGUSE2 "Disables printers."

/*----Input arguments */
#define ARGCANC 'c'
#define ARGREAS 'r'
#define ARGDASH '-'

/*----Output arguments */
#define OUTDISA "-D"
#define OUTKILL "-K"
#define OUTPRNT "-P"

char	*progname = "disable";


/*====MAIN PROGRAM MODULE */
main(argc,argv)
int	argc;
char 	**argv;
{
	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_DISABL, NL_CAT_LOCALE);

	/*----This is not qdaemon */
	qdaemon = FALSE;

	/*----Get the arguments to LPR, xlate for ENQ */
	read_args(argc,argv,outargs);
}

/*====READ ALL ARGUMENTS TO LPR AND XLATE TO NEW ARG LIST FOR ENQ:QSTATUS */
read_args(ra_argc,ra_argv,ra_outargs)
int		ra_argc;
char 		**ra_argv;
char		*ra_outargs[];
{
	char		*ra_thisarg;
	char		*ra_killarg;
	boolean		ra_err;
	int		ra_outcnt;
	int		ra_incnt;
	boolean		did_something = FALSE;

	/*----Set the first argument to ENQ program name */
	ra_outcnt = 0;
	ra_outargs[ra_outcnt++] = ENQPATH;
	ra_killarg = OUTDISA;		/* default argument is to die gracefully */

	/*----Handle case of no arguments */
	if(ra_argc == 1)
		usage();

	/*----Look for all arguments */
	ra_err = FALSE;
	for(ra_incnt = 1;
	    (ra_argv[ra_incnt] != NULL) && (ra_incnt < ra_argc);
	    ra_incnt++)
	{
		ra_thisarg = ra_argv[ra_incnt];
		switch(ra_thisarg[0]) {

		/*----Look at stuff starting with '-' */
		case ARGDASH:
			switch(ra_thisarg[1]) {

			/*----Cancel presently running jobs */
			case ARGCANC:
				ra_killarg = OUTKILL;
				break;

			/*----Reason for bringing down */
			case ARGREAS:
				/* nop */
				break;
					
			/*----Other spurious input */
			default:
				ra_err = TRUE;
				break;
			}	/* inner case */
			break;

		/*----Handle printer names */
		default:
			if(ra_err == FALSE)
			{
				/*----Set up the parms */
				ra_outargs[ra_outcnt] = ra_killarg;
				ra_outargs[ra_outcnt + 1] = OUTPRNT;
				ra_outargs[ra_outcnt + 2] = ra_argv[ra_incnt];
				ra_outargs[ra_outcnt + 3] = NULL;

				did_something = TRUE;

				/*----Execute ENQ with xlated arguments */
				exe_enq(ra_outargs);
			}
			break;
		}	/* outer case */

		/*----If bad argument found */
		if (ra_err == TRUE)
			/* getopt already spit out an error message */
			usage();
	} /* for */

	/*----Set end of argument list */
	ra_outargs[ra_outcnt] = NULL;
	if (! did_something)
	    usage();
	return(0);
}

/*====PRINT USAGE MESSAGE AND EXIT */
usage()
{
	sysuse( TRUE,
		MSGSTR(MSGUSE1,QMSGUSE1),
		MSGSTR(MSGUSE2,QMSGUSE2),
		(char *)0
	      );
}
