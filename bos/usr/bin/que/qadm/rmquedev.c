static char sccsid[] = "@(#)81	1.18.1.2  src/bos/usr/bin/que/qadm/rmquedev.c, cmdque, bos411, 9428A410j 12/16/93 15:42:30";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: rmquedev
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
#include <locale.h>
#include <sys/limits.h>
#include "common.h"
#include "qcadm.h"
#include <ctype.h>

#include "qcadm_msg.h"
nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_QCADM,num,str)

/*----Input arguments */
#define OURARGS "d:q:"
#define ARGQUE 	'q'
#define ARGDEV	'd'
#define BIT_ARGQUE  0x1
#define BIT_ARGDEV  0x2

/*----State machine defines */
#define STLKQUE	1
#define STLKDEV 2

char    	*progname = "rmquedev";

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

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_QCADM, NL_CAT_LOCALE);

	/*----Acquire a write lock on /etc/qconfig */
	lock_qconfig(1);

	/*----Read in the qconfig file and set up queue:device list */
	read_qconfig(&quedevlist);

        /*----Get the arguments */
        read_args(argc,argv,&parmlist);

	/*----Mark the elements in main list that are in desired list */
	mark_items(parmlist,quedevlist);

	/*----Scan main list and print desired queues */
	delete_quedevs(parmlist,quedevlist);
	qexit((int)EXITOK);
}

/*====READ ALL ARGUMENTS AND PUT INTO USER DESIRED LIST */
read_args(argc,argv,pqdlist)
int             argc;      /* arg count */
char            **argv;    /* arg list */
struct quelist  **pqdlist; /* desired que:devs to print */
{
	extern char     *optarg;
	extern int      optind;
	char            *currque = NULL ;/* current queue as read in */
	char            state = 0 ;      /* machine state */
	int             thisarg;         /* read in argument */
	char            tmpstr[MAXLINE]; /* for parsing input clause */
	struct quelist  *qptr ;

	/*----Check for no. of args */
	if(argc < 2)
		usage();

	/*----Get all the arguments and pass through */
	*pqdlist = NULL;
	while((thisarg = getopt(argc,argv,OURARGS)) != EOF)
	{
		switch(thisarg)
		{
		case ARGQUE:
			state |= BIT_ARGQUE ;
			currque = optarg;
			TRUNC_NAME( currque, QNAME ) ;
			for ( qptr = *pqdlist ; qptr ; qptr = qptr->next )
				strcpy( qptr->qname, currque ) ;
			break;

		case ARGDEV:
			state |= BIT_ARGDEV ;
			addqd(currque,optarg,FALSE,pqdlist);
			break;

		default:
			/*----Anything else is an error */
			usage();
		} /* case */
	}
	/*----Check if both device and queue were entered by user */
	if( state != (BIT_ARGQUE | BIT_ARGDEV) )
		usage();

	/*----Check for bad additional arguments */
	if(argv[optind] != NULL)
		usage();

	/*----Reverse the order of the list and exit */
	qdinvert(pqdlist);
	return;
}

/*====MARK ALL ITEMS IN THE MAIN LIST THAT ARE IN USER DESIRED LIST */
mark_items(pqdlist,qdlist)
struct quelist	*pqdlist;		/* user desired list */
struct quelist	*qdlist;		/* main que/dev list */
{
	struct quelist	*thisqd;
	struct quelist	*thispqd;

	/*----Set all items in main list to "not print" */
	set_listflags(qdlist,FALSE);

	/*----For each item in desired list... */
	for(thispqd = pqdlist;
	    thispqd != NULL;
	    thispqd = thispqd->next)
	{
		/*----Search for it in the main list, flag if found */
		thisqd = searchqd(thispqd->qname,thispqd->dname,qdlist);
		if(thisqd == NULL)
			/*----User error if not found */
                        syserr((int)EXITFATAL,MSGSTR(MSGQDND,
                               "Queue:device, %s:/%s: not found in qconfig file. Not deleted."),
                               thispqd->qname,thispqd->dname);

		/*----All ok, flag it */
		thisqd->flag = TRUE;
	}
	return;
}

/*====SCAN THE MAIN QUEUE LIST AND DELETE DESIRED QUEUES */
delete_quedevs(pqdlist,qdlist)
struct quelist	*pqdlist;		/* user list of queues to act upon */
struct quelist	*qdlist;		/* main list of qconfig queues */
{
	int			type;			/* type of qstatus line parsed */
	FILE    		*rfile;			/* stream for reading from */
	FILE			*wfile;			/* stream for writing to */
	struct quelist		*thisqd;		/* current place in master list */
	boolean			printit;		/* indicates whether to print line or not */
	boolean			dummy_dev;		/* add a dummy device for digest to swallow */
	char			tmpstr[MAXLINE];	/* junk heap of garbage */
        char                    thisline[MAXLINE];	/* current line read in from qconfig */
	char			tmpfilename[PATH_MAX];	/* name of temporary file */

