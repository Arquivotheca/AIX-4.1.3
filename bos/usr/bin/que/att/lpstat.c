static char sccsid[] = "@(#)92	1.23  src/bos/usr/bin/que/att/lpstat.c, cmdque, bos41B, 9504A 12/19/94 15:11:32";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: lpstat
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
#include <time.h>
#include <sys/stat.h>
#include "common.h"
#include "frontend.h"


#include "lpstat_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_LPSTAT,num,str)

/*----Misc. */
#define QMSGUSE1"[-drst] [-a<dest>,...] [-c<classname>,...]"
#define QMSGUSE2"[-o<outreq>,...] [-p<printer>,...] [-u<user>,...]"
#define QMSGUSE3"[-v<printer>,...] [<jobid>,...]"
#define QMSGUSE4 "Prints LP status information."

/*----Input arguments */
#define ARGACPT 'a'
#define ARGCLSS 'c'
#define ARGDFLT 'd'
#define ARGOTPT 'o'
#define ARGPRNT 'p'
#define ARGRQST 'r'
#define ARGSUMM 's'
#define ARGTOTL 't'
#define ARGUSER 'u'
#define ARGPRNV 'v'
#define ARGDASH '-'

/*----Output arguments */
#define OUTALLQ "-A"
#define OUTDFLT "-q"
#define OUTPRNT "-P"
#define OUTALLV "-A"
#define OUTLONG "-L"
#define OUTUSER "-u"
#define OUTJOBI "-#"
#define OUTQSTA "-s"

char	*progname = "lpstat";

extern boolean palladium_inst;

/*====MAIN PROGRAM MODULE */
main(argc,argv)
int	argc;
char 	**argv;
{
        struct stat statb;

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_LPSTAT, NL_CAT_LOCALE);

        /* check if the Palladium product is installed */
        if (!stat(PDENQ_PATH, &statb))
                palladium_inst = TRUE;

	/*----This is not qdaemon */
	qdaemon = FALSE;

	/*----Get the arguments to LPR, xlate for ENQ */
	read_args(argc,argv,outargs);

	/*----Execute ENQ with xlated arguments */
	exe_enq(outargs);
}

