static char sccsid[] = "@(#)35	1.27.1.12  src/bos/usr/bin/que/params.c, cmdque, bos41J, 9508A 2/16/95 17:16:16";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 9, 27
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
#include <sys/param.h>
#include <errno.h>
#include <sys/types.h>
#include <IN/backend.h>
#include <IN/standard.h>
#include "common.h"
#include "enq.h"
#include <ctype.h>

#include "enq_msg.h"
#define MSGSTR(num,str)	catgets(catd,MS_ENQ,num,str)
nl_catd	catd;

extern boolean palladium_inst;
extern boolean palladium_que = FALSE;

char *scopy();
char *copyfile();


/* plug in queue and dev requested by user 
   return whether we plugged.
   In the remote case, we can't know whether a device
   name given is valid.  We must assume that it is, and pass it across the
   network. 
*/
argqd(arg,qe,movflag)
char *arg;			/* arg from user */
struct qe *qe;
boolean movflag;
{
	char qname[QNAME +1];	/* decoded from arg */
	char dname[DNAME +1];	/* decoded from arg */
	register struct q *q;
	extern struct q *qlist;
	register struct d *d;
	register char *from, *to;

#ifdef	DEBUG
	if( getenv("ARGQD") )
		sysraw("argqd(%s,%x,%x)\n", arg?arg:"NULL", 
			qe->qe_queue , qe->qe_dev );
#endif

	/* copy the queue name */
	for (from = arg, to = qname; *from && *from != ':';)
	{
		*to++ = *from++;
		if (to - qname > QNAME)
			return(FALSE);	/* too long for argname */
	}
	*to++ = '\0';
	/* and the device name, if any */
	if (*from++ == ':')
	{
		if (!*from )
			return(FALSE);	/* nothing after colon */
		strcpy(dname,from);
	}
	else
		strcpy(dname,ANYDEV);

	/* now check to be sure this queue:dev combo is legal */
	for (q = qlist; q; q = q->q_next)
	{
		if (strncmp(q->q_name,qname,QNAME) == 0)/* find the queue */
		{
			if( strcmp(dname,ANYDEV) == 0 )
			{
				if (movflag) 
				{
					qe->qe_movqueue = q->q_name;
					qe->qe_movdev = ANYDEV;
				} else {
					qe->qe_queue  = q;
					qe->qe_dev = NULL;
				}
				goto end_argqd;
			}
			/* find the device */
			for (d = q->q_devlist; d; d = d->d_next)
			{
				if(strcmp(d->d_name,dname)==0)  /* if equal*/
				{
					if (movflag) 
					{
						qe->qe_movqueue = q->q_name;
						qe->qe_movdev = d->d_name;
					} else {
					 	qe->qe_queue = q;
						qe->qe_dev = d;
					}
					goto end_argqd;
				}
			}
			/*
			* If the queue is remote, then the q:d he
			* specified might be valid.  Can't tell.
			*/
			if( remote(q->q_hostname) )
			{
				if (strcmp(dname,ANYDEV))
				syswarn(MSGSTR(MSGRSPC,"Cannot specify device with remote queue."));
				qe->qe_queue  = q;
				qe->qe_dev = NULL;
				goto end_argqd;	/* MAYBE */
			}
			else
				return(FALSE);	/* nope */
		}
	}
        /*----Trap BSD default queue "lp" */
        if (!strcmp(qname,BSD_DEFAULT_QUEUE))
        {
                strcpy(dname,ANYDEV);
                return(TRUE);
        }
        else
                return(FALSE);

end_argqd:
	return(TRUE);
}

	
opterror(flg)
char * flg;
{
	syserr((int)EXITFATAL,MSGSTR(MSGNPRM,
		"%s flag ignored; you must be an administrator for that operation."),
		flg);
}

	

/*
	Rdopts reads the command line into the params struct.
	There is no semantic ambiguity in the command line so getopts
	works ok.
	Filenames must come after all options have been specified.
	Anything not preceded by a '-' or an option which requires an
	argument, is a filename.
*/

extern uid_t invokers_uid;
extern int lpdID;

