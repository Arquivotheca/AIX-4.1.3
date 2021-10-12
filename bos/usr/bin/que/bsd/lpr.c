static char sccsid[] = "@(#)54	1.21.1.6  src/bos/usr/bin/que/bsd/lpr.c, cmdque, bos411, 9428A410j 2/8/94 16:14:06";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: lpr
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

#include <IN/standard.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <sys/id.h>
#include "../common.h"
#include "../frontend.h"


#include "lpr_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_LPR,num,str)

/*----Misc. */
#define QMSGUSE1 "[-fghjlmnprs] [-PPrinter] [-#NumberCopies] [-CClass]"
#define QMSGUSE2 "[-JJob] [-TTitle] [-iNumberColumns] [-wWidth] [Filename ...]"
#define QMSGUSE3 "Prints the specified files when facilities become available."

/*----Input arguments */
#define SINGLETTS	"pltndgvcfrmhsj"
#define ARGPFMT 'p'
#define ARGPCTL 'l'
#define ARGTOFF 't'
#define ARGDOFF 'n'
#define ARGPTEX 'd'
#define ARGPLOT 'g'
#define ARGRAST 'v'
#define ARGCIFP 'c'
#define ARGFORT 'f'
#define ARGDELF 'r'
#define ARGMAIL 'm'
#define ARGNOBP 'h'
#define ARGSYMB 's'
#define ARGPRNT 'P'
#define ARGCOPS '#'
#define ARGJCLS 'C'
#define ARGJNAM 'J'
#define ARGTITL 'T'
#define ARGINDT 'i'
#define ARGWDTH 'w'
#define ARGDASH '-'
#define ARGJBID 'j'

/*----Output arguments */
#define OUTBKND "-o"
#define OUTFILT "-f"
#define OUTDELF	"-r"
#define OUTMAIL "-C"
#define OUTCOPY "-c"
#define OUTNOTF "-n"
#define OUTNOBP "-Bnn"
#define OUTBANN "-Bgn"
#define OUTPRNT "-P"
#define OUTJNAM "-T"
#define OUTCOPS "-N"
#define OUTJCLS "-H"
#define OUTTITL "-h"
#define OUTONEF "-1"
#define OUTTWOF "-2"
#define OUTTHRF "-3"
#define OUTFORF "-4"
#define OUTINDT "-i"
#define OUTWDTH "-w"
#define OUTJBID "-j"

char	*progname = "lpr";


/*====MAIN PROGRAM MODULE */
main(argc,argv)
int	argc;
char 	**argv;
{
	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_LPR, NL_CAT_LOCALE);

	/*----This is not qdaemon */
	qdaemon = FALSE;

	/*----Get the arguments to LPR, xlate for ENQ */
	read_args(argc,argv,outargs);

#ifdef DEBUG
        {
        int i;
        for (i=0; outargs[i] != 0; i++) fprintf(stderr,"%s ",outargs[i]);
	fprintf(stderr,"\n");
        }
#endif

	/*----Execute ENQ with xlated arguments */
	exe_enq(outargs);
}

