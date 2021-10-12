static char sccsid[] = "@(#)49	1.15  src/bos/usr/bin/que/aix/qchk.c, cmdque, bos411, 9428A410j 1/26/94 09:15:41";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: qchk
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
#include <time.h>
#include <locale.h>
#include "common.h"
#include "frontend.h"


#include "qchk_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_QCHK,num,str)

/*----Misc. */
#define QMSGUSE1 "[-ALq] [-P<qname>] [-#<job number>]"
#define QMSGUSE2 "[-u<username>] [-w<delaysec>]"
#define QMSGUSE3 "Shows the status of a queue and lists all queue devices."

/*----Input arguments */
#define OURARGS "ALqP:#:u:w:"
#define ARGALLQ 'A'
#define ARGVERB 'L'
#define ARGDEFQ 'q'
#define ARGPRNT 'P'
#define ARGJOBN '#'
#define ARGUSER 'u'
#define ARGLOOP 'w'
#define ARGDASH '-'

char	*progname = "qchk";


/*====MAIN PROGRAM MODULE */
main(argc,argv)
int	argc;
char 	**argv;
{
	char **ppTmp;

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_QCHK, NL_CAT_LOCALE);

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

	/*----Set the first argument to ENQ program name */
	ra_outcnt = 0;
	ra_outargs[ra_outcnt++] = ENQPATH;

	/*
	 * Always specify the -q option
	 * This will keep enq happy when qchk -P<printer> is used.
	 */
	ra_newstr = (char *)Qalloc(3);
	ra_newstr[0] = ARGDASH;
	ra_newstr[1] = ARGDEFQ;
	ra_newstr[2] = '\0';
	ra_outargs[ra_outcnt++] = ra_newstr;

	/*----Get all the arguments and pass through */
	while((ra_thisarg = getopt(ra_argc,ra_argv,OURARGS)) != EOF)
		switch(ra_thisarg) {

		case ARGALLQ:
		case ARGVERB:
		case ARGDEFQ:
			/*----Construct the parameter string */
			ra_newstr = (char *)Qalloc(3);
			ra_newstr[0] = ARGDASH;
			ra_newstr[1] = ra_thisarg;
			ra_newstr[2] = '\0';
			ra_outargs[ra_outcnt++] = ra_newstr;
			break;

		case ARGPRNT:
		case ARGJOBN:
		case ARGUSER:
		case ARGLOOP:
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

	/*----Check for bad additional parameters */
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
		MSGSTR(MSGUSE3,QMSGUSE3),
		(char *)0
	      );
}
