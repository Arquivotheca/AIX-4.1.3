static char sccsid[] = "@(#)70	1.15  src/bos/usr/bin/que/qadm/lsallq.c, cmdque, bos411, 9428A410j 12/16/93 15:41:53";
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

/*----Misc. */
char    	*progname = "lsallq";
#define LSALLQDEV	"/usr/bin/lsallqdev" 	/* this really should be in /usr/lpd */
/*----Function declarations */
struct quelist	*searchqd();
FILE		*open_stream();



/*====MAIN PROGRAM MODULE */
main(argc,argv)
int     argc;		/* arg count */
char    **argv;		/* arg list */
{
	struct quelist	*quedevlist;	/* Main list that reflects qconfig file */
	int cflag = 0;			/* if set output list in SMIT form */

	/*----NLS stuff */
	(void)setlocale(LC_ALL,"");
	catd = catopen(MF_QCADM, NL_CAT_LOCALE);

	/*----Acquire a read lock on /etc/qconfig */
	lock_qconfig(0);

	/*----Read in the qconfig file and set up queue:device list */
	read_qconfig(&quedevlist);

	/*----Check for proper argument count */
        if(argc > 1 && strcmp(argv[1],"-c") == 0)
		cflag++;
	else if (argc > 1)
                usage();

	/*----Scan main list and print desired ques */
	print_queues(quedevlist,cflag);
	qexit((int)EXITOK);
}

/*====SCAN THE MAIN QUEUE LIST AND PRINT NAMES OF QUEUES */
print_queues(qdlist,mode)
struct quelist	*qdlist;	/* main list from qconfig */
int mode;
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
			if(!strncmp(thisqd->qname,tmpstr,QNAME) &&
			   !strcmp(thisqd->dname,""))  
			{ 
				printf("%s\n",tmpstr);			

				/*---- print device list too */
				if (mode)
				{
					char cmd[MAXPATH];

					sprintf(cmd,"%s -q%s -c",LSALLQDEV,tmpstr);
					system(cmd);
				}
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
		MSGSTR(MSGUSEQ,"[-c]"),
		(char *)0
	      );
}

