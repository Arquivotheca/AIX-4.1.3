static char sccsid[] = "@(#)51	1.17.1.4  src/bos/usr/bin/que/aix/qprt.c, cmdque, bos411, 9428A410j 1/5/94 12:15:40";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: qprt
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <locale.h>
#include <time.h>
#include "common.h"
#include "frontend.h"
#include "qcadm.h"


#include "qprt_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_QPRT,num,str)

/*----Misc. */
#define QMSGUSE1 "[-cCnr] [-Bnn|-Bna|-Bng|-Ban|-Baa|-Bag|-Bgn|-Bga|-Bgg]"
#define QMSGUSE2 "[-aPreviewOpt] [-ALevel] [-bBottomMargin] [-dInputType]"
#define QMSGUSE3 "[-DUser] [-eEmpasizedOpt] [-EDblHigh] [-fFilter]"
#define QMSGUSE4 "[-FFontFile] [-gBegin] [-GCoord] [-hHeader] [-HHostname]"
#define QMSGUSE5 "[-iIndent] [-IFontID] [-jInit] [-JRestore] [-kColor]"
#define QMSGUSE6 "[-K] [-lLength] [-LLineWrap] [-mMessage] [-QValue]"
#define QMSGUSE7 "[-MMessageFile] [-NNumberCopies] [-OPaperHandl] [-PPrinter]"
#define QMSGUSE8 "[-pPitch] [-qQuality] [-r] [-RPriority] [-sNameType]"
#define QMSGUSE9 "[-SSpeed] [-tTopMargin] [-TTitle] [-uPaperSrc]"
#define QMSGUSE10 "[-UDirectional] [-vLinesPerIn] [-VVertical] [-wPageWidth]"
#define QMSGUSE11 "[-WDblWide] [-xLineFeed] [-XCodePage] [-yDblStrike]"
#define QMSGUSE12 "[-YDuplex] [-zRotate] [-ZFormFeed] [-#h] [-#j] [-#v] File ..."
#define QMSGUSE13 "Starts a print job."
#define QMSGFOERR "%1$s: Error in opening the file '%2$s'\n"


/*----Input arguments */
#define OURARGS "cCB:D:m:M:nN:o:P:rR:T:#:"
#define OTHER_ARGS "A:a:b:d:e:E:f:F:g:G:h:H:i:I:j:J:k:K:l:L:n:N:O:p:q:Q:s:S:t:u:U:v:V:w:W:x:X:y:Y:z:Z:0:1:2:3:4:5:6:7:8:9:?"
#define ARGCOPY 'c'
#define ARGMAIL 'C'
#define ARGNOTF 'n'
#define ARGREMV 'r'
#define ARGBRST 'B'
#define ARGDELV 'D'
#define ARGMSGC 'm'
#define ARGMSGF 'M'
#define ARGCOPS 'N'
#define ARGBKND 'o'
#define ARGPRNT 'P'
#define ARGPRTY 'R'
#define ARGTITL 'T'
#define ARGDASH '-'
#define ARGQUES	'?'
#define ARGHASH '#'

/*----Output arguments */
#define OUTDELV 't'
#define OUTBKND "-o"
#define OUTJBID 'j'
#define OUTHOLD 'H' 

char *progname = "qprt";
#ifdef DEBUG
boolean	verbose;
#endif

extern void		read_qconfig(struct quelist **);
extern int		piovalflags(const char*,const char*,uint_t,char **);

static char		*beav[MAXARGS];	/* backend argv */
static uint_t		beac;		/* backend argc */
static uchar_t		validate; 	/* flag to validate command flags */
static char		qnm[QNAME+1];	/* queue specified */

/*====MAIN PROGRAM MODULE */
main(argc,argv)
int	argc;
char 	**argv;
{
	char **ppTmp;

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_QPRT, NL_CAT_LOCALE);

#ifdef DEBUG
	verbose = FALSE;
#endif
	
	/*----This is not qdaemon */
	qdaemon = FALSE;

	/*----Get the arguments to LPR, xlate for ENQ */
	read_args(argc,argv,outargs);

#ifdef DEBUG
        if ( verbose ) {
                for (ppTmp = outargs; *ppTmp != NULL; ppTmp++) {
                        fputs(*ppTmp, stdout);
                        putchar(' ');
                }
                putchar('\n');
        }
