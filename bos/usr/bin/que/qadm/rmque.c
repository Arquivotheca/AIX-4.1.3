static char sccsid[] = "@(#)80	1.14.1.2  src/bos/usr/bin/que/qadm/rmque.c, cmdque, bos411, 9428A410j 12/16/93 15:42:25";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: rmque
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
#include <string.h>
#include <locale.h>
#include <time.h>
#include <dirent.h>
#include <sys/limits.h>
#include "common.h"
#include "qcadm.h"
#include <ctype.h>

#include "qcadm_msg.h"
nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_QCADM,num,str)

/*----Input arguments */
#define OURARGS "q:"
#define ARGQUE 	'q'

/*----Misc. */
char    	*progname = "rmque";

/*----Function declarations */
struct quelist	*searchqd();
FILE		*open_stream();



/*====MAIN PROGRAM MODULE */
main(argc,argv)
int     argc;		/* arg count */
char    **argv;		/* agg list */
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

	/*----Mark all the items in the main list that were requested */
	mark_items(parmlist,quedevlist);

	/*----Scan main list and delete desired queues */
	delete_queues(parmlist,quedevlist);
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
        int             thisarg;	/* current letter argument */
	char		tmpstr[MAXLINE];/* for parsing input clause */
	DIR		*dirp;
	struct dirent	*dire;

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
			if ((dirp=opendir(QUEDIR)) == NULL)
				syserr((int)EXITFATAL,MSGSTR(MSGNDIR,"Failure to read queue directory."));

			/* keep adding names as long as they keep coming. */
			while ((dire = readdir(dirp)) != NULL)
			{
				char *name;
 
				name = dire->d_name;
				if( strncmp(optarg,getqn(name),QNAME) == 0)
				syserr((int)EXITFATAL,MSGSTR(MSGNEMP," Queue not empty cannot rename."));
			}
			addqd(optarg,"",FALSE,pqdlist);
			break;

                default:
                        /*----Anything else is an error */
                        usage();
                } /* case */
	}
	/*----Check to see if at least one queue wqs entered */
	if(pqdlist == NULL)
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
struct quelist  *pqdlist;               /* user desired (parameters) list */
struct quelist  *qdlist;                /* main que/dev list */
{
        struct quelist  *thisqd;
        struct quelist  *thispqd;

        /*----Set all items in main list to "not print" */
	set_listflags(qdlist,FALSE);

        /* for each item in desired list... */
        for(thispqd = pqdlist;
            thispqd != NULL;
            thispqd = thispqd->next)
        {
                /*----Search for it in the main list, flag if found */
                thisqd = searchqd(thispqd->qname,thispqd->dname,qdlist);
                if(thisqd == NULL)
                        /*----User error if not found */
                        syserr((int)EXITFATAL,MSGSTR(MSGQUND,
                               "Queue, %s: not found in qconfig file. Not deleted."),
                               thispqd->qname);

		/*----Check for contained devices which are illegal */
		if(!strncmp(thisqd->next->qname,thisqd->qname,QNAME))
			syserr((int)EXITFATAL,MSGSTR(MSGNDEL,
				"Cannot delete %s: Queue contains devices."),
				thisqd->qname);

		/*----All ok, flag it */
		thisqd->flag = TRUE;
        }
        return;
}

/*====SCAN THE MAIN QUEUE LIST AND DELETE DESIRED QUEUES */
delete_queues(pqdlist,qdlist)
struct quelist	*pqdlist;	/* user desired list to delete */
struct quelist	*qdlist;	/* main list from qconfig */
{
	int			type;			/* type of qstatus line parsed */
	FILE    		*rfile;			/* input stream that is opened */
	FILE			*wfile;			/* output stream that is opened */
	struct quelist		*thisqd;		/* current place in master list */
        struct quelist          *thisqd2;               /* to save previous thisqd */
	boolean			printit;		/* indicates whether to print line or not */
	char			tmpstr[MAXLINE];	/* utility storage */
	char			tmpfilename[PATH_MAX];	/* storage for name of temporary file */
        char                    thisline[MAXLINE];	/* current line read in from qconfig */

        /*----Open the qconfig file, and a temp file to write to */
        rfile = open_stream(QCONFIG,"r");
	open_temp(TMPDIR,tmpfilename);
	wfile = open_stream(tmpfilename,"w");

	/*----For each non-user desired queue, print it out */
	thisqd = qdlist;
	printit = TRUE;

        /* readln2 doen not ignore dummy lines, D30039 */
        while(readln2(rfile,thisline) != NOTOK)
        {
		/*----Parse the line */
		type = parseline(thisline,tmpstr);

		/*---Act according to type of line read in */
		switch(type)
		{
		case TYPNAME:
			/*----New stanza, turn off printing */
			printit = TRUE;

                        /* D30039, dummy is not a new stanza, */
                        /* point to the last one */
                        if (!strncmp(DUMMY,thisline,strlen(DUMMY)))
                           thisqd = thisqd2;

			/*---Turn on printing if stanza is flagged */
			if(thisqd->flag == TRUE)
				printit = FALSE;

                        /* D30039 save thisqd,to be used if next stanza=dummy */
                        thisqd2 = thisqd;

			/*----Point to next item in master list (should correspond
				to what is being read in from the stream) */
			thisqd = thisqd->next;
			/* no break; here */

		case TYPASSG:
			/*----Print if printing of this stanza enabled */
			if(printit == TRUE)
				writln(wfile,thisline);
			break;

		default:
			writln(wfile,thisline);
		} /* case */
	} /* while */

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
		MSGSTR(MSGUSE5,"-q<queue> ..."),
		(char *)0
	      );
}