/*====READ ALL ARGUMENTS AND XLATE TO NEW ARG LIST FOR ENQ */
read_args(ra_argc,ra_argv,ra_outargs)
int		ra_argc;
char 		**ra_argv;
char		*ra_outargs[];
{
	char		*ra_thisarg;
	boolean		ra_err;
	int		ra_outcnt;
	int		ra_incnt;
	char		*ra_newstr,
			*ra_new2str;
	int		ra_i;
	boolean		ra_copy;
	int		argp = FALSE, argt = FALSE;
	boolean		hdr=TRUE, ra_hdr=TRUE;

	/*----Set the first argument to ENQ program name */
	ra_outcnt = 0;
	ra_outargs[ra_outcnt++] = ENQPATH;

	/*----Set up defaults */
	ra_copy = TRUE;

	/*----Look for all arguments */
	ra_err = FALSE;
	for(ra_incnt = 1;
	    (ra_argv[ra_incnt] != NULL) && (ra_incnt < ra_argc);
	    ra_incnt++)
	{
		ra_thisarg = ra_argv[ra_incnt];
		switch(ra_thisarg[0]) {

		/*----Look at stuff starting with '-' */
		case ARGDASH:
			/*----Handle single letter args that may be clumped together */
			ra_i = 1;
			while(ra_thisarg[ra_i] != '\0')
			{
				if(strchr(SINGLETTS,ra_thisarg[ra_i]) != NULL)
				{
					switch(ra_thisarg[ra_i]) {

					/*----Delete file after printing */
					case ARGDELF:	
						ra_outargs[ra_outcnt++] = OUTDELF;
						break;

					/*----Send Mail upon completion */
					case ARGMAIL:
						ra_outargs[ra_outcnt++] = OUTMAIL;
						ra_outargs[ra_outcnt++] = OUTNOTF;
						break;

					/*----Suppress burst page printing */
					case ARGNOBP:
						hdr = !hdr;
						break;

					/*----Symbolic linking */
					case ARGSYMB:
						ra_copy = FALSE;
						break;

					/*----Single char parms to send through to backend */
					case ARGPFMT:	/* Use pr to format files */
						argp = TRUE;
						/* no break */
					case ARGPCTL:	/* Allow ctrl chars sent to printer */
					case ARGTOFF:	/* Data is from troff */
					case ARGDOFF:	/* Data is from ditroff */
					case ARGPTEX:	/* Data is from tex */
					case ARGPLOT:	/* Data is from plot */
					case ARGRAST:	/* Data contains a raster image */
					case ARGCIFP:	/* Data is from cifplot */
					case ARGFORT:	/* Use fortran filter for 1st char */
						ra_outargs[ra_outcnt++] = OUTBKND;
						ra_newstr = Qalloc(4);
						strcpy(ra_newstr,OUTFILT);
						ra_newstr[2] = ra_thisarg[ra_i];
						ra_newstr[3] = '\0';
						ra_outargs[ra_outcnt++] = ra_newstr;
						break;
					/* return a job number */
					case ARGJBID:
						ra_outargs[ra_outcnt++] = OUTJBID;
						break;
					} 		/* switch */
					++ra_i;
				} 			/* if */
				else
					break; 		/* out of loop */
			}				/* while */

			/*----Handle other args with optargs */
			switch(ra_thisarg[ra_i])
			{
			/*----End of group found */
			case '\0':
				if(ra_i == 1)
					ra_err = TRUE;
				break;
				
			/*----Files to print */
			case ARGPRNT:
				ra_outargs[ra_outcnt++] = OUTPRNT;
				goto Waco;

			/*----Number of copies */
			case ARGCOPS:
				ra_outargs[ra_outcnt++] = OUTCOPS;
				goto Waco;
        
			/*----Job name to print on burst page */
			case ARGJNAM:
				hdr++;
				ra_outargs[ra_outcnt++] = OUTJNAM;

Waco:				/*----Boo, hiss, a goto! */
				if(ra_thisarg[ra_i + 1] == '\0')
				{
					if(ra_argv[ra_incnt + 1] != NULL)
						ra_outargs[ra_outcnt++] = ra_argv[++ra_incnt];
					else
						ra_err = TRUE;
				}
				else
					ra_outargs[ra_outcnt++]  = &ra_thisarg[ra_i + 1];
				break;
			
			/*----Parameters with arguments to send  to backend */
			/*----Title for pr instead of filename */
			case ARGTITL:
				argt = TRUE;
				ra_newstr = OUTTITL;
				goto Odessa;

			/*----Job classification on burst page */
			case ARGJCLS:
				ra_newstr = OUTJCLS;
				hdr++;
                                goto Odessa;

			/*----Indent output */
			case ARGINDT:
				ra_newstr = OUTINDT;
				/* D42054 - the following lines were copied
				** and modified from the Odessa path since
				** the value after the -i is optional.
				*/
				ra_outargs[ra_outcnt++] = OUTBKND;
				if(ra_thisarg[ra_i + 1] == '\0')
				{
					char *tptr;
					if(ra_argv[ra_incnt + 1] != NULL && (strtol(ra_argv[ra_incnt+1],&tptr,10) != 0 ) && *tptr == '\0')
						/* lpr -i 5 file */
						ra_new2str = sconcat(ra_newstr,ra_argv[++ra_incnt],0);
					else
						/* lpr -i file */
						ra_new2str = sconcat(ra_newstr,"8",0);
				} else
						/* lpr -i5 file */
					ra_new2str = sconcat(ra_newstr,&ra_thisarg[ra_i + 1],0); 
				ra_outargs[ra_outcnt++] = ra_new2str;
				break;
				/* D42054 - end of fix */

			/*----Page width */
			case ARGWDTH:
				ra_newstr = OUTWDTH;

Odessa:				/*----Boo, hiss, a goto! */
				ra_outargs[ra_outcnt++] = OUTBKND;
				if(ra_thisarg[ra_i + 1] == '\0')
				{
					if(ra_argv[ra_incnt + 1] != NULL)
						ra_new2str = sconcat(ra_newstr,ra_argv[++ra_incnt],0);
					else
						ra_err = TRUE;
				} else
					ra_new2str = sconcat(ra_newstr,&ra_thisarg[ra_i + 1],0);
				ra_outargs[ra_outcnt++] = ra_new2str;
				break;

			/*----Other spurious input */
			default:
				ra_err = TRUE;
				break;
			}	/* inner case */
			break;

		/*----Handle file names */
		default:
			/*----Set the copy flag before any files */
			if(ra_copy == TRUE)
			{
				ra_outargs[ra_outcnt++] = OUTCOPY;
				ra_copy = FALSE;
		
				/*----10577/42226: Use first filename as default pr header */
				if (argp && !argt)
				{
					ra_outargs[ra_outcnt++] = OUTBKND;	
					ra_outargs[ra_outcnt++] =
						sconcat(OUTTITL, ra_argv[ra_incnt], 0);
					argt = TRUE;
				}
			}

			if (ra_hdr == TRUE)
			{
				if (hdr) {
					ra_outargs[ra_outcnt++] = OUTBANN;
				}
				else {
					ra_outargs[ra_outcnt++] = OUTNOBP;
				}
				ra_hdr = FALSE;
			}
			ra_outargs[ra_outcnt++] = ra_argv[ra_incnt];
			break;
		}	/* outer case */

		/*----If bad argument found */
		if (ra_err == TRUE)
			/* getopt already spit out an error message */
			usage();
	} /* for */

	/*----Possibly force the banner page if stdin was used    */
	if (ra_hdr == TRUE)
	{
		if (hdr) {
			ra_outargs[ra_outcnt++] = OUTBANN;
		}
		else {
			ra_outargs[ra_outcnt++] = OUTNOBP; 
		}
		ra_hdr = FALSE;
	}

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