#endif 
	
	/*----Validate the flags, if needed */
	if (validate)
	{
	   register char		*cp;
	   char				qdnm[DNAME+1];
	   struct quelist		*qhdp		= NULL;
	   int				qcfgfd;
	   struct flock			lck;
	   register struct quelist	*qp;

	   /* Build a list of queues and their devices by parsing the
	      /etc/qconfig file. */
	   if ((qcfgfd = open(QCONFIG,O_RDONLY)) == -1)
	      syserr((int)EXITFATAL,MSGSTR(MSGFOERR,QMSGFOERR),progname,
		     QCONFIG);
	   lck.l_whence = lck.l_start = lck.l_len = 0;
	   lck.l_type = F_RDLCK;
	   (void) fcntl(qcfgfd,F_SETLKW,&lck);
	   read_qconfig(&qhdp);
	   (void) close(qcfgfd);
	   do
	   {
	      if (!qhdp)
	         break;
	      if (!*qnm)
		 (void)strncpy(qnm,(cp = getenv("LPDEST")) ||
			           (cp = getenv("PRINTER")) ?
				   cp : qhdp->qname,sizeof(qnm)-1);
	      *(qdnm+sizeof(qdnm)-1) = 0;
	      if (cp = strchr(qnm,':'))
		 (void)strncpy(qdnm,cp+1,sizeof(qdnm)-1);
	      else
	      {
		 for (qp = qhdp; qp && strcmp(qp->qname,qnm); qp = qp->next)
		    ;
		 if (!qp || !((qp = qp->next) && !strcmp(qp->qname,qnm)))
		    break;
		 (void)strncpy(qdnm,qp->dname,sizeof(qdnm)-1);
	      }
	      (void)piovalflags(qnm,qdnm,beac,beav);
	   } while (0);
	}

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

	char		allargs[sizeof(OURARGS) + sizeof(OTHER_ARGS)];
	int		ra_thisarg;
	char		*ra_newstr;
	int		ra_incnt;
	int		ra_outcnt;
	register char	*cp;


	/*----Set the first argument to ENQ program name */
	ra_outcnt = 0;
	ra_outargs[ra_outcnt++] = ENQPATH;
	beav[beac++] = progname;

	strcpy(allargs, OURARGS);
	strcat(allargs, OTHER_ARGS);
	/*----Get all the arguments and pass through */
	while((ra_thisarg = getopt(ra_argc,ra_argv, allargs)) != EOF)
		switch(ra_thisarg) {

		case ARGCOPY:
		case ARGMAIL:
		case ARGNOTF:
		case ARGREMV:
			/*----Construct the parameter string */
			ra_newstr = (char *)Qalloc(3);
			ra_newstr[0] = ARGDASH;
			ra_newstr[1] = ra_thisarg;
			ra_newstr[2] = '\0';
			ra_outargs[ra_outcnt++] = ra_newstr;
			break;
			
		case ARGPRNT:
			(void)strncpy(qnm,optarg,sizeof(qnm)-1);
					/* fall through */
		case ARGBRST:
		case ARGDELV:
		case ARGMSGC:
		case ARGMSGF:
		case ARGCOPS:
		case ARGBKND:
		case ARGPRTY:
		case ARGTITL:
			/*----Construct the parameter string */
			ra_newstr = (char *)Qalloc(3);
			ra_newstr[0] = ARGDASH;
			if(ra_thisarg == ARGDELV)
				ra_newstr[1] = OUTDELV;
			else
				ra_newstr[1] = ra_thisarg;
			ra_newstr[2] = '\0';
			ra_outargs[ra_outcnt++] = ra_newstr;

			/*----Construct the argument string */ 
			ra_newstr = (char *)Qalloc(strlen(optarg) + 1);
			strcpy(ra_newstr,optarg);
			ra_outargs[ra_outcnt++] = ra_newstr;
			break;
		
		case ARGHASH:
                        /*----Construct the parameter string */
			if (*optarg == 'v')
				validate = TRUE;
			else
			{
                        	ra_newstr = (char *)Qalloc(3);
                        	ra_newstr[0] = ARGDASH;
				switch(optarg[0])
				{	case 'h':
						ra_newstr[1] = OUTHOLD;
						break;
					case 'j':
                                		ra_newstr[1] = OUTJBID;
						break;
					default:
						usage();
				}
                        	ra_newstr[2] = '\0';
                        	ra_outargs[ra_outcnt++] = ra_newstr;
			}
			break;

		case ARGQUES:
				/*
				 * If the argument is not in getopt's
				 * list, it comes out as a ?
				 */
			usage();
			break;


			/* Fall Through */
		default:
			/*----Send anything else to the backend */
			ra_outargs[ra_outcnt++] = OUTBKND;
			ra_newstr = (char *)Qalloc(4);
			ra_newstr[0] = '-';
			ra_newstr[1] = ra_thisarg;
			ra_newstr[2] = '\0';
			ra_outargs[ra_outcnt++] = ra_newstr;
			cp = (char *)Qalloc(3);
			(void)memcpy(cp,ra_newstr,3);
			beav[beac++] = cp;
			if(optarg != NULL)
			{
				ra_outargs[ra_outcnt++] = OUTBKND;
				ra_newstr = (char *)Qalloc(ARGSIZE + 1);
				strcpy(ra_newstr,optarg);
				ra_outargs[ra_outcnt++] = ra_newstr;
				cp = (char *)Qalloc(strlen(ra_newstr)+1);
				(void)strcpy(cp,ra_newstr);
				beav[beac++] = cp;
			}
			break;
		}	/* case */

	/*----Pass the remaining filename straight thru */
	for(ra_incnt = optind;
	    ra_incnt < ra_argc;
	    ra_incnt++)
		ra_outargs[ra_outcnt++] = ra_argv[ra_incnt];

	/*----Set the end of the argument list */
	beav[beac] = ra_outargs[ra_outcnt] = NULL;
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
		MSGSTR(MSGUSE5,QMSGUSE5),
		MSGSTR(MSGUSE6,QMSGUSE6),
		MSGSTR(MSGUSE7,QMSGUSE7),
		MSGSTR(MSGUSE8,QMSGUSE8),
		MSGSTR(MSGUSE9,QMSGUSE9),
		MSGSTR(MSGUSE10,QMSGUSE10),
		MSGSTR(MSGUSE11,QMSGUSE11),
		MSGSTR(MSGUSE12,QMSGUSE12),
		MSGSTR(MSGUSE13,QMSGUSE13),
		(char *)0
	      );
}