        /*----Open the qconfig file, and temporary file */
        rfile = open_stream(QCONFIG,"r");
        open_temp(TMPDIR,tmpfilename);
        wfile = open_stream(tmpfilename,"w");

	/*----For each user desired queue/device, omit it */
	thisqd = qdlist;
	printit = TRUE;
	dummy_dev = FALSE;

        /* readln2 does not ignore dummy lines, D30039 */
        while(readln2(rfile,thisline) != NOTOK)
        {
		/*----Parse the line */
		type = parseline(thisline,tmpstr);

		/*---Do what you gotta do */
		switch(type)
		{
                case TYPNAME:
                        /*----New stanza, turn on printing */
                        printit = TRUE;

                        /*---Turn off printing if stanza is flagged */
                        if(thisqd->flag == TRUE)
                                printit = FALSE;

                        /*----Print if not deleted */
			if(printit == TRUE)
				writln(wfile,thisline);
			else
			{
				/*
				 * D16105:
				 *	Delete the status file for this
				 *	queue/device.
				 */
				struct d temp_d;
				struct q temp_q;
				strcpy(temp_q.q_name, thisqd->qname);
				temp_d.d_q = &temp_q;
				strcpy(temp_d.d_name, thisqd->dname);
				unlink(stname(&temp_d));
				/*----Deleted, print dummy device stanza, if needed */
				if(dummy_dev == TRUE)
				{
					fprintf(wfile,"%s:\n",DUMMY);
					fprintf(wfile,"\tbackend = %s\n",DUMMY);
					dummy_dev = FALSE;
				}
			}

                        /*----Point to next item in master list (should correspond
                                to what is being read in from the stream) */

                        /* D30039, if dummy stanza, don't point to next */
                        if (strncmp(DUMMY,thisline,6))
                           thisqd = thisqd->next;

			break;

                case TYPASSG:
			/*----Adjust any "device =" clause */
			if(!strcmp(tmpstr,DEVICES))
			{
				adj_devices(thisline,thisqd->qname,pqdlist);
	
				/*----Empty line means add DUMMY device */
				if(thisline[0] == '\0')
				{
					fprintf(wfile,"\tdevice = %s\n",DUMMY);
					dummy_dev = TRUE;
				}
				else
					/*----Print normal device clause */
					writln(wfile,thisline);
			}
			else
				/*----Print if not deleted and not empty string */		
                        	if(printit == TRUE &&
				   thisline[0] != '\0')
					writln(wfile,thisline);
			break;

		default:
			/*----Just print anything else */
			writln(wfile,thisline);
		} /* case */
	} /* while */

	/*----Close the stream data file */ 
	fclose(rfile);
	fclose(wfile);

	/*----Do a trial digest and goose qdaemon if OK */
	if( trydigest(tmpfilename) == 0)
		dogoose(tmpfilename);
	return;
}

/*====DELETE DEVICE(S) FROM THE "DEVICE =" CLAUSE */
adj_devices(line,currque,pqdlist)
char		*line;		/* line with "device =" to delete device from */
char		*currque;	/* name of queue we are currently scanning */
struct quelist	*pqdlist;	/* list of user desired devices to delete */
{
	struct quelist	*devslist;	/* list of devices extracted from clause */
	struct quelist	*thisdqd;	/* for scanning devslist */
	struct quelist	*thispqd;	/* for scanning user list */
	boolean		firstone;	/* for controlling comma and case of no devices */

	/*----Extract line text into linked list */
	devslist = NULL;
	getdevs(line,currque,&devslist,FALSE);
	qdinvert(&devslist);

	/*----Mark items in devslist that are in users list to delete */
	for(thisdqd = devslist;
	    thisdqd != NULL;
	    thisdqd = thisdqd->next)
		for(thispqd = pqdlist;
		    thispqd != NULL;
		    thispqd = thispqd->next)
			if(!strncmp(thisdqd->qname,thispqd->qname,QNAME) &&
			   !strncmp(thisdqd->dname,thispqd->dname,DNAME)  )
				thisdqd->flag = TRUE;

	/*----Re-construct the "device =" clause */
	line[0] = '\0';
	sprintf(line,"\t%s = ",DEVICES);
	for(thisdqd = devslist,firstone = TRUE;
	    thisdqd != NULL;
	    thisdqd = thisdqd->next)
		if(thisdqd->flag == FALSE)
		{
			if(firstone == TRUE)
				firstone = FALSE;
			else
				strcat(line,",");
			strcat(line,thisdqd->dname);
		}

	/*----Check for no device at all, nullify string if so */
	if(firstone == TRUE)
		line[0] = '\0';

	/*----Return used memory space to garbage, adios */
	dumpqd(devslist);
	return;
}

/*====PRINT USAGE MESSAGE AND EXIT */
usage()
{
	sysuse( TRUE,
		MSGSTR(MSGUSE6,"-q<queue> -d<device> ..."),
		(char *)0
	      );
}
