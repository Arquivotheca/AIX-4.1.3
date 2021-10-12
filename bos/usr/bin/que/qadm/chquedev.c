static char sccsid[] = "@(#)74	1.21.1.3  src/bos/usr/bin/que/qadm/chquedev.c, cmdque, bos411, 9428A410j 3/18/94 09:30:55";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: chquedev
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
#define OURARGS "q:d:a:"
#define ARGQUE 	'q'
#define ARGDEV	'd'
#define ARGARG	'a'
#define BIT_ARGQUE  0x1
#define BIT_ARGDEV  0x2

/*----State machine defines */
#define	STLKQUE	1
#define STLKDEV 2
#define STLKARG 3

/*----Misc. */
char    	*progname = "chquedev";

/*----Function declarations */
struct quelist	*searchqd();
FILE		*open_stream();



/*====MAIN PROGRAM MODULE */
main(argc,argv)
int     argc;		/* arg count */
char    **argv;		/* arg list */
{
	struct quelist	*quedevlist;	/* Main list that reflects qconfig file */
	struct parms	params;		/* desired que:devs, etc. from user parameters */

	/*----NLS stuff */
	(void)setlocale(LC_ALL,""); 
	catd = catopen(MF_QCADM, NL_CAT_LOCALE);

	/*----Acquire a write lock on /etc/qconfig */
	lock_qconfig(1);

	/*----Read in the qconfig file and set up queue:device list */
	read_qconfig(&quedevlist);

        /*----Get the arguments */
        read_args(argc,argv,&params);

	/*----See if queue:device already exists */
	if(searchqd(params.queues->qname,params.queues->dname,quedevlist) == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGQDXS,
			"The queue and device, %s:%s does not exist in qconfig file."),
			params.queues->qname,params.queues->dname);

	/*----Change desired queue:device */
	change_queue(&params,quedevlist);
	qexit((int)EXITOK);
}

/*====READ ALL ARGUMENTS INTO USER DESIRED LIST */
read_args(argc,argv,params)
int             argc;		/* arg count */
char            **argv;		/* arg list */
struct parms	*params;	/* desired que:devs, etc. to print */
{
	extern char     *optarg;
	extern int      optind;
	int             i;               /* loop counter */
	char            *currque = NULL ;/* queue most recently read in */
	char            state = 0 ;      /* present state */
	int             thisarg;	       /* current letter argument */
	char            tmpstr[MAXLINE]; /* for parsing input clause */
	struct quelist  *qptr ;


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
		case ARGQUE:		/* Queue name */
			state |= BIT_ARGQUE ;
			currque = optarg;
			TRUNC_NAME( currque, QNAME ) ;
			for ( qptr = params->queues ; qptr ; qptr = qptr->next )
				strcpy( qptr->qname, currque ) ;
			break;

		case ARGDEV:		/* Device name */
			state |= BIT_ARGDEV ;
			addqd(currque,optarg,FALSE,&(params->queues));
			break;

		case ARGARG:		/* Device clauses */
			if (! params->queues)
				usage();
			/*----Check if attribute has no value */
                        chk_empty(optarg);
			/*----Add a clause */
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
	/*----Check to see if both queue name and device name were entered by user */
	if( state != (BIT_ARGQUE | BIT_ARGDEV) )
		usage();

	/*----Reverse the order of the list and exit */
	qdinvert(&(params->queues));
	clinvert(&(params->queues->clauses));
        return;
}

/*====CREATE THE DESIRED DEVICE, ADD TO END OF FILE */
change_queue(params,qdlist)
struct parms	*params;           	/* user list of queues, etc. to act upon */
struct quelist  *qdlist;                /* main list of qconfig queues */
{
        int                     type;                   /* type of qstatus line parsed */
        FILE                    *rfile;                 /* stream for reading from */
        FILE                    *wfile;                 /* stream for writing to */
        struct quelist          *thisqd;                /* current place in master list */
	char			tmpstr[MAXLINE];	/* place to store line parsing results */
	char			tmpitem[MAXLINE];	/* place to store line parsing results */
        char                    thisline[MAXLINE];      /* current line read in from qconfig */
        char                    tmpfilename[PATH_MAX];  /* name of temporary file */
	boolean 		devactive;		/* indicates we are in the device to
							   make changes to */
	boolean			lookfrdev;		/* indicates we are looking for
							   device = clause */
	boolean			placedev;		/* indicated that  stanza should 
							   be printed before new stanza read in */
	struct clause		*stzclauses;		/* clauses stored for particular queue */
        struct clause           *thisclause;            /* temp clause list pointer */

        /*----Open the qconfig file, and temporary file */
        rfile = open_stream(QCONFIG,"r");
        open_temp(TMPDIR,tmpfilename);
        wfile = open_stream(tmpfilename,"w");

        /*----For each line in qconfig, read in a line... */
	thisqd = qdlist;
	stzclauses = NULL;
	devactive = FALSE;
	lookfrdev = FALSE;
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
                        if(devactive == TRUE)
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
                        devactive = FALSE;

                        /*----See if queue name matches desired queue, if so set flag */
                        if(!strncmp(thisqd->qname,params->queues->qname,QNAME))
			{
                           	if(!strncmp(thisqd->dname,params->queues->dname,DNAME))
                                	devactive = TRUE;
	
				if(!strcmp(thisqd->dname,""))
					lookfrdev = TRUE;
			}

                        /*----Send this line straight to output */
                        writln(wfile,thisline);

                        /*----Point to next item in master list (should correspond
                                to what is being read in from the stream) */
                        thisqd = thisqd->next;
                        break;

                case TYPASSG:
			/*----Change the devices clause if necessary */
			if(lookfrdev == TRUE && !strcmp(tmpstr,DEVICES))
				lookfrdev = FALSE;


                        /*----Handle clause replacements */
                        if(devactive)
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

	if (devactive)
	{
		for(thisclause = params->queues->clauses;
		    thisclause != NULL;
		    thisclause = thisclause->cnext)
			if(thisclause->cflag == TRUE)
			{
				thisclause->cflag = FALSE;
				sprintf(tmpitem,"\t%s",thisclause->ctext);
				writln(wfile,tmpitem);
			}
	}

	/*----Write rest of clauses */
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
        syserr((int)EXITFATAL,MSGSTR(MSGNDVA,"Illegal attempt to add/change devices clause."));
}

/*====PRINT USAGE MESSAGE AND EXIT */
usage()
{
	sysuse( TRUE,
		MSGSTR(MSGUSE8C,"-q<queue> -d<device>"),
		MSGSTR(MSGUSE8B,"-a<clause1> [-a<clause2> ...]"),
		(char *)0
	      );
}
