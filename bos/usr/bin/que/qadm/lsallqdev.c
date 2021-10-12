static char sccsid[] = "@(#)71	1.14  src/bos/usr/bin/que/qadm/lsallqdev.c, cmdque, bos411, 9428A410j 12/16/93 15:41:59";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS:
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
#include <time.h>
#include "common.h"
#include "qcadm.h"
#include <ctype.h>

#include "qcadm_msg.h"
nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_QCADM,num,str)

/*----Input arguments */
#define OURARGS "q:c"
#define ARGQUE  'q'
#define ARGCOL  'c'
int cflag = 0;

/*----Misc. */
char    	*progname = "lsallqdev";

/*----Function declarations */
struct quelist	*searchqd();
FILE		*open_stream();



/*====MAIN PROGRAM MODULE */
main(argc,argv)
int     argc;		/* arg count */
char    **argv;		/* arg list */
{
	struct quelist	*quedevlist;	/* Main list that reflects qconfig file */
	struct quelist	*parmlist;	/* User Input parameter list */

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_QCADM, NL_CAT_LOCALE);

	/*----Acquire a read lock on /etc/qconfig */
	lock_qconfig(0);

	/*----Read in the qconfig file and set up queue:device list */
	read_qconfig(&quedevlist);

        /*----Get the arguments */
        read_args(argc,argv,&parmlist);

	/*----Scan main list and print desired ques */
	print_queues(parmlist,quedevlist);
	qexit((int)EXITOK);
}

/*====READ ALL ARGUMENTS INTO USER DESIRED LIST */
read_args(argc,argv,pqdlist)
int             argc;           /* arg count */
char            **argv;         /* arg list */
struct quelist  **pqdlist;      /* desired que:devs to print */
{
        extern char     *optarg;
        extern int      optind;
	boolean		added;
        int             thisarg;

        /*----Check for no. of args */
        if(argc < 2)
                usage();

        /*----Get all the arguments and pass through */
	added = FALSE;
        *pqdlist = NULL;
        while((thisarg = getopt(argc,argv,OURARGS)) != EOF)
        {
                switch(thisarg)
                {
		/*----Allow only one -q argument */
                case ARGQUE:
			if(added == FALSE)
			{
				added = TRUE;
                        	addqd(optarg,"",FALSE,pqdlist);
			}
			else
				usage();
                        break;

		/*----put out as que:device  */
                case ARGCOL:
			cflag++;
			break;

                default:
                        /*----Anything else is an error */
                        usage();
                } /* case */
        }
        /*----Check to see if at least one queue was entered by user */
        if(*pqdlist == NULL)
                usage();

        /*----Check for bad additional arguments */
        if(argv[optind] != NULL)
                usage();
        return;
}

/*====SCAN THE MAIN QUEUE LIST AND PRINT NAMES OF QUEUES */
print_queues(pqdlist,qdlist)
struct quelist	*pqdlist;	/* user input parameter list */
struct quelist	*qdlist;	/* main list from qconfig */
{
	int			type;			/* type of qstatus line parsed */
	FILE    		*rfile;			/* stream that is opened */
	struct quelist		*thisqd;		/* current place in master list */
	char			tmpstr[MAXLINE];	/* junk heap of garbage */
        char                    thisline[MAXLINE];	/* current line read in from qconfig */

        /*----Open the qconfig file */
        rfile = open_stream(QCONFIG,"r");

	/*----For each user desired queue, print it out */
	thisqd = qdlist;
	while(readln(rfile,thisline) != NOTOK)
        {
		/*----Parse the line */
		type = parseline(thisline,tmpstr);

		/*---Do what you gotta do */
		if(type == TYPNAME)
		{
			/*----Stanza name, print name if a queue */
			if(!strncmp(thisqd->qname,pqdlist->qname,QNAME) &&
			   !strncmp(thisqd->dname,tmpstr,DNAME))
			{
				if (cflag)
					printf("%s:",pqdlist->qname);
				printf("%s\n",tmpstr);			
			}

			/*----Point to next item in master list (should correspond
				to what is being read in from the stream) */
			thisqd = thisqd->next;
		} /* if */
	} /* while */

	/*----Close the stream data file */ 
	fclose(rfile);
	return;
}

/*====PRINT USAGE MESSAGE AND EXIT */
usage()
{
	sysuse( TRUE,
		MSGSTR(MSGUSED,"[-c] -q<queue>"),
		(char *)0
	      );
}

