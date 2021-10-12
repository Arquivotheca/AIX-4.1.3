static char sccsid[] = "@(#)78	1.19.1.6  src/bos/usr/bin/que/qadm/mkque.c, cmdque, bos41B, 9504A 12/19/94 15:15:33";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: mkque
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <IN/standard.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <sys/limits.h>
#include <sys/stat.h>
#include "common.h"
#include "qcadm.h"
#include <ctype.h>

#include "qcadm_msg.h"
nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_QCADM,num,str)

/*----Input arguments */
#define OURARGS "Dq:a:"
#define ARGDFLT	'D'
#define ARGQUE 	'q'
#define ARGARG	'a'

/*----State machine defines */
#define	STLKQUE	1
#define STLKARG 2

/*----Misc. */
char    	*progname = "mkque";
boolean		qdefault;		/* indicates that (default) queue should be put at top */

extern boolean palladium_inst;

/*----Function declarations */
struct quelist	*searchqd();
FILE		*open_stream();



/*====MAIN PROGRAM MODULE */
main(argc,argv)
int     argc;		/* arg count */
char    **argv;		/* arg list */
{
	struct quelist	*quedevlist;	/* Main list that reflects qconfig file */
	struct quelist	*parmlist;	/* desired que:devs from user parameters */
	struct stat statb;

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_QCADM, NL_CAT_LOCALE);
	/*----Acquire a write lock on /etc/qconfig */
	lock_qconfig(1);

	/*----Read in the qconfig file and set up queue:device list */
	read_qconfig(&quedevlist);

        /*----Get the arguments */
        read_args(argc,argv,&parmlist);

	/*----See if queue already exists */
	if(searchqd(parmlist->qname,parmlist->dname,quedevlist) != NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGEXST,
			"The queue, %s: already exists in qconfig file."),
			parmlist->qname);

	/*----Add desired queue */
	create_queue(parmlist,quedevlist);
	qexit((int)EXITOK);
}

/*====READ ALL ARGUMENTS INTO USER DESIRED LIST */
read_args(argc,argv,pqdlist)
int             argc;		/* arg count */
char            **argv;		/* arg list */
struct quelist	**pqdlist;	/* desired que:devs to print */
{
        extern char     *optarg;
        extern int      optind;
	int		i;		/* loop counter */
	char		state = 0 ;	/* present state */
        int             thisarg;	/* current letter argument */
	char		tmpstr[MAXLINE];/* for parsing input clause */

        /*----Check for no. of args */
        if(argc < 2)
                usage();

        /*----Get all the arguments and pass through */
	qdefault = FALSE;
	*pqdlist = NULL;
        while((thisarg = getopt(argc,argv,OURARGS)) != EOF)
	{
                switch(thisarg)
		{
                case ARGQUE:
			state = 1 ;
			addqd(optarg,"",FALSE,pqdlist);
			break;

		case ARGARG:
			if (! *pqdlist)
				usage();
			/*----Check if attribute has no value */
			chk_empty(optarg);
			/*----Add a clause, if not "device =" */
			parseline(optarg,tmpstr);
			if(!strcmp(tmpstr,DEVICES))
				deverr();
			addcl(optarg,&((*pqdlist)->clauses));
			break;

		case ARGDFLT:
			qdefault = TRUE;
			break;

                default:
                        /*----Anything else is an error */
                        usage();
                } /* case */
	}
	/*----Check to see if we at least got the queue name */
	if( ! state )
		usage();

	/*----Reverse the order of the list and exit */
	qdinvert(pqdlist);
	clinvert(&((*pqdlist)->clauses));
        return;
}

/*====CREATE THE DESIRED QUEUE, ADD TO FILE */
create_queue(pqdlist,qdlist)
struct quelist  *pqdlist;               /* user list of queues to act upon */
struct quelist  *qdlist;                /* main list of qconfig queues */
{
	FILE			*rfile;			/* input stream that is opened */
	FILE			*wfile;			/* output stream to write to */
	struct quelist		*thisqd;		/* current queuedev pointer */
	char			thisline[MAXLINE];	/* for constructing line images */
        char                    tmpfilename[PATH_MAX];  /* name of temporary file */
	boolean			skipline=TRUE;

        /*----Open the qconfig file, and a temp file to write to */
        rfile = open_stream(QCONFIG,"r");
        open_temp(TMPDIR,tmpfilename);
        wfile = open_stream(tmpfilename,"w");

	/*----Write the stanza if it is to be the default queue */
	if(qdefault == TRUE)
	{
		do
		{
			/* SKIP blank lines and comment lines */
			if ( readln(rfile,thisline) == NOTOK ) {
				/*** no other queues in qconfig ***/
				write_stanza(wfile,pqdlist,TRUE);
				/*--- these lines ensure that the qconfig file
		      		digests correctly in our trial digest */
				fprintf(wfile,"\tdevice = %s\n",DUMMY);
				fprintf(wfile,"%s:\n",DUMMY);
				fprintf(wfile,"\tbackend = %s\n",DUMMY);
				writln(wfile,"\0");
				writln(wfile,thisline);
				skipline = FALSE;
				break;
			}
			switch (thisline[0]) {
				case '\0':
				case '*':
					writln(wfile,thisline);
					break;
				default:
					write_stanza(wfile,pqdlist,TRUE);
					/*--- these lines ensure that the qconfig file
		      			digests correctly in our trial digest */
					fprintf(wfile,"\tdevice = %s\n",DUMMY);
					fprintf(wfile,"%s:\n",DUMMY);
					fprintf(wfile,"\tbackend = %s\n",DUMMY);
					writln(wfile,"\0");
					writln(wfile,thisline);
					skipline = FALSE;
					break;
			}
		} while (skipline == TRUE);
	}

	/*----Pass through the entire file and write as read in */
	thisqd = qdlist;
	while(readln(rfile,thisline) != NOTOK)
		writln(wfile,thisline);

	/*----Write the stanza if not default queue */
	if(qdefault != TRUE)
	{
		write_stanza(wfile,pqdlist,TRUE);
		/*--- these lines ensure that the qconfig file
		      digests correctly in our trial digest */
		fprintf(wfile,"\tdevice = %s\n",DUMMY);
		fprintf(wfile,"%s:\n",DUMMY);
		fprintf(wfile,"\tbackend = %s\n",DUMMY);
	}

	/*----Close the stream data files */ 
	fclose(rfile);
	fclose(wfile);

	/*----Do a trial digest and goose qdaemon if OK */
	if( trydigest(tmpfilename) == 0)
		dogoose(tmpfilename);
	return;
}

/*====PRINT USAGE MESSAGE AND EXIT */
usage()
{
	sysuse( TRUE,
		MSGSTR(MSGUSE3,"-D -q<queue> -a<clause1> [-a<clause2> ...]"),
		(char *)0
	      );
}

/*====PRINT ERROR MESSAGE FOR TRYING TO CHANGE A DEVICES= CLAUSE */
deverr()
{
	syserr((int)EXITFATAL,MSGSTR(MSGNDVA,"Illegal attempt to add/change devices clause."));
}
