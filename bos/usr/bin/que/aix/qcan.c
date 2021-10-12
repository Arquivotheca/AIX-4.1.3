static char sccsid[] = "@(#)48	1.16  src/bos/usr/bin/que/aix/qcan.c, cmdque, bos411, 9428A410j 1/27/94 14:08:34";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: qadm
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <IN/standard.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <time.h>
#include "common.h"
#include "frontend.h"

#include "qcan_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_QCAN,num,str)

/*----Misc. */
#define QMSGUSE1 "[-X [-PPrinter]] | [-xJobNumber]"
#define QMSGUSE2 "Cancels a print job."

/*----Input arguments */
#define OURARGS "XP:x:"
#define ARGCANA 'X'
#define ARGPRNT 'P'
#define ARGCANC 'x'
#define ARGDASH '-'

char	*progname = "qcan";


/*====MAIN PROGRAM MODULE */
main(argc,argv)
int	argc;
char 	**argv;
{
	char **ppTmp;

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_QCAN, NL_CAT_LOCALE);

	/*----This is not qdaemon */
	qdaemon = FALSE;

	/*----Get the arguments to LPR, xlate for ENQ */
	read_args(argc,argv,outargs);

	/*----Execute ENQ with xlated arguments */
	exe_enq(outargs);
}

/*====READ ALL ARGUMENTS AND XLATE TO NEW ARG LIST FOR ENQ */
read_args(ra_argc,ra_argv,ra_outargs)
int		ra_argc;
char 		**ra_argv;
char		*ra_outargs[];
{
	extern char	*optarg;
	extern int	optind;

	int		ra_thisarg;
	char		*ra_newstr;
	int		ra_outcnt;
	boolean		gotCancelArg;  /* True if -x or -X is given */

	/*----Set the first argument to ENQ program name */
	ra_outcnt = 0;
	ra_outargs[ra_outcnt++] = ENQPATH;

	/*----Check for no parameters */
	if(ra_argc == 1)
		usage();

	gotCancelArg = FALSE;
	/*----Get all the arguments and pass through */
	while((ra_thisarg = getopt(ra_argc,ra_argv,OURARGS)) != EOF) {
		switch(ra_thisarg) {

		case ARGCANA: /* -X */
			/*----Construct the parameter string */
			ra_newstr = (char *)Qalloc(3);
			ra_newstr[0] = ARGDASH;
			ra_newstr[1] = ra_thisarg;
			ra_newstr[2] = '\0';
			ra_outargs[ra_outcnt++] = ra_newstr;
			gotCancelArg = TRUE;
			break;
			
		case ARGCANC:   /* -x */
			gotCancelArg = TRUE;

		case ARGPRNT:   /* -P */
			/*----Construct the parameter string */
			ra_newstr = (char *)Qalloc(3);
			ra_newstr[0] = ARGDASH;
			ra_newstr[1] = ra_thisarg;
			ra_newstr[2] = '\0';
			ra_outargs[ra_outcnt++] = ra_newstr;

			/*----Construct the argument string */ 
			ra_newstr = (char *)Qalloc(strlen(optarg) + 1);
			strcpy(ra_newstr,optarg);
			ra_outargs[ra_outcnt++] = ra_newstr;
			break;

		default:
			/*----Anything else is an error */
			usage();
			break;
		}	/* case */
	}
	/*
	 * Make sure we send at least one cancel argument, or
	 * enq will not know what to do.
	 */
	if ( FALSE == gotCancelArg ) {
		/*----Construct the parameter string */
		ra_newstr = (char *)Qalloc(3);
		ra_newstr[0] = ARGDASH;
		ra_newstr[1] = ARGCANA; /* X */
		ra_newstr[2] = '\0';
		ra_outargs[ra_outcnt++] = ra_newstr;
	}

	/*----Check for bad additional args */
	if(ra_argv[optind] != NULL)
		usage();

	/*----Set the end of the argument list */
	ra_outargs[ra_outcnt] = NULL;
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