rdopts(argc,argv,p,option_index,qe,chgrequest)
int	argc;
char 	**argv;
struct params *p;	/* the main idea of this routine*/
struct qe *qe;	
int *option_index;	/* how far did we read thru the argv*/
boolean *chgrequest;
{
	char * dq;
	extern struct q *qlist;
	struct q *qarg = NULL;	/* queue requested by user */
	struct d *darg = NULL;	/* device requested by user */
	int c;
	extern char * optarg;
	extern int optind;
	int numcops=0;		/* number of copies requested*/
	char *noFileArg;	/*
				 * Set to a string when an option is found
				 * that does not allow file arguments.
				 */

	palcnt=0;

	palarg[palcnt++] = PDENQ_PATH;	/* set the pdenq path */
	palarg[palcnt++] = PAL_PRINT;		/* set to print request if it is a cancel
													request it will be change later */
	
	noFileArg = NULL;
	while ((c = getopt(argc, argv,
		    "AHGLcdehijnpqrsXDKUCYN:M:m:R:T:t:a:B:o:#:P:Q:u:w:x:Z:")
	       ) !=EOF)
	{
		switch (c)
		{
		case 'A': 	/* All status */
		/*Since we just exec qstatus for -w, -u, -s, -A, -L and -q, with the
		   same args, we can quit now and go do that. 
		*/
			p->p_Aflg++;
			noFileArg = "-A";
			return(OK); 	
		case 'w': 	/* Wait and loop */
		/*Since we just exec qstatus for -w, -u, -s, -A, -L and -q, with the
		   same args, we can quit now and go do that. 
		*/
			p->p_wflg++;
			noFileArg = "-w";
			return(OK); 	
		case 'C': 	/* always use mail */
			p->p_Cflg++;
			palarg[palcnt++] = scopy("-c\0");
			break;
		case 'c': 	/* Copy a file */
			p->p_cflg++;
			break;
		case 'd':	/* force a digest */
			p->p_dflg++;
			return(OK);
			break;
                case 'e':       /* exclude status of another spooler's queues */
                        p->p_eflg++;
                        noFileArg = "-e";
                        return(OK);
                case 's':       /* show the status w/o any files */
                        p->p_sflg++;
                        noFileArg = "-s";
                        return(OK);
		case 'q': 	/* get q status */
		/* Since we just exec qstatus for -w, -u, -e, -s, -A, -L and 
		   -q, with the same args, we can quit now and go do that. 
		*/
			p->p_qflg++;
			noFileArg = "-q";
			return(OK); 	
		case 'n': 	/* Notify */
			p->p_nflg++;
			palarg[palcnt++] = scopy("n\0");
			break;
		case 'D': 	/* Bring down dev after this job */
			p->p_Dflg++;
			if (!checkPriv(TRUE))
				opterror("-D");
			noFileArg = "-D";
			break;
		case 'G': 	/* Die Gracefully */
			p->p_Gflg++;
			if (!checkPriv(TRUE))
				opterror("-G");
			return(OK);
			break;
		case 'X': 	/* cancel all my jobs.  empty queue if root. */
			p->p_Xflg++;
			*chgrequest = TRUE;
			noFileArg = "-X";
			break;
		case 'r': 	/* Remove file after print */
			p->p_rflg++;
			qe->qe_rm=TRUE;
			palarg[palcnt++] = scopy("-r\0");
			break;
		case 'K': 	/* Kill device now and kill this job */
			if (!checkPriv(TRUE))
				opterror("-K");
			noFileArg = "-K";
			p->p_Kflg++;
			break;
		case 'L': 	/* Long status */
		/*Since we just exec qstatus for -w, -u, -s, -A, -L and -q, with the
		   same args, we can quit now and go do that. 
		*/ 
			p->p_Lflg++;
			noFileArg = "-L";
			return(OK); 	
		case 'U': 	/* Bring up device now */
			p->p_Uflg++;
			if (!checkPriv(TRUE))
				opterror("-U");
			noFileArg = "-U";
			break;
		case 'N': 	/* Number of copies */
			p->p_Nflg++;
			p->p_Nrem=scopy(optarg);
			numcops = atoi(p->p_Nrem);
			if (numcops < 1)
				syserr((int)EXITBAD,MSGSTR(MSGCOPY,"Invalid number of copies: %d."),numcops);
			palarg[palcnt++] = sconcat("-N",optarg,0);
			break;
		case 'M': 	/* Message in file */
			p->p_Mflg++;
			p->p_Mrem=scopy(optarg);
			opmsg(ARGM1, p->p_Mrem);
			palarg[palcnt++] = sconcat("-M",optarg,0);
			break;
		case 'm': 	/* Message on command line */
			p->p_mflg++;
			p->p_mrem=scopy(optarg);
			opmsg(ARGM0, p->p_mrem);
			palarg[palcnt++] = sconcat("-m",optarg,0);
			break;
		case 'a':	/* alter priority */
			p->p_aflg++;
			p->p_arem=scopy(optarg);
			*chgrequest = TRUE;
			noFileArg = "-a";
			goto chkprio;
		case 'R': 	/* Sets priority to num=optarg */
			p->p_Rflg++;
			p->p_Rrem=scopy(optarg);
chkprio:
			{   	int prio;
				prio = atoi(optarg);
				errno = 0 ;
				if (prio < 0 ||
				    prio > P_SLIMIT ||
				    (prio > P_ULIMIT && !checkPriv(TRUE)))
					syserr((int)EXITFATAL,MSGSTR(MSGBARG,
						"Priority out of range: %d (%d for system users)."),
						P_ULIMIT,P_SLIMIT);
				qe->qe_priority = prio;
			}
			palarg[palcnt++] = sconcat("-R",optarg,0);
			break;
		case 'T': 	/* Title on command line */
			p->p_Tflg++;
			p->p_Trem=scopy(optarg);
			qe->qe_reqname=scopy(optarg);
			palarg[palcnt++] = sconcat("-T",optarg,0);
			break;
		case 't': 	/* Send to name */
			p->p_tflg++;
			p->p_trem=qe->qe_to=scopy(optarg);
			palarg[palcnt++] = sconcat("-t",optarg,0);
			break;
		case 'B': 	/* burst pages */
			p->p_Bflg++;
			p->p_Brem=scopy(optarg);
			switch(p->p_Brem[0])
			{   case 'n':
				qe->qe_head = NEVER;
				break;
			    case 'a':
				qe->qe_head = ALWAYS;
				break;
			    case 'g':
				qe->qe_head = GROUP;
				break;
			    default:
				syserr((int)EXITFATAL,MSGSTR(MSGILLB,"Illegal header burst page option."));
			}
			switch(p->p_Brem[1])
			{   case 'n':
				qe->qe_trail = NEVER;
				break;
			    case 'a':
				qe->qe_trail = ALWAYS;
				break;
			    case 'g':
				qe->qe_trail = GROUP;
				break;
			    default:
				syserr((int)EXITFATAL,MSGSTR(MSGILLB,"Illegal trailer burst page option."));
			}
			palarg[palcnt++] = sconcat("-B",optarg,0);
			break;
		case 'x': 	/* Cancel */
			p->p_xflg++;
			p->p_xrem=scopy(optarg);
			if ((-1)==(qe->qe_jobnum=checkjobnum(optarg)))
				syswarn(MSGSTR(MSGBADJ,"Bad job number: %s."),optarg);
			*chgrequest = TRUE;
			noFileArg = "-x";
			palarg[palcnt++] = sconcat("-x",optarg,0);
			break;
		case 'o': 	/* optional backend arguments */
			p->p_oflg++;
/*			p->p_orem=scopy(optarg);*/
			addflag(optarg);
			palarg[palcnt++] = sconcat("-o",optarg,0);
			break;
		case '#':	/* job number for enq -q or enq -a */ 
			/* Specifying -# more than once only makes
			   sense for enq -q.  In that case we exec
			   qstatus with the same args, so we don't
			   need to remember all of the queues. 
			   This code uses the last -# specified. 
			*/ 
			p->p_numflg++;
			p->p_numrem=scopy(optarg);
			qe->qe_jobnum=checkjobnum(optarg);
			break;
		case 'u':	/* username for enq -q */ 
			/* Specifying -u only makes
			   sense for enq -q.  In that case we exec
			   qstatus with the same args, so we don't
			   need to remember all of the queues. 
			   This code saves the last -u specified. 
			*/ 
			p->p_uflg++;
			p->p_urem=scopy(optarg);
			palarg[palcnt++] = sconcat("-u",optarg,0);
			break;
		case 'P':	/* queue name */
			/* Specifying -P more than once only makes
			   sense for enq -q.  In that case we exec
			   qstatus with the same args, so we don't
			   need to remember all of the queues. 
			   This code uses the last -P specified. 
			*/
			if (!argqd(optarg,qe,FALSE))
			{
				if (!palladium_inst)
					syserr((int)EXITFATAL,MSGSTR(MSGBADQ,"Bad queue or device name: %s."),optarg);
				else
				{
					palladium_que = TRUE;
					palarg[palcnt++] = sconcat("-P",optarg,0);
				}
			} else {
				p->p_Pflg++;
				p->p_Prem=scopy(optarg);
			}
			break;
		case 'Y':	/* quit processing, (used only by lpd)*/
			if (palladium_que && palladium_inst)
			{
				p->p_Yflg++;
				palarg[palcnt++] = scopy("-Y\0");
				return(OK);
			}
			else
				qexit(EXITOK);
		case 'Z': 	/* Send to name */
			p->p_Zflg++;
			if ( (invokers_uid == lpdID) || (checkPriv(TRUE)) )
			{
			     if ( strlen (optarg) > S_FROM-1)
				syswarn(MSGSTR(MSGZLIMIT,"-Z %s: Value cannot exceed 63 characters."), optarg);
			     else 
				  {
						p->p_Zrem=qe->qe_logname=scopy(optarg);
						palarg[palcnt++] = sconcat("-Z",optarg,0);
					}
			}
			else
			     syswarn(MSGSTR(MSGNOPR,"Not priviledged to use -Z %s"),optarg);
			break;
		case 'j':	/* return job number */
			p->p_jflg++;
			break;
		case 'H':	/* submit job with in HELD state */
			p->p_Hflg++;
			break;
		case 'h': 	/* change to HELD state */
			p->p_hflg++;
			*chgrequest = TRUE;
			noFileArg = "-h";
			break;
		case 'p': 	/* change to release state */
			p->p_pflg++;
			*chgrequest = TRUE;
			noFileArg = "-p";
			break;
		case 'Q':	/* move a print job */
			p->p_Qflg++;
			if (!argqd(optarg,qe,TRUE)) 
				syserr((int)EXITFATAL,MSGSTR(MSGBADQ,"Bad queue or device name: %s."),optarg);
			else {
				p->p_Qrem = scopy(optarg);
				*chgrequest = TRUE;
				noFileArg = "-Q";
			}
			break;
                case 'i':       /* show local job status */
                        p->p_iflg++;
                        noFileArg = "-s";
                        return(OK);
		case '?':
			usage();
			return(NOTOK);
		}
	}
	*option_index=optind;
	if ( (argc > optind) && (noFileArg != NULL) ) {
		syserr((int)EXITFATAL,MSGSTR(MSGNOFIL,"File arguments are not allowed with the %s option"), noFileArg );
	}

	if(qlist == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGNOQUE,"There are no queues defined in the /etc/qconfig file."));


	if ((p->p_aflg && !p->p_numflg) || (p->p_numflg && !*chgrequest))
	{
		usage();
		return(NOTOK);
	}

	if ((p->p_Qflg || p->p_hflg || p->p_pflg) && !(p->p_Pflg || p->p_numflg || p->p_uflg))
		syserr((int)EXITFATAL,MSGSTR(MSGMISF,"The %s flag requires job number (-#), queue name (-P), or user name (-u) be specified."), noFileArg);

	if (!p->p_Pflg)			/* no -P flag */
	{
		if ((dq = getenv("LPDEST")) || (dq = getenv("PRINTER")))
		{
			if (!argqd(dq,qe,FALSE)) {
				if (!palladium_inst)
					syserr((int)EXITFATAL,MSGSTR(MSGBADP,"Bad PRINTER or LPDEST env. variable %s."),dq);
				else
					palladium_que = TRUE;
			}
			else
				p->p_Prem = scopy(dq);
		}
		else if (palladium_inst && (dq = getenv("PDPRINTER")))
		{
			palladium_que = TRUE;
		}
		else /* priority change and cancel is by job number not queue */
		{
			if (*chgrequest)
				return (OK);
			qe->qe_queue=qlist;
			p->p_Prem = scopy(qe->qe_queue->q_name);
		}
	}
	
	return(OK);
}


/*==== Print usage message and don't exit */
usage()
{
	sysuse( FALSE,
		MSGSTR(MSGUSE1,"[-cdehijnpqrsACDGHKLUX] [-#<JobNum>] [-B<nn,na,ng,an,aa,ag,gn,ga,gg>]"),
		MSGSTR(MSGUSE2,"[-M<MessageFile>] [-N<NumCopies>] [-P<Printer>] [-Q<Queue>] [-R<Priority>]"),
		MSGSTR(MSGUSE3,"[-T<Title>] [-Z<Name>] [-a<Priority>] [-m<Message>] [-o<Option>]"),
		MSGSTR(MSGUSE4,"[-t<DestName>] [-u<Name>] [-w<Seconds>] [-x<JobNum>] [-Y]"),
		(char *)0
	      );
	return(0);
}
