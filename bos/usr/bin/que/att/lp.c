static char sccsid[] = "@(#)91	1.15.1.3  src/bos/usr/bin/que/att/lp.c, cmdque, bos411, 9428A410j 1/24/94 10:00:31";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: lp
 *
 * ORIGINS: 3, 27
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
 */

#include <IN/standard.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include "common.h"
#include "frontend.h"


#include "lp_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_LP,num,str)

/*----Misc */
#define QMSGUSE1 "[-cmsw] [-dDestination] [-nNumber] [-oOption]"
#define QMSGUSE2 "[-tTitle] [-] File ..."
#define QMSGUSE3 "Prints a file in a format suitable for sending to a line printer."

/*----Input arguments */
#define ARGCOPY 'c'
#define ARGDEST 'd'
#define ARGMAIL 'm'
#define ARGCOPS 'n'
#define ARGBKND 'o'
#define ARGSLNT 's'
#define ARGTITL 't'
#define ARGWRIT 'w'
#define ARGSTDI '\0'
#define ARGDASH	'-'

/*----Output arguments */
#define OUTCOPY "-c"
#define OUTDEST "-P"
#define OUTMAIL "-C"
#define OUTNOTF "-n"
#define OUTCOPS "-N"
#define OUTBKND "-o"
#define OUTTITL "-T"
#define OUTNOTF "-n"
#define OUTJBID "-j"

char	*progname = "lp";

extern int optind;
extern char * optarg;


/*====MAIN PROGRAM MODULE */
main(argc,argv)
int	argc;
char 	**argv;
{
	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_LP, NL_CAT_LOCALE);

	/*----This is not qdaemon */
	qdaemon = FALSE;

	/*----Get the arguments to LPR, xlate for ENQ */
	read_args(argc,argv,outargs);

#ifdef DEBUG
	{
	int i;
	printf ("%s: calling ", argv[0]);
	for (i=0; outargs[i]; i++)
		printf ("%s ", outargs[i]);
	printf ("\n");
	}
#endif
	/*----Execute ENQ with xlated arguments */
	exe_enq(outargs);
}

/*====READ ALL ARGUMENTS TO LPR AND XLATE TO NEW ARG LIST FOR ENQ:QSTATUS */
read_args(ra_argc,ra_argv,ra_outargs)
int		ra_argc;
char 		**ra_argv;
char		*ra_outargs[];
{
	int		ra_outcnt;
	int		ra_incnt;
	int		c;
	boolean		ra_jobid;

	/*----Set the first argument to ENQ program name */
	ra_outcnt = 0;
	ra_outargs[ra_outcnt++] = ENQPATH;

	/*----Look for all arguments */
	ra_jobid = TRUE;

	while ((c = getopt(ra_argc, ra_argv,
				"cd:mn:o:st:w")) != EOF)
	{
		switch(c)
		{
			/*----Copy file to destination */
			case ARGCOPY:
				ra_outargs[ra_outcnt++] = OUTCOPY;
				break;

			/*----Destination */
			case ARGDEST:
				ra_outargs[ra_outcnt++] = OUTDEST;
				ra_outargs[ra_outcnt++] = optarg;
				break;

			/*----Use mail for messages */
			case ARGMAIL:
				ra_outargs[ra_outcnt++] = OUTMAIL;
				ra_outargs[ra_outcnt++] = OUTNOTF;
				break;

			/*----Copies */
			case ARGCOPS:
				ra_outargs[ra_outcnt++] = OUTCOPS;
				ra_outargs[ra_outcnt++] = optarg;
				break;

			/*----Backend options */
			case ARGBKND:
				ra_outargs[ra_outcnt++] = OUTBKND;
				ra_outargs[ra_outcnt++] = optarg;
				break;

			/*----Silent mode */
			case ARGSLNT:
				ra_jobid = FALSE;
				break;
	
			/*----Title */
			case ARGTITL:
				ra_outargs[ra_outcnt++] = OUTTITL;
				ra_outargs[ra_outcnt++] = optarg;
				break;

			/*----Write messages to console */
			case ARGWRIT:
				ra_outargs[ra_outcnt++] = OUTNOTF;
				break;

			/*----Other spurious input */
			default:
			 	usage();	
		}	/* case */

	} /* while */

	if (ra_jobid)
		ra_outargs[ra_outcnt++] = OUTJBID;

	if (ra_incnt < ra_argc)
		ra_outargs[ra_outcnt++] = "--";

	/*----Handle file names */
	for(ra_incnt = optind;
	    ra_incnt < ra_argc;
	    ra_incnt++)
	 	ra_outargs[ra_outcnt++] = ra_argv[ra_incnt];


	/*----Set end of argument list */
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
