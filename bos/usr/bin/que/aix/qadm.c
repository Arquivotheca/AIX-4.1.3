static char sccsid[] = "@(#)47	1.18  src/bos/usr/bin/que/aix/qadm.c, cmdque, bos411, 9428A410j 1/26/94 09:15:16";
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
#include <time.h>
#include <locale.h>
#include "common.h"
#include "frontend.h"


#include "qadm_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_QADM,num,str)

/*----Misc. */
#define QMSGUSE1 "[-G] | [-X<printer>] | [-D<printer>] | [-K<printer>] | [-U<printer>]"
#define QMSGUSE2 "Configures print queues and queue devices."

/*----Input arguments */
#define OURARGS "GX:D:K:U:"
#define ARGDIEG 'G'
#define ARGCANA 'X'
#define ARGDWNQ 'D'
#define ARGKILQ 'K'
#define ARGUPQU 'U'
#define ARGDASH '-'
#define ARGP 	'P'

char	*progname = "qadm";


/*====MAIN PROGRAM MODULE */
main(argc,argv)
int	argc;
char 	**argv;
{
	char **ppTmp;

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_QADM, NL_CAT_LOCALE);

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

	/*----Check for no args */
	if(ra_argc == 1)
		usage();

	/*----Get all the arguments and pass through */
	while((ra_thisarg = getopt(ra_argc,ra_argv,OURARGS)) != EOF)
		switch(ra_thisarg) {

		case ARGDIEG:
			/*----Construct the parameter string */
			ra_newstr = (char *)Qalloc(3);
			ra_newstr[0] = ARGDASH;
			ra_newstr[1] = ra_thisarg;
			ra_newstr[2] = '\0';
			ra_outargs[ra_outcnt++] = ra_newstr;
			break;

		case ARGCANA:
		case ARGDWNQ:
		case ARGKILQ:
		case ARGUPQU:
			/*----Construct the parameter string */
			/*
			 *  i.e.  enq -X -P QueueName
			 */
			ra_newstr = (char *)Qalloc(3);
			ra_newstr[0] = ARGDASH;
			ra_newstr[1] = ra_thisarg;
			ra_newstr[2] = '\0';
			ra_outargs[ra_outcnt++] = ra_newstr;

			/*----Construct the argument string */ 
			ra_newstr = (char *)Qalloc(sizeof(ARGP) + strlen(optarg) + 1 );
			strcpy(ra_newstr,"-P");
			strcat(ra_newstr,optarg);
			ra_outargs[ra_outcnt++] = ra_newstr;
			break;
		default:
			/*----Anything else is an error */
			usage();
			break;
		}	/* case */

	/*----Check for bad additional arguments */
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
