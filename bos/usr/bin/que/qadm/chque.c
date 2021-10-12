static char sccsid[] = "@(#)11	1.20.1.3  src/bos/usr/bin/que/qadm/chque.c, cmdque, bos411, 9428A410j 3/18/94 09:30:37";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: chque
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
#include "common.h"
#include "qcadm.h"
#include <ctype.h>

#include "qcadm_msg.h"
nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_QCADM,num,str)

/*----Input arguments */
#define OURARGS "q:a:"
#define ARGQUE 	'q'
#define ARGARG	'a'

/*----State machine defines */
#define	STLKQUE	1
#define STLKARG 2

/*----Misc. */
char    	*progname = "chque";

/*----Function declarations */
struct quelist	*searchqd();
FILE		*open_stream();



/*====MAIN PROGRAM MODULE */
main(argc,argv)
int     argc;		/* arg count */
char    **argv;		/* arg list */
{
	struct quelist	*quedevlist;	/* Main list that reflects qconfig file */
	struct parms	params;		/* User parameters */

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_QCADM, NL_CAT_LOCALE);

	/*----Acquire a write lock on /etc/qconfig */
	lock_qconfig(1);

	/*----Read in the qconfig file and set up queue:device list */
	read_qconfig(&quedevlist);

        /*----Get the arguments */
        read_args(argc,argv,&params);

	/*----See if queue already exists */
	if(searchqd(params.queues->qname,params.queues->dname,quedevlist) == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGNXST,
			"The queue, %s: does not exist in qconfig file."),
			params.queues->qname);

	/*----Change desired queue */
	change_queue(&params,quedevlist);
	qexit((int)EXITOK);
}

/*====READ ALL ARGUMENTS INTO USER DESIRED LIST */
read_args(argc,argv,params)
int             argc;		/* arg count */
char            **argv;		/* arg list */
struct parms	*params;	/* user parameters */
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
	params->deflt = FALSE;
	params->queues = NULL;
        while((thisarg = getopt(argc,argv,OURARGS)) != EOF)
	{
                switch(thisarg)
		{
                case ARGQUE:
                        state = 1 ;
                        addqd(optarg,"",FALSE,&(params->queues));
                        break;

                case ARGARG:
			if (! params->queues)
				usage();
			/*----Check if attribute has no value */
                        chk_empty(optarg);
			/*----Add a clause, if not "device =" */
			parseline(optarg,tmpstr);
			if(!strcmp(tmpstr,DEVICES))
				deverr();
			addcl(optarg,&(params->queues->clauses));
			break;

                default:
                        /*----Anything else is an error */
                        usage();
                } /* case */
	}
	/*----Check if we got at least the queue name */
	if( ! state )
		usage();

	/*----Reverse the order of the list and exit */
	qdinvert(&(params->queues));
	clinvert(&(params->queues->clauses));
        return;
}

/*====CHANGE THE DESIRED QUEUE */
change_queue(params,qdlist)
struct parms    *params;                /* user list of queues to act upon */
struct quelist  *qdlist;                /* main list of qconfig queues */
{
        int                     type;                   /* type of qstatus line parsed */
        FILE                    *rfile;                 /* stream for reading from */
        FILE                    *wfile;                 /* stream for writing to */
        struct quelist          *thisqd;                /* current place in master list */
        char                    tmpstr[MAXLINE];        /* place to store line parsing results */
	char			tmpitem[MAXLINE];	/* place to store assignment line item */
        char                    thisline[MAXLINE];      /* current line read in from qconfig */
        char                    tmpfilename[PATH_MAX];  /* name of temporary file */
        boolean                 queactive;              /* indicates we are in the queue to make
							   changes to */
	struct clause		*stzclauses;		/* clauses stored for particular queue */
	struct clause		*thisclause;		/* temp clause list pointer */

        /*----Open the qconfig file, and a temp file to write to */
        rfile = open_stream(QCONFIG,"r");
        open_temp(TMPDIR,tmpfilename);
        wfile = open_stream(tmpfilename,"w");

	/*----Pass through the entire file and write as read in */
	thisqd = qdlist;
	stzclauses = NULL;
	queactive = FALSE;
	set_clauseflags(params->queues->clauses,TRUE);
	while(readln(rfile,thisline) != NOTOK)
        {
                /*----Parse it for line type */
                type = parseline(thisline,tmpstr);

                /*---Do what you gotta do */
                switch(type)
                {
                case TYPNAME:
			/*----If we were in active que, finish up */
			if(queactive == TRUE)
				/*----Scan clauses left over and print them out */
				for(thisclause = params->queues->clauses;
				    thisclause != NULL;
				    thisclause = thisclause->cnext)
					if(thisclause->cflag == TRUE)
					{
						sprintf(tmpitem,"\t%s",thisclause->ctext);
						writln(wfile,tmpitem);
					}

                        /*----Write the stored clauses */
                        spew_clauses(wfile,&stzclauses);
                        queactive = FALSE;

                        /*----See if queue name matches desired queue, if so set flag */
                        if(!strcmp(thisqd->dname,"") &&
                           !strncmp(thisqd->qname,params->queues->qname,QNAME))
                        	queactive = TRUE;


                        /*----Send this line straight to output */
                       writln(wfile,thisline);

                        /*----Point to next item in master list (should correspond
                                to what is being read in from the stream) */
                        thisqd = thisqd->next;
                        break;

                case TYPASSG:
                        /*----If we are in the proper queue, handle change or addition of
                              device clause */
                        if(queactive)
                        {
                                /*----Scan the desired clause list and look for replacement */
				for(thisclause = params->queues->clauses;
				    thisclause != NULL;
				    thisclause = thisclause->cnext)
				{
					parseline(thisclause->ctext,tmpitem);
					if(!strcmp(tmpitem,tmpstr))
					{
						thisclause->cflag = FALSE;
						sprintf(tmpitem,"\t%s",thisclause->ctext);
						addcl(tmpitem,&stzclauses);
						break;
					}
				} /* for */
				/*----If no match found, send original through */
				if(thisclause == NULL)
					addcl(thisline,&stzclauses);
                        } /* if */
                        else
                                addcl(thisline,&stzclauses);
                        break;

                default:
                        /*----Just print  or add anything else */
                        addcl(thisline,&stzclauses);
		} /* case */
	} /* while */
	/*----Wrap up if we are changing last stanza */
        if(queactive == TRUE)
                /*----Scan clauses left over and print them out */
        	for(thisclause = params->queues->clauses;
                    thisclause != NULL;
                    thisclause = thisclause->cnext)
                	if(thisclause->cflag == TRUE) {
				sprintf(thisline,"\t%s",thisclause->ctext);
				writln(wfile,thisline);
			}

        /*----Write the stored clauses */
        spew_clauses(wfile,&stzclauses);

	/*----Close the stream data files */ 
	fclose(rfile);
	fclose(wfile);

	/*----Do a trial digest and goose qdaemon if OK */
	if( trydigest(tmpfilename) == 0)
		dogoose(tmpfilename);
	return;
}

/*====PRINT ERROR MESSAGE FOR TRYING TO CHANGE A DEVICES= CLAUSE */
deverr()
{
	syserr((int)EXITFATAL,MSGSTR(MSGNODV,"Illegal attempt to change devices clause."));
}

/*====PRINT USAGE MESSAGE AND EXIT */
usage()
{
	sysuse( TRUE,
		MSGSTR(MSGUSE7C,"-q<queue>"),
		MSGSTR(MSGUSE7B,"-a<clause1> [-a<clause2> ...]"),
		(char *)0
	      );
}
