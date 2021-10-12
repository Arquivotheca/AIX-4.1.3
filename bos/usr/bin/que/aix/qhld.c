static char sccsid[] = "@(#)59	1.2  src/bos/usr/bin/que/aix/qhld.c, cmdque, bos411, 9428A410j 12/16/93 15:46:45";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: qhld
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

#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <time.h>
#include "common.h"
#include "frontend.h"


#include "qhld_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_QHLD,num,str)

/*----Misc. */
#define QMSGUSE1 "[-r] {-#<job number> [-P<queue>] | -P<queue> |"
#define QMSGUSE2 " -u<user> [-P<queue>]}"
#define QMSGUSE3 "Hold or release a job in the queue."

/*----Input arguments */
#define OURARGS "#:P:u:r"
#define ARGJOBN '#'
#define ARGPRNT 'P'
#define ARGUSER 'u'
#define ARGRELE 'r'
#define ARGDASH '-'

/*----Output arguments */
#define OUTHOLD 'h' 
#define OUTRELE 'p'

char	*progname = "qhld";


/*====MAIN PROGRAM MODULE */
main(argc,argv)
int	argc;
char 	**argv;
{
	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_QHLD, NL_CAT_LOCALE);

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
	boolean		release = FALSE;
	boolean		other = FALSE;

	/*----Set the first argument to ENQ program name */
	ra_outcnt = 0;
	ra_outargs[ra_outcnt++] = ENQPATH;

	/*----Check for no args */
	if(ra_argc == 1)
		usage();

	/*----Get all the arguments and pass through */
	while((ra_thisarg = getopt(ra_argc,ra_argv,OURARGS)) != EOF)
		switch(ra_thisarg) {

		case ARGJOBN:
		case ARGUSER:
		case ARGPRNT:
			/*----Construct the parameter string */
			other = TRUE;
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

		case ARGRELE:
			/*----Set release flag */
			release = TRUE;
			ra_newstr = (char *)Qalloc(3);
			ra_newstr[0] = ARGDASH;
			ra_newstr[1] = OUTRELE;
			ra_newstr[2] = '\0'; 	
			ra_outargs[ra_outcnt++] = ra_newstr;
			break;

		default:
			/*----Anything else is an error */
			usage();
			break;
		}	/* case */

	/*----Check for errors */
	if(ra_argv[optind] != NULL) 
		usage();

	if( release && !(other))
		usage();

	if (!release)
	{
		ra_newstr = (char *)Qalloc(3);
		ra_newstr[0] = ARGDASH;
		ra_newstr[1] = OUTHOLD;
		ra_newstr[2] = '\0';
		ra_outargs[ra_outcnt++] = ra_newstr;
	}
 

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
