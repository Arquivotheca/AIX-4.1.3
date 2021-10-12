static char sccsid[] = "@(#)50	1.15  src/bos/usr/bin/que/aix/qpri.c, cmdque, bos411, 9428A410j 12/16/93 15:46:30";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: qpri
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
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


#include "qpri_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_QPRI,num,str)

/*----Misc. */
#define QMSGUSE1 "-#JobNumber -aPriority"
#define QMSGUSE2 "Prioritizing a job in the print queue."

/*----Input arguments */
#define OURARGS "#:a:"
#define ARGJOBN '#'
#define ARGALTR 'a'
#define ARGDASH '-'
#define BIT_ARGJOBN 0x1
#define BIT_ARGALTR 0x2

char	*progname = "qpri";


/*====MAIN PROGRAM MODULE */
main(argc,argv)
int	argc;
char 	**argv;
{
	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_QPRI, NL_CAT_LOCALE);

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
	int		ra_numparms;
	char		state = 0;

	/*----Set the first argument to ENQ program name */
	ra_outcnt = 0;
	ra_outargs[ra_outcnt++] = ENQPATH;
	ra_numparms = 0;

	/*----Check for not enough arguments */
	if(ra_argc == 1)
		usage();

	/*----Get all the arguments and pass through */
	while((ra_thisarg = getopt(ra_argc,ra_argv,OURARGS)) != EOF)
		switch(ra_thisarg) {

		case ARGJOBN:
	        case ARGALTR:
			if (ra_thisarg == ARGJOBN)
				state |= BIT_ARGJOBN;
			else	
				state |= BIT_ARGALTR;
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
			ra_numparms++;
			break;

		default:
			/*----Anything else is an error */
			usage();
			break;
		}	/* case */

	/*----Check for errors */
	if((ra_argv[optind] != NULL) ||
	   (ra_numparms < 2))
		usage();
 
	if(state != (BIT_ARGJOBN | BIT_ARGALTR))
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
