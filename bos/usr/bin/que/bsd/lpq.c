static char sccsid[] = "@(#)53	1.16  src/bos/usr/bin/que/bsd/lpq.c, cmdque, bos41B, 9504A 12/19/94 15:11:37";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: lpq
 *
 * ORIGINS: 26, 27 
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
#include <time.h>
#include <sys/stat.h>
#include "common.h"
#include "frontend.h"


#include "lpq_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_LPQ,num,str)

/*----Misc. */
#define QMSGUSE1 "[+Number] [-l] [-PPrinter] [JobNumber ...] [UserName ...]"
#define QMSGUSE2 "Examines the spool queue."
#define DFLTTIM		"5"		/* Default delay time for status display updates */

/*----Input arguments */
#define ARGVERB 'l'
#define ARGPRNT 'P'
#define ARGLOOP '+'
#define ARGDASH	'-'

/*----Output arguments */
#define OUTVERB "-L"
#define OUTPRNT "-qP"
#define OUTLOOP "-w"
#define OUTUSER "-u"
#define OUTJOBN "-#"
#define OUTDFLT "-q"

char	*progname = "lpq";

extern boolean palladium_inst;

/*====MAIN PROGRAM ENTRY POINT */
main(argc,argv)
int	argc;
char 	**argv;
{
        struct stat statb;

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_LPQ, NL_CAT_LOCALE);

        /* check if the Palladium product is installed */
        if (!stat(PDENQ_PATH, &statb))
                palladium_inst = TRUE;

	/*----This is not qdaemon */
	qdaemon = FALSE;

	/*----Get the arguments to LPQ, xlate for ENQ */
	read_args(argc,argv,outargs);
#ifdef DEBUG
	{
	int i;
	printf ("calling exe_enq with ");
	for (i = 0; outargs[i]; i++)
		printf("%s ", outargs[i]);
	printf("\n");
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
	boolean		ra_err;
	char		*ra_thisarg;
	int		ra_outcnt;
	int		ra_incnt;

	/*----Retrieve all dashed -args from commmand line */
	ra_outcnt = 0;

	/*----Set the first argument to ENQ program name */
	ra_outargs[ra_outcnt++] = ENQPATH;

	/*----All requests will need -q */
	ra_outargs[ra_outcnt++] = OUTDFLT;

	/*----Look at each parameter */
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

			/*----Look at '-Pxxxxx' or '-P xxxxx' */
			case ARGPRNT:
				ra_outargs[ra_outcnt++] = OUTPRNT;
				if(ra_thisarg[2] == '\0')
				{
					if(ra_argv[ra_incnt + 1] != NULL)
						ra_outargs[ra_outcnt++] = ra_argv[++ra_incnt];
					else
						ra_err = TRUE;
				}	
				else
					ra_outargs[ra_outcnt++] = &ra_thisarg[2];
				break;

			/*----Look at '-l' */
			case ARGVERB:
				ra_outargs[ra_outcnt++] = OUTVERB;
				break;

			/*----Anything else is bad news */
			default:
				ra_err = TRUE;
				break;
			}
			break;

		/*----Look at "+[n]".  Force default if no "[n]" */
		case ARGLOOP:
			ra_outargs[ra_outcnt++] = OUTLOOP;
			if(!isdigit((int)ra_thisarg[1]))
				ra_outargs[ra_outcnt++] = DFLTTIM;
			else
				ra_outargs[ra_outcnt++] = &ra_thisarg[1];
			break;

		/*----Look at job numbers and user names */
		default:
			if(is_purenum(ra_argv[ra_incnt]))
				ra_outargs[ra_outcnt++] = OUTJOBN;
			else
				ra_outargs[ra_outcnt++] = OUTUSER;
			ra_outargs[ra_outcnt++] = ra_argv[ra_incnt];
			break;
		}	/* case */
		
		/*----Handle errors */
		if(ra_err == TRUE)
			usage();
	}	/* for */

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
		(char *)0
	      );
}
