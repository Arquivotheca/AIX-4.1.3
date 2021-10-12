static char sccsid[] = "@(#)79	1.19.1.4  src/bos/usr/bin/que/qadm/mkquedev.c, cmdque, bos41J, 9517B_all 4/7/95 11:32:46";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: mkquedev
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
#include <time.h>
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
char    	*progname = "mkquedev";

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

	/*----See if queue exists */
	if(searchqd(parmlist->qname,"",quedevlist) == NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGNXST,
			"The queue, %s: does not exist in the qconfig file."),
			parmlist->qname);
	/*----See if queue already exists, but device does not */
	if(searchqd(parmlist->qname,parmlist->dname,quedevlist) != NULL)
		syserr((int)EXITFATAL,MSGSTR(MSGDXST,
			"The device, %s: already exists in the qconfig file."),
			parmlist->dname);

	/*----Add desired queue:device */
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
	int             i;               /* loop counter */
	char            *currque = NULL ;/* queue most recently read in */
	char            state = 0 ;      /* present state */
	int             thisarg;         /* current letter argument */
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

		case ARGARG:
			if (! *pqdlist)
				usage();
			/*----Check if attribute has no value */
                        chk_empty(optarg);
			/*----Add a clause */
			addcl(optarg,&((*pqdlist)->clauses));
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
	qdinvert(pqdlist);
	clinvert(&((*pqdlist)->clauses));
	return;
}

/*====CREATE THE DESIRED DEVICE, ADD TO END OF FILE */
create_queue(pqdlist,qdlist)
struct quelist  *pqdlist;               /* user list of queues to act upon */
struct quelist  *qdlist;                /* main list of qconfig queues */
{
        int                     type;                   /* type of qstatus line parsed */
        FILE                    *rfile;                 /* stream for reading from */
        FILE                    *wfile;                 /* stream for writing to */
        struct quelist          *thisqd;                /* current place in master list */
	char			tmpstr[MAXLINE];	/* place to store line parsing results */
        char                    thisline[MAXLINE];      /* current line read in from qconfig */
        char                    tmpfilename[PATH_MAX];  /* name of temporary file */
	boolean			lookfrdev;		/* indicates we are looking for
							   device = clause */
	boolean			placedev;		/* indicated that device stanza should 
							   be printed before new stanza read in */
	struct clause		*stzclauses;		/* clauses stored for particular queue */


        /*----Open the qconfig file, and temporary file */
        rfile = open_stream(QCONFIG,"r");
        open_temp(TMPDIR,tmpfilename);
        wfile = open_stream(tmpfilename,"w");

        /*----For each line in qconfig, read in a line... */
	thisqd = qdlist;
	stzclauses = NULL;
	lookfrdev = FALSE;
	placedev = FALSE;
	while(readln(rfile,thisline) != NOTOK)
        {
                /*----Parse it for line type */
                type = parseline(thisline,tmpstr);

                /*---Do what you gotta do */
                switch(type)
                {
                case TYPNAME:
			/*----If device= clause not found (queue w/no devices),
			      construct the device= clause and write it. */
			if(lookfrdev == TRUE)
			{
				sprintf(tmpstr,"\t%s = %s",DEVICES,pqdlist->dname);
				writln(wfile,tmpstr);
				placedev = TRUE;
			}
			/*----Write the stored clauses */
			spew_clauses(wfile,&stzclauses);
			lookfrdev = FALSE;
				
			/*----Write the devices stanza in the proper place */
			if(placedev == TRUE && !strcmp(thisqd->dname,""))
			{
				write_stanza(wfile,pqdlist,FALSE);
				placedev = FALSE;
			}

			/*----Send this line straight to output */
                        writln(wfile,thisline);

			/*----See if queue name matches desired queue, if so set flag */
			if(!strcmp(thisqd->dname,""))
				if(!strncmp(thisqd->qname,pqdlist->qname,QNAME))
					lookfrdev = TRUE;

                        /*----Point to next item in master list (should correspond
                                to what is being read in from the stream) */
                        thisqd = thisqd->next;
                        break;

                case TYPASSG:
                        /*----If we are in the proper queue, handle change or addition of
			      device= clause */
			if(lookfrdev == TRUE && !strcmp(tmpstr,DEVICES))
			{
				/*----Add device to device= clause */
			   	strcat(thisline,",");
				strcat(thisline,pqdlist->dname);
				writln(wfile,thisline);
				lookfrdev = FALSE;
				placedev = TRUE;
			}
                        else
				addcl(thisline,&stzclauses);
                        break;

                default:
                        /*----Just print  or add anything else */
			addcl(thisline,&stzclauses);
                } /* case */
        } /* while */
	/*----If we reached the end of the qconfig file and we are still looking
	      for a device= clause, construct it for the newly added device. */
	if(lookfrdev == TRUE)
	{
		sprintf(tmpstr,"\t%s = %s",DEVICES,pqdlist->dname);
		writln(wfile,tmpstr);
		placedev = TRUE;
	}
	/*----Write rest of clauses and write out the new devices stanza if needed */
	spew_clauses(wfile,&stzclauses);
	if(placedev == TRUE)
		write_stanza(wfile,pqdlist,FALSE);

	/* need to sleep so that we don't finish before we start in
	qdaemons mind */
	sleep (2);

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
		MSGSTR(MSGUSE4,"-q<queue> -d<device> -a<clause1> [-a<clause2> ...]"),
		(char *)0
	      );
}
