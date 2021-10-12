static char sccsid[] = "@(#)75	1.14  src/bos/usr/bin/que/qadm/lsque.c, cmdque, bos411, 9428A410j 12/16/93 15:42:05";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: lsque
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
#include "common.h"
#include "digest.h"
#include "qcadm.h"
#include <ctype.h>


#include "qcadm_msg.h"
nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_QCADM,num,str)

/*----Input arguments */
#define OURARGS "cq:"
#define ARGQUE 	'q'
#define ARGCOLN 'c'

/*----Misc. */
char    	*progname = "lsque";
boolean		colnout;		/* Triggers SMIT:colon:output:mode */

/*----Function declarations */
struct quelist	*searchqd();
FILE		*open_stream();
void		fill_list_with_default_values();
void		remember_this_value();
void		print_header_with_colons();
void		print_name_list_with_colons();


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

	/*----Acquire a read lock on /etc/qconfig */
	lock_qconfig(0);

	/*----Read in the qconfig file and set up queue:device list */
	read_qconfig(&quedevlist);

        /*----Get the arguments */
        read_args(argc,argv,&parmlist);

	/*----Mark all the items in the main list that were requested */
	mark_items(parmlist,quedevlist);

	/*----Scan main list and print desired ques */
	print_queues(parmlist,quedevlist);
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
        int             thisarg; 

        /*----Check for no. of args */
        if(argc < 2)
                usage();

        /*----Get all the arguments and pass through */
	*pqdlist = NULL;
	colnout = FALSE;	/* assume regular output */
        while((thisarg = getopt(argc,argv,OURARGS)) != EOF)
	{
                switch(thisarg)
		{
                case ARGQUE:
			/*---Add a queue name */
			addqd(optarg,"",FALSE,pqdlist);
			break;

		case ARGCOLN:
			/*----Smit colon output mode */
			colnout = TRUE;
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
                        syserr((int)EXITFATAL,MSGSTR(MSGQUNP,
                               "Queue, %s: not found in qconfig file. Not printed."),
                               thispqd->qname);

		/*----All ok, flag it */
                thisqd->flag = TRUE;
        }
        return;
}

/*====SCAN THE MAIN QUEUE LIST AND PRINT DESIRED QUEUES */
print_queues(pqdlist,qdlist)
struct quelist	*pqdlist;	/* user desired list */
struct quelist	*qdlist;	/* main list from qconfig */
{
	int			type;			/* type of qstatus line parsed */
	FILE    		*rfile;			/* stream that is opened */
	struct quelist		*thisqd;		/* current place in master list */
	boolean			printit;		/* indicates whether to print line or not */
	char			namestr[MAXLINE];	/* name parsed form the line */
        char                    thisline[MAXLINE];	/* current line read in from qconfig */
	char			queue_name[MAXLINE];	/* name of the queue being printed */
							/* stanza heading */

        /*----Open the qconfig file */
        rfile = open_stream(QCONFIG,"r");

	/*----if colnout is TRUE, print the header for the colon format 
		output */
	if( colnout == TRUE )
		print_header_with_colons( fnames );

	/*----For each user desired queue, print it out */
	thisqd = qdlist;
	printit = FALSE;
	while(readln(rfile,thisline) != NOTOK)
        {
		/*----Parse the line */
		type = parseline(thisline, namestr);

		/*---Do what you gotta do */
		switch(type)
		{
		case TYPNAME:

			/*----If printit is TRUE, this must at least be the
				second time through this loop.  And if colnout
				is TRUE, I need to print the collectd values
				before the are changed by the new stanza.  */
			if( printit == TRUE && colnout == TRUE )
				print_name_list_with_colons( queue_name, fnames);

			/*----New stanza, turn off printing */
			printit = FALSE;

			/*---Turn on printing if stanza is flagged */
			if(thisqd->flag == TRUE)
				printit = TRUE;

			/*----Point to next item in master list (should correspond
				to what is being read in from the stream) */
			thisqd = thisqd->next;

			if( printit == TRUE )
				/*----Handle colon format output */
				if( colnout == TRUE )
				{
					/*----Remember name of this stanza */
					strcpy( queue_name, namestr );
					/*---Only concerned with the default 
						values if user wants listing in
						colon format */
					fill_list_with_default_values( fnames );
				}
				else
					printf( "%s\n", thisline );
			break;

		case TYPASSG:
			/*----Print if printing of this stanza enabled */
			if( printit == TRUE )
				/*---- List normally if the -c flag was not 
					specified */
				if( colnout == FALSE )
					printf("%s\n",thisline);
				/*---- otherwise just remember the value */
				else
					remember_this_value( thisline, fnames );
			break;
		} /* case */
	} /* while */

