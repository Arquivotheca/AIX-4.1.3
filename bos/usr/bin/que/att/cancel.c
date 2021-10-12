static char sccsid[] = "@(#)87	1.18  src/bos/usr/bin/que/att/cancel.c, cmdque, bos41B, 9504A 12/19/94 15:12:13";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: cancel
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
#include <sys/stat.h>
#include "common.h"
#include "frontend.h"


#include "cancel_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_CANCEL,num,str)

/*----Misc */
#define QMSGUSE1 "{JobNumber ... | PrinterName}"
#define QMSGUSE2 "Cancels print jobs specified with the lp command."

/*----Output arguments */
#define OUTCANJ "-x"
#define OUTKALL "-X"
#define OUTPRNT "-P"

char	*progname = "cancel";

extern boolean palladium_inst;

/*====MAIN PROGRAM ENTRY POINT */
main(argc,argv)
int	argc;
char 	**argv;
{
        struct stat statb;

	/* NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_CANCEL, NL_CAT_LOCALE);

        /* check if the Palladium product is installed */
        if (!stat(PDENQ_PATH, &statb))
                palladium_inst = TRUE;

	/*----This is not qdaemon */
	qdaemon = FALSE;

	/*----Get the arguments to LPQ, xlate for ENQ */
	read_args(argc,argv,outargs);
}


/*====READ ALL ARGUMENTS TO LPR AND XLATE TO NEW ARG LIST FOR ENQ:QSTATUS */
read_args(ra_argc,ra_argv,ra_outargs)
int		ra_argc;
char 		**ra_argv;
char		*ra_outargs[];
{
	int		ra_incnt;

	/*----Set the first argument to ENQ program name */
	ra_outargs[0] = ENQPATH;

	/*----Handle case of no arguments */
	if(ra_argc == 1)
		usage();

	/*----Execute ENQ for each parameter */
	for(ra_incnt = 1;
	    (ra_argv[ra_incnt] != NULL) && (ra_incnt < ra_argc);
	    ra_incnt++)
	{
		if(is_purenum(ra_argv[ra_incnt]))
		{
			/*----Generate the job number argument */
			ra_outargs[1] = OUTCANJ;
			ra_outargs[2] = ra_argv[ra_incnt];
			if (ra_incnt + 1 < ra_argc && ! is_purenum(ra_argv[ra_incnt + 1])) 
			{
				ra_outargs[3] = OUTPRNT;
				ra_outargs[4] = ra_argv[ra_incnt + 1];
				ra_outargs[5] = NULL;
				ra_incnt++;
			}
			else
				ra_outargs[3] = NULL;
		}
		else
		{
                        /*----Generate the printer argument */
			if (ra_incnt + 1 < ra_argc && is_purenum(ra_argv[ra_incnt + 1])) 
			{
				ra_outargs[1] = OUTPRNT;
				ra_outargs[2] = ra_argv[ra_incnt++];
				ra_outargs[3] = OUTCANJ;
				ra_outargs[4] = ra_argv[ra_incnt];
                        	ra_outargs[5] = NULL;
			}
			else
			{
                        	ra_outargs[1] = OUTKALL;
				ra_outargs[2] = OUTPRNT;
                        	ra_outargs[3] = ra_argv[ra_incnt];
                        	ra_outargs[4] = NULL;
			}
		}

#ifdef DEBUG
		{
		int i;
		printf ("%s: calling ", ra_argv[0]);
		for (i=0; ra_outargs[i]; i++)
			printf ("%s ", ra_outargs[i]);
		printf ("\n");
		}
#endif
		/*----Execute ENQ with xlated arguments */
		exe_enq(ra_outargs);
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
