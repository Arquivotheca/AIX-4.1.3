static char sccsid[] = "@(#)28  1.6.1.1  src/bos/usr/lib/pios/pioburst.c, cmdpios, bos411, 9428A410j 12/8/93 18:34:06";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*******************************************************************************

   Module:      pioburst.c

   Function:    main

   Description: All functions specifically intended for bursting are contained
		within this module. Bursting can be performed, at the users
		discretion with headers, trailers or both.  Both headers and
		trailers are treated as separate jobs, neither has any know-
		ledge of the other and does not require the other to be pre-
		sent.  In fact, the print job to which they apply has no
		knowledge that a header or trailer ever or will ever exist.
		As a result, the completion status of the headers or trailers
		will in no way impact real print jobs.  Warning messages will
		be issued based on internal status; however, pioburst will
		always exit with a "good" status.

		main() retrieves and stores the arguements passed to pioburst.
		It also opens for input the specified header or trailer file
		if it exists and begins searching for the flag character.
		If the character at the pointer is not a "%" then the char-
		acter is directly output to stdout.  If and when a "%" is
		encountered the data associated with the flag argument is
		output to stdout.  This is done for the entire file.

   ASSUMPTIONS: The user is responsible for the format of the header and
		trailer pages.  There is no attempt made here to guarantee that
		the specified burst page(s) actually help in bursting.

   Arguments:
	Mandatory: one or the other NOT both
		-h: full path name of header text file
				-or-
		-t: full path name of trailer text file
	Optional:
		-H: hostname

   Header/Trailer File Format:
		In general the files are ascii text files with embedded flags
		that indicate the type of field to place at the flag location.
		The flag is a "%", the allowable flags arguments are listed
		below:

		%H      hostname
		%S      person who submitted the job
		%D      person to deliver job to
		%Q      time queued
		%P      time printed
		%T      title of document printed
		%A      formatting flag values
		%B      both formatting flag values and pipeline
		%L      security label
		%%      %

   Parameters:  argc - arguement count
		argv - argument vector

   Return code: PIOEXITGOOD


*******************************************************************************/

#include <stdio.h>
#include <time.h>
#include <locale.h>
#include "piostruct.h"
#include "piobe_msg.h"

#define MSGBUFLEN	(1023)

/* Declarations for message ipc (for Palladium) */
extern uchar_t		piomsgipc;	/* flag to indicate if msgs need be
					   sent via ipc */
extern int	pmi_initmsgipc(void);
extern int	pmi_bldhdrfrm(uchar_t,ushort_t,ushort_t,char const *);
extern int	pmi_bldintfrm(int);
extern int	pmi_bldstrfrm(const char *);
extern int	pmi_dsptchmsg(const char *);

/*
 * Misc. functions
 */
extern long time();
char  *getenv();
char  *find_field_hdgs();
void   print_field_hdgs();
extern char *getmsg();
char *mem_start;
char *hash_table;
int statusfile;