	/*----If printit is TRUE and colnout is TRUE, then the data collected
		about the last stanza read has not been printed, so do so */
	if( printit == TRUE && colnout == TRUE )
		print_name_list_with_colons( queue_name, fnames );

	/*----Close the stream data file */ 
	fclose(rfile);
	return;
}


/*==== FILLS THE GIVEN LIST OF 'struct namtab' WITH THE APPROPRIATE DEFAULT
       VALUE */
void fill_list_with_default_values( name_list ) 
struct namtab *name_list;
{
	while( name_list->name != NULL )
	{
		name_list->valstr = (char *)scopy( name_list->default_value );
		name_list++;
	}
}


/*==== REMEMBERS THE VALUE ON THE RIGHT SIDE OF THE = IN THE CLAUSE */
void remember_this_value( clause, name_list )
char *clause;
struct namtab *name_list;
{
	char 	*parsing_tokens = " \t=";
	char 	*left_of_equals;
	char 	right_of_equals[MAXLINE];
	char	*tmp;

	/*----Parse the string on the left side of the = */
	if( ( left_of_equals = strtok( clause, parsing_tokens ) ) == NULL )
                mangled(0,clause,MSGSTR(MSGTBAD,"Syntax error in qconfig file."));

	/*----Parse the string on the right side of the = */
	right_of_equals[0] = '\0';
	while( ( tmp = strtok((char *)NULL, parsing_tokens ) ) != NULL )
	{
		if( right_of_equals[0] == '\0' )
			parsing_tokens = " \t";	 /* first time through */
		else /* add the space if more than one thing to parse */
			strcat( right_of_equals, " " );
		strcat( right_of_equals, tmp );
	}
	if( right_of_equals[0] == '\0' )
                mangled(0,clause,MSGSTR(MSGTBAD,"Syntax error in qconfig file."));

	/*----Search the name_list for the value that is equal to the value
		found on the left side of the equals */
	while( strcmp( name_list->name, left_of_equals ) )
	{
		name_list++;
		if( name_list->name == NULL )
        	        mangled(0,clause,MSGSTR(MSGTBAD,"Syntax error in qconfig file."));
	}

	/*----Once found, copy the information on the right of the equals into
		its buffer */
	name_list->valstr = (char *)scopy( right_of_equals );

}


/*====PRINTS THE HEADER FOR THE QUEUE STANZA IN COLON FORMAT */
void print_header_with_colons( name_list )
struct namtab name_list[];
{
	printf( "#%s:%s:%s:%s:%s:%s:%s:%s:%s\n", QUEUE_NAME, 
		name_list[DEVICE_INDEX].name,
		name_list[DISCIPLINE_INDEX].name, 
		name_list[UP_INDEX].name, 
		name_list[ACCTFILE_INDEX].name,
		name_list[HOST_INDEX].name,
		name_list[S_STATFILTER_INDEX].name, 
		name_list[L_STATFILTER_INDEX].name, 
		name_list[RQ_INDEX].name );
}


/*====PRINTS THE QUEUE STANZA IN COLON FORMAT */
void print_name_list_with_colons( queue_name, name_list )
char *queue_name;
struct namtab name_list[];
{
	printf( "%s:%s:%s:%s:%s:%s:%s:%s:%s\n", queue_name, 
		name_list[DEVICE_INDEX].valstr,
		name_list[DISCIPLINE_INDEX].valstr, 
		name_list[UP_INDEX].valstr, 
		name_list[ACCTFILE_INDEX].valstr,
		name_list[HOST_INDEX].valstr,
		name_list[S_STATFILTER_INDEX].valstr, 
		name_list[L_STATFILTER_INDEX].valstr, 
		name_list[RQ_INDEX].valstr );

}


/*====PRINT USAGE MESSAGE AND EXIT */
usage()
{
	sysuse( TRUE,
		MSGSTR(MSGUSE1,"[-c] -qName ..."),
		(char *)0
	      );
}

