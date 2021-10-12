static char sccsid[] = "@(#)89	1.15  src/bos/usr/bin/que/att/enable.c, cmdque, bos411, 9428A410j 12/16/93 15:40:08";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: enable
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


#include "enable_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_ENABLE,num,str)

/*----Misc. */
#define QMSGUSE1 "PrinterName ..."
#define QMSGUSE2 "Enables or activates printers."

/*----Output arguments */
#define OUTENAB "-U"
#define PARG "-P"

char	*progname = "enable";


/*====MAIN PROGRAM ENTRY POINT */
main(argc,argv)
int	argc;
char 	**argv;
{
	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_ENABLE, NL_CAT_LOCALE);

	/*----This is not qdaemon */
	qdaemon = FALSE;

	/*----Get the arguments, xlate for ENQ */
	read_args(argc,argv,outargs);

}


/*====READ ALL ARGUMENTS TO LPR AND XLATE TO NEW ARG LIST FOR ENQ:QSTATUS */
read_args(ra_argc,ra_argv,ra_outargs)
int		ra_argc;
char 		**ra_argv;
char		*ra_outargs[];
{
	int		ra_outcnt;
	int		ra_incnt;

	/*----Set the first argument to ENQ program name */
	ra_outcnt = 0;
	ra_outargs[ra_outcnt++] = ENQPATH;
	
	/*----Handle case of no command line args */
	if(ra_argc == 1)
		usage();

	/*----Look at each parameter */
	for(ra_incnt = 1;
	    (ra_argv[ra_incnt] != NULL) && (ra_incnt < ra_argc);
	    ra_incnt++)
	{
		/*----Execute enq once for each printer */
		ra_outargs[ra_outcnt] = OUTENAB;   /* "-U" */
		ra_outargs[ra_outcnt + 1] = PARG;   /* "-P" */
		ra_outargs[ra_outcnt + 2] = ra_argv[ra_incnt];
		ra_outargs[ra_outcnt + 3] = NULL;


		/*----Execute ENQ with xlated arguments */
			exe_enq(outargs);
	}
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
