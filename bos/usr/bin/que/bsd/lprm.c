static char sccsid[] = "@(#)55	1.22  src/bos/usr/bin/que/bsd/lprm.c, cmdque, bos41B, 9504A 12/19/94 15:11:35";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: lprm 
 *
 * ORIGINS: 26, 27
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


#include <stdio.h>
#include <pwd.h>
#include <ctype.h>
#include <locale.h>
#include <IN/standard.h>
#include <sys/access.h>
#include <sys/stat.h>
#include "common.h"
#include "frontend.h"


#include "lprm_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_LPRM,num,str)

/*----Misc */
#define QMSGUSE1 "[-PPrinter] [-] [JobNumber] [UserName ...]"
#define QMSGUSE2 "Removes jobs from the line printer spooling queue."

/*----Input arguments */
#define ARGPRNT 'P'
#define ARGDASH	'-'
#define ARGZERO '\0'

/*----Output arguments */
#define OUTPRNT "-P"
#define OUTJOBN "-x"
#define OUTRALL "-X"
#define OUTUSER "-u"
#define QADM "/usr/bin/qadm"

char	*progname = "lprm";

extern boolean palladium_inst;

/*====MAIN PROGRAM ENTRY POINT */
main(argc,argv)
int	argc;
char 	**argv;
{
	char **ppTmp;
        struct stat statb;

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_LPRM, NL_CAT_LOCALE);

        /* check if the Palladium product is installed */
        if (!stat(PDENQ_PATH, &statb))
                palladium_inst = TRUE;

	/*----This is not qdaemon */
	qdaemon = FALSE;

	read_args(argc,argv,outargs); 
}

/*====READ ALL ARGUMENTS TO LPR AND XLATE TO NEW ARG LIST FOR ENQ:QSTATUS */
read_args(ra_argc,ra_argv,ra_outargs)
int		ra_argc;
char 		**ra_argv;
char		*ra_outargs[];
{
	char		*ra_thisarg;
	int		ra_outcnt;
	int		ra_incnt;
	int		ra_true_args;
	char		*ra_newstr;
	boolean		ra_err;
	boolean		ra_xflg;
	boolean		ra_Xflg;
	int		ra;
	struct passwd 	*pw;
	int		savcnt;
	int		execed;

	/*----Set the first argument to ENQ program name */
	ra_outcnt = 0;
	execed = 0;
	ra_outargs[ra_outcnt++] = ENQPATH;

	/*----Look at each parameter */
	ra_err = FALSE;
	ra_xflg = FALSE;
	ra_Xflg = FALSE;
	ra_true_args = 0;
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
						usage();
				}
				else
					ra_outargs[ra_outcnt++] = &ra_thisarg[2];
				break;

			/*----Look at '-' */
			case ARGZERO:
				/*
				 * Run enq -X -u username
				 * unless system administrator; then run
				 * enq -X
				 */
				pw = getpwuid( getuid() );

				ra_outargs[ra_outcnt++] = OUTRALL;
				ra_Xflg = TRUE;

        			if(accessx(QADM,X_ACC,ACC_INVOKER)) {
					ra_outargs[ra_outcnt++]  = OUTUSER;
					ra_outargs[ra_outcnt]  = Qalloc( strlen( pw->pw_name ) +1 );
					strcpy(ra_outargs[ra_outcnt++], pw->pw_name);
				}
				ra_true_args++;
				break;

			/*----Anything else is bad news */
			default:
				usage();
				break;
			}
			break;

		/*----Look at job numbers and user names */
		default:
			savcnt = ra_outcnt;
			if(is_purenum(ra_argv[ra_incnt]))
			{
				ra_xflg = TRUE;
				ra_outargs[ra_outcnt++] = OUTJOBN;
				ra_outargs[ra_outcnt++] = ra_argv[ra_incnt];
			}
			else {
				ra_outargs[ra_outcnt++] = OUTRALL;
				ra_outargs[ra_outcnt++] = OUTUSER;
				ra_outargs[ra_outcnt++] = ra_argv[ra_incnt];
			}
			ra_outargs[ra_outcnt] = NULL;
			exe_enq(ra_outargs);
			execed++;
			ra_outcnt = savcnt;
			break;
		}

	}	/* for */

	if(!(execed)){
		/*---Finish off the list */
		ra_outargs[ra_outcnt] = NULL;

		/*----Handle case of no arguments */
		if(ra_argc == 1 || ra_true_args == 0) {
			usage();
		}

		/*----Add -X flag if necessary */
		if (FALSE == ra_xflg && FALSE == ra_Xflg)
		{
			for(ra = ra_outcnt; ra > 0; ra--)
				ra_outargs[ra + 1] = ra_outargs[ra];
			ra_outargs[1] = OUTRALL;
		}

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