/*====READ ALL ARGUMENTS AND XLATE TO NEW ARG LIST FOR ENQ:QSTATUS */
read_args(ra_argc,ra_argv,ra_outargs)
int		ra_argc;
char 		**ra_argv;
char		*ra_outargs[];
{
	char		*ra_thisarg;
	boolean		ra_err;
	int		ra_outcnt;
	int		ra_incnt;

	/*----Set the first argument to ENQ program name */
	ra_outcnt = 0;
	ra_outargs[ra_outcnt++] = ENQPATH;

	/*----Handle case of no args */
	if(ra_argc == 1)
		ra_outargs[ra_outcnt++] = OUTALLQ;

	/*----Look for all arguments */
	ra_err = FALSE;
	do {
		if (optind >= ra_argc) break;
		ra_thisarg = ra_argv[optind];
		ra_incnt = ra_thisarg[0];
		if (ra_incnt == '-') {
			if ((ra_thisarg[1] == ARGACPT) || (ra_thisarg[1] == ARGCLSS) || (ra_thisarg[1] == ARGPRNT) || (ra_thisarg[1] == ARGPRNV))	
			{
				ra_outargs[ra_outcnt++] = OUTDFLT;
				if(ra_thisarg[2] == NULL)
					ra_outargs[ra_outcnt++] = OUTALLQ;
				else {
					xpndlist(&ra_thisarg[2],
						 &ra_outcnt,
						 ra_outargs,
						 OUTPRNT);
				}
				optind++;
			}
			else if ( ra_thisarg[1] == ARGOTPT)
			{
			/*----Status of a queue and/or jobID */
				ra_outargs[ra_outcnt++] = OUTDFLT;
				if(ra_thisarg[2] == NULL)
					ra_outargs[ra_outcnt++] = OUTALLQ;
				else {
					xpndoflag(&ra_thisarg[2],
						 &ra_outcnt,
						 ra_outargs);
				}
				optind++;
			}
			else if ( ra_thisarg[1] == ARGUSER)
			{
                                if(ra_thisarg[2] == NULL)
                                        ra_outargs[ra_outcnt++] = OUTALLQ;
                                else {
                                        xpndlist(&ra_thisarg[2],
                                                 &ra_outcnt,
                                                 ra_outargs,
                                                 OUTUSER);
				}
				optind++;
			}
			else
			{
				ra_incnt = getopt(ra_argc, ra_argv,"drst");
				switch (ra_incnt) {
					/*----request scheduler, status summary */
					case ARGRQST:
					case ARGSUMM:
						ra_outargs[ra_outcnt++] = OUTALLQ;
						break;

					/*----Default destination */
					case ARGDFLT:
					ra_outargs[ra_outcnt++] = OUTDFLT;
						break;

					/*----All status information */
					case ARGTOTL:
						ra_outargs[ra_outcnt++] = OUTALLV;
						ra_outargs[ra_outcnt++] = OUTLONG;
						break;

					/*----Other spurious input */
					case '?':
						ra_err = TRUE;
						break;
				}	/* inner case */
			}
		}
		else
		{
			if (is_purenum(ra_argv[optind])) {
				ra_outargs[ra_outcnt++] = OUTALLQ;
				ra_outargs[ra_outcnt++] = OUTJOBI;
				ra_outargs[ra_outcnt++] = ra_argv[optind];
				optind++;
			}
			else	/* assume it's a file name */
			 	ra_err = TRUE;
		}

		/*----If bad argument found */
		if (ra_err == TRUE)
			usage();
	} while(ra_incnt != -1);

        /* check for jobid's after -- string */
        for ( ; optind < ra_argc; optind++)
        {
                if (is_purenum(ra_argv[optind])) {
                        ra_outargs[ra_outcnt++] = OUTALLQ;
                        ra_outargs[ra_outcnt++] = OUTJOBI;
                        ra_outargs[ra_outcnt++] = ra_argv[optind];
                }
                else    /* assume it's a file name or flag */
                        usage();
        }

        /* check for "lpstat --" instance */
        if ((ra_argc == 2) && (strcmp(ra_argv[1], "--") == 0))
                ra_outargs[ra_outcnt++] = OUTALLQ;

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
		MSGSTR(MSGUSE4,QMSGUSE4),
		(char *)0
	      );
}



/*====EXPAND A LIST OF ITEMS FOR THE -o FLAG (QUEUES/JOBIDS) */
xpndoflag(xl_arg,xl_index,xl_outargs)
char	*xl_arg;
int	*xl_index;
char	**xl_outargs;
{
	char	*xl_value;
	int	id;
	char	*outp=NULL;

	while(*xl_arg != '\0')
	{
		/*----Skip past all commas, spaces */
		xl_value = xl_arg;
		while(*xl_value == '\"' ||
		      *xl_value == ' '  ||
		      *xl_value == ',')
			xl_value++;
		if(*xl_value == '\0')
			break;

		/*----Set pointer, go to end of string, and set a '\0' */
		xl_arg = xl_value;
		while(*xl_arg != '\0' &&
		      *xl_arg != '\"' &&
		      *xl_arg != ' '  &&
		      *xl_arg != ',')
			xl_arg++;
		if(*xl_arg != '\0')
			*(xl_arg++) = '\0';

		/*----Send new argument to output parmeter list */

		if ((id = strtol(xl_value, &outp, 10)) && ( *outp == NULL )
		    && ( id <= MAXJOB )) {
			xl_outargs[(*xl_index)++] = OUTJOBI ;
			xl_outargs[(*xl_index)++] = xl_value;
			}
		else {
			xl_outargs[(*xl_index)++] = OUTPRNT ;
			xl_outargs[(*xl_index)++] = xl_value;
			}
	}
	return(0);

}

