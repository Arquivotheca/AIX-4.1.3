static char sccsid[] = "@(#)61	1.2  src/bos/usr/bin/que/aix/qmov.c, cmdque, bos411, 9428A410j 12/16/93 15:46:54";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: qmov
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
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


#include "qmov_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_QMOV,num,str)

/*----Misc. */
#define QMSGUSE1 "-m<queue moving to> {-#<job number> [-P<queue>] | -P<queue> |"
#define QMSGUSE2 "-u<user> [-P<queue>]}"
#define QMSGUSE3 "Move jobs from one queue to another."

/*----Input arguments */
#define OURARGS "#:P:m:u:"
#define ARGJOBN '#'
#define ARGPRNT 'P'
#define ARGUSER 'u'
#define ARGMOVE 'm'
#define ARGDASH '-'

/*----Output arguments */
#define OUTMOVE 'Q' 

char	*progname = "qmov";


/*====MAIN PROGRAM MODULE */
main(argc,argv)
int	argc;
char 	**argv;
{
	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_QMOV, NL_CAT_LOCALE);

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
	boolean		move = FALSE;
	boolean		other = FALSE;

	/*----Set the first argument to ENQ program name */
	ra_outcnt = 0;
	ra_outargs[ra_outcnt++] = ENQPATH;

	/*----Get all the arguments and pass through */
	while((ra_thisarg = getopt(ra_argc,ra_argv,OURARGS)) != EOF)
		switch(ra_thisarg) {

		case ARGJOBN:
		case ARGUSER:
		case ARGPRNT:
		case ARGMOVE:
			/*----Construct the parameter string */
			ra_newstr = (char *)Qalloc(3);
			ra_newstr[0] = ARGDASH;
			if(ra_thisarg == ARGMOVE){
				move = TRUE;
				ra_newstr[1] = OUTMOVE;
 			}
			else
			{
				other = TRUE;
				ra_newstr[1] = ra_thisarg;
			}
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

	/*----Check for errors */
	if(move && other){
		/*----Set the end of the argument list */
		ra_outargs[ra_outcnt] = NULL;
	 	return(0);
	}
	else
		usage();	
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