main(argc, argv)
    int argc;
    char *argv[];
{
    extern      char    *optarg;    /* points to argument found */
    extern      int     optind;
    extern      int     opterr;
    extern      int     optopt;
    extern      int     errno;

    char        *hdrpath, *tlrpath, *txtpath, *host, *cp;

    char        pdate[50],
		getname[50];	/* used in gethostname system call */

    int         c, ch, i, cnt,
		next_ch,				    /* next character */
		gnlen,				   /* length of getname array */
		tlen,				 /* max length of time string */
		scratch;

    char        *title,
		*from,
		*to,
		*qdate,
		*devname,
		*ptrtype,
		*flags;

    char        *Title,					  /* %t     job title */
		*Ptime,					  /* %p  time printed */
		*Qtime,					  /* %q   time queued */
		*Printat,				  /* %h    printed at */
		*From,					  /* %s  submitted by */
		*To,					  /* %d    deliver to */
		*Flags,					  /* %a   flag values */
		*End;					  /* %e end of output */

    long        clock;		       /* pointer to time string after time() */

    struct tm   *tdate;					 /* used by strftime */

    FILE        *burstfn;		     /* pointer to specified filename */

    setlocale(LC_ALL, "");

    opterr = 0;				  /* prevent error messages to stderr */
    hdrpath = NULL;
    tlrpath = NULL;
    host = NULL;

    gnlen = sizeof(getname);	     /* used in gethostname system call below */

    /*
    ** Get info. provided by piobe
    */

    title    = getenv("PIOTITLE");
    qdate    = getenv("PIOQDATE");
    from     = getenv("PIOFROM");
    to       = getenv("PIOTO");
    devname  = getenv("PIODEVNAME");
    ptrtype  = getenv("PIOPTRTYPE");
    flags    = getenv("PIOFLAGS");

    /*
    ** First lets check for full path name of either the header or trailer.
    ** If neither of the flags are found or there is not an associated
    ** argument then need to issue a warning message and get the heck out
    ** of dodge.
    */

    while ((c = getopt(argc,argv,"h:t:H:")) != EOF)
	{
	switch(c)
	    {
	    case 'h':			      /* full pathname of header file */
		hdrpath = optarg;
		break;

	    case 't':			     /* full pathname of trailer file */
		tlrpath = optarg;
		break;

	    case 'H':						  /* hostname */
		host = optarg;
		break;

	    case '?':					 /* Unrecognized flag */
		fprintf(stderr, getmsg(MF_PIOBE, 1, MSG_BURSTBAD),
								  (char)optopt);
		return(0);
	    }
	}

    txtpath = argv[optind];

    /*
    ** If we did not get a header or trailer specification, or we got both,
    ** then issue warning and exit.
    */

    if ( (!hdrpath && !tlrpath && !txtpath) || (hdrpath && tlrpath))
	{
	fprintf(stderr, getmsg(MF_PIOBE, 1, MSG_BURSTARG));
	return(0);
	}

    /* Message ipc initialization */
    pmi_initmsgipc();

    /*
    ** Now, try to open the specified burst file.  If it cannot be opened,
    ** output an EOF, issue a warning message and exit.  If it can be opened,
    ** get the Header page field title message (since the titles could also
    ** appear on trailer pages), break them up into their respecitve
    ** start searching for the field delimiter "%"
    */

    if ( !txtpath ) txtpath = hdrpath ? hdrpath : tlrpath;
    if ( !(burstfn = fopen(txtpath, "r")) )
	{
	if (piomsgipc)
	{
	    char		s[MSGBUFLEN+1];

	    (void)sprintf(s,getmsg(MF_PIOBE, 1, MSG_BURSTOPEN),txtpath,errno);
	    (void)pmi_bldhdrfrm(
		ID_VAL_EVENT_ABORTED_BY_SERVER,
		MSG_BURSTOPEN,1,MF_PIOBE);
	    (void)pmi_bldstrfrm(txtpath);
	    (void)pmi_bldintfrm(errno);
	    (void)pmi_dsptchmsg(s);
	}
	else
	   fprintf(stderr,
	    getmsg(MF_PIOBE, 1, MSG_BURSTOPEN), txtpath, errno);
	return(0);
	}

    Title   = getmsg(MF_PIOBE, 1, MSG_HEADER_TITLES);
    Ptime   = find_field_hdgs(Title);
    Qtime   = find_field_hdgs(Ptime);
    Printat = find_field_hdgs(Qtime);
    From    = find_field_hdgs(Printat);
    To      = find_field_hdgs(From);
    Flags   = find_field_hdgs(To);
    End     = find_field_hdgs(Flags);

    /*
    ** All the work pertaining to building a header or trailer takes place from
    ** here on out.  The basic flow is explained below:
    **
    ** PLAIN OLD VANILLA/ NO HITCHES:
    **  Characters are read one at a time from the specified burst file.  If
    **  the character is != "%" (the field flag) the character is simply output
    **  to stdout.  If the character = "%" then the associated argument field is
    **  identified and the appropriate string from the user supplied list or the
    **  statusfile is output to stdout in its place.  This process is repeated
    **  until EOF is encountered at which point main () is exited with a good
    **  status.
    */

    while ((c = getc(burstfn)) != EOF)
	{
	switch ( c )
	    {
	    case '%':
		switch ( c = getc(burstfn) )
		    {
		    case 't':					 /* job title */
			print_field_hdgs(Title);
			break;

		    case 'p':				      /* time printed */
			print_field_hdgs(Ptime);
			break;

		    case 'q':				       /* time queued */
			print_field_hdgs(Qtime);
			break;

		    case 'h':					/* printer at */
			print_field_hdgs(Printat);
			break;

		    case 's':				      /* submitted by */
			print_field_hdgs(From);
			break;

		    case 'd':					/* deliver to */
			print_field_hdgs(To);
			break;

		    case 'a':				       /* flag values */
			print_field_hdgs(Flags);
			break;

		    case 'e':				     /* end of output */
			print_field_hdgs(End);
			break;

		    case 'H':					  /* Hostname */
			if ( cp = devname )
			    {
			    while ( *cp ) putchar(*cp++);
			    putchar(' ');
			    }
			if ( cp = ptrtype )
			    {
			    putchar('(');
			    while ( *cp ) putchar(*cp++);
			    putchar(')');
			    putchar(' ');
			    }
			if ( cp = host )
			    {
			    putchar('@');
			    putchar(' ');
			    while ( *cp ) putchar(*cp++);
			    }
			else
			    {
			    scratch = gethostname(getname,gnlen);
			    if ( scratch == 0 && *getname )
				{
				putchar('@');
				putchar(' ');
				cp = getname;
				while ( *cp ) putchar(*cp++);
				}
			    }
			break;

		    case 'S':		      /* person who submitted the job */
			cp = from;
			while ( *cp ) putchar(*cp++);
			break;

		    case 'D':			  /* person to deliver job to */
			cp = to;
			while ( *cp ) putchar(*cp++);
			break;

		    case 'T':			    /* get title of print job */
			cp = title;
			while ( *cp ) putchar(*cp++);
			break;

		    case 'Q':			     /* time print job queued */
			cp = qdate;
			while ( *cp && *cp != '\n' ) putchar(*cp++);
			break;

		    case 'P':				  /* time job printed */
			clock = time ((long *) 0);  /* get the time */
			tdate = localtime(&clock);
			tlen = 350;
			(void) strftime(pdate,tlen,"%a %h %d %T %Y",tdate);

			cnt = strlen(pdate);
			for (i = 0; i < cnt; i++) putchar(pdate[i]);
			break;

		    case 'A':				/* command line flags */
			cp = flags;
			while ( *cp ) putchar(*cp++);
			break;

		    case '%':				 /* really want a '%' */
			putchar('%');
			break;

		    default:	       /* unknown flag argument in burst file */
			break;
		    }				  /* end of switch on % flags */
		break;

	    default:
		putchar(c);
		break;
	    }
	}
    return(0);
}

/*============================================================================*/
char *find_field_hdgs(pointer)
/*
 *  Finds the beginning of the next Header page title string ( read from the
 *  message catalog ) into the field pointed to by the 'field' pointer
 */

char *pointer;
{
    while ( *pointer != ':' ) *pointer++;
    return ++pointer;
}

/*============================================================================*/
void print_field_hdgs(pointer)
/*
 *  Prints the Header page title string pointed to by 'pointer'
 */

char *pointer;
{
    char *begin = pointer;

    while ( *begin != ':' ) putchar(*begin++);
    putchar(*begin++);
    while ( (int)((begin++)-pointer) < 20 ) putchar(' ');
}

