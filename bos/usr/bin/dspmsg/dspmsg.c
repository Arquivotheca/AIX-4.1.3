static char sccsid[] = "@(#)39	1.19  src/bos/usr/bin/dspmsg/dspmsg.c, cmdmsg, bos411, 9428A410j 4/2/94 10:26:34";
/*
 * COMPONENT_NAME: (CMDMSG) Message Catalogue Facilities
 *
 * FUNCTIONS: main, pars_args
 *
 * ORIGINS: 18, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 * 
 * RCSfile: dspmsg.c,v Revision: 1.4  (OSF) Date: 90/10/07 16:45:19 
 */


/*                                                                   
 * EXTERNAL PROCEDURES CALLED: standard library functions
 */

#define _ILS_MACROS

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <errno.h>
#include <ctype.h>
#include "catio.h"
#include "msgfac_msg.h"

#define isanumber(c) (c >= '0' && c <= '9') ? 1 : 0
#define isaoctal(c) (c >= '0' && c <= '7') ? 1 : 0
#define toanumber(c) (c - '0')
#define NOT_SET -1
#define TRUE 	1
#define FALSE   0

struct arguments {
	int	set,
		msg,
		argmax;
	char	*catname,
		*def,
		**args;
};

	/*-- subroutine used to parse the input arguments ---*/
void parse_args(int argc, char *argv[], struct arguments *args);

nl_catd	catderr;	/* error message catalog descriptor */



/*
 * NAME: main
 *                                                                    
 * FUNCTION: 	Extract a message string from a catalog. Perform printf
 * 		style substitutions and print it out.
 * 
 * EXECUTION ENVIRONMENT:
 *  	User mode.
 *	                                                                    
 * RETURNS: 	Exit with 0, except when: the format string 
 *		is invalid.
 */  

int main(int argc,char *argv[]) 

	/* argc: Number of arguments */
	/* argv: argument vector */

{
	struct arguments   args;   /* place to store the parsed arguments*/
	nl_catd	catdmsg;           /* catalog descriptor for message catalog */
	char 	*message;	   /* place to store message */
	char 	*p;		   /* pointer to current pos within message */
	int 	idx, 		   /* current argument to be printed */
		reorder = NOT_SET; /* Reordering  (TRUE, FALSE, NOT_SET) */
	int	n;		   /* # bytes in a character */

	setlocale (LC_ALL,"");
	catderr = catopen(MF_MSGFAC, NL_CAT_LOCALE);
	if (argc < 3) {
		die(catgets(catderr,MS_DSPMSG,M_DSPMSG, "Usage: dspmsg [-s setno] <catname> <msgno> ['default' arg ... ]"));
	}

/*______________________________________________________________________
	Parse the input arguments int the args structure.
  ______________________________________________________________________*/

	parse_args(argc,argv,&args);

/*______________________________________________________________________
	get the message out of the catalog.
  ______________________________________________________________________*/

	catdmsg = catopen(args.catname, NL_CAT_LOCALE);
	message = catgets(catdmsg,args.set,args.msg,args.def);

        if (message == 0)
		die(catgets(catderr,MS_DSPMSG,M_NODEFAULT,"dspmsg: missing default message"));

/*______________________________________________________________________

	print out the message making the appropriate sub's for
	the parameters.  Reorder the parameters if necessary.
	Do not use mixed reordering!!!
  ______________________________________________________________________*/

	for (p = message , idx = 0 ; *p ; p++ )  {

		/* quoted escape characters */
		if (*p == '\\' && message == args.def) {
			switch (*++p) {
				case 'n':
					putc('\n',stdout);
					break;

				case 't':
					putc('\t',stdout);
					break;

				case 'b':
					putc('\b',stdout);
					break;

				case 'r':
					putc('\r',stdout);
					break;

				case 'v':
					putc('\v',stdout);
					break;

				case 'f':
					putc('\f',stdout);
					break;

				case 'x':
					{
					char *pesc = p+1;
					int hex, hexlen = 0;

					while (isxdigit(*pesc))
						hexlen++,pesc++;
					if (hexlen == 2)
						sscanf (p+1, "%2x", &hex);
					else if (hexlen == 4)
						sscanf (p+1, "%4x", &hex);
					else {
						putc('x',stdout);
						break;
					}
					putc(hex,stdout);
					p += hexlen;
					break;
					}

				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					{
					int c = 0;
					char *pesc = p;

					do
						c = c * 8 + *pesc++ - '0';
					while (isaoctal(*pesc) && pesc < p+3);
					if (c <= 0377) {
						putc(c,stdout);
						p = pesc - 1;
					} else
						putc(*p,stdout);
					break;
					}

				default: 
					putc(*p,stdout);
					break;
			}
		}

                /* printf % style substitution */
		else if (*p == '%') {

			/* %% prints one % */
			if (*++p == '%')  {
				putc(*p,stdout);
				continue;
			}

			/* %n$ reorders the argument list and uses variable n next */
			/* once this is used, all arguments must use it */
			if (isanumber(*p)) {

				/* do not allow mixing of reorder types */
				if (reorder == FALSE) {
					die(catgets(catderr,MS_DSPMSG,M_REORDER,"\ndspmsg: Either none or all arguments in message must use %n$ format"));
				}
				for (idx = 0 ; isanumber(*p) ; p++)
					idx += idx * 10 + toanumber(*p);
				idx--;
				if (*p++ != '$') {
					die(catgets(catderr,MS_DSPMSG,M_INVRE,"\ndspmsg: $ missing from %n$ format in message"));
				}
				reorder = TRUE;
			}
			else {
				/* do not allow mixing of reorder types */
				if (reorder == TRUE) {
					die(catgets(catderr,MS_DSPMSG,M_REORDER,"\ndspmsg: Either none or all arguments in message must use %n$ format"));
				}
				reorder = FALSE;	
			}
			/* report invalid printf argument number */
			if (idx < 0 || idx >= args.argmax) {
				die(catgets(catderr,MS_DSPMSG,M_REINDEX,"\ndspmsg: Invalid argument index in message.  May need more\narguments on the command line."));
			}
			/* report unsupported % type */
			if (*p == 's')
				;
			else if (*p == 'l' && p[1] == 'd')
				p++;
			else {
				die(catgets(catderr,MS_DSPMSG,M_NOSUPPORT,"\ndspmsg: Invalid conversion specifier, must use %s or %ld."));
			}
			fwrite(args.args[idx],strlen(args.args[idx]),1,stdout);
			idx++;				
		}

		/* just print the next character */
		else {
			n = mblen(p, MB_CUR_MAX);
			if (n < 0)
				n = 1;
			do
				putc(*p++,stdout);
			while (--n > 0);
			p--;
		}
	}
	exit(0);
}



/*
 * NAME: parse_args
 *
 * FUNCTION: Sets up the args-> data structure for main().
 *
 * EXECUTION ENVIRONMENT:
 *  	User mode.
 * 
 * RETURNS: void
 */

void parse_args(int argc, char *argv[], struct arguments *args) 

	/* argc: The number or arguments */
	/* argv: The input argument vector */
	/* args: The output argument structure */

{
	char * s[1];

	args->args = NULL;
	args->def = 0;
	args->argmax = 0;
	args->set = 1;
	argv++ ; 
	argc--;				/* Skip the program name */
	if (!strcmp(*argv,"-s")) {	/* check for a set number */
		if (argc < 4) 		/* check for sufficient arguements */
			die(catgets(catderr,MS_DSPMSG,M_DSPMSG, "Usage: dspmsg [-s setno] <catname> <msgno> ['default' arg ... ]"));
		argv++; 
		argc--;				/* skip past the '-s' */
		errno=0;
		args->set=strtol(*argv,s,10);	/* get the real set number */
		if ((errno!=0) || (*s[0]!='\0') || (*argv[0]=='\0'))
			die(catgets(catderr,MS_DSPMSG,M_DSPMSG, "Usage: dspmsg [-s setno] <catname> <msgno> ['default' arg ... ]"));
		argv++; 
		argc--;				/* skip past the set number */
	}
	args->catname = *argv++;		/* get the cat name */
	argc--;
	if (!strcmp(*argv,"-s")) {		/* check for a set number */
		if (argc < 3) 		/* check for sufficient arguements */
			die(catgets(catderr,MS_DSPMSG,M_DSPMSG, "Usage: dspmsg [-s setno] <catname> <msgno> ['default' arg ... ]"));

		argv++; 
		argc--;				/* skip past the '-s' */
		errno=0;
		args->set=strtol(*argv,s,10);	/* get the real set number */
		if ((errno!=0) || (*s[0]!='\0') || (*argv[0]=='\0'))
			die(catgets(catderr,MS_DSPMSG,M_DSPMSG, "Usage: dspmsg [-s setno] <catname> <msgno> ['default' arg ... ]"));
		argv++; 
		argc--;				/* skip past the set number */
	}
	errno=0;
	args->msg=strtol(*argv,s,10);		/* scan the message number */
	if ((errno!=0) || (*s[0]!='\0') || (*argv[0]=='\0'))
		die(catgets(catderr,MS_DSPMSG,M_DSPMSG, "Usage: dspmsg [-s setno] <catname> <msgno> ['default' arg ... ]"));
	argv++;
	argc--;
	if (argc) {				/* check for the arg count 
       						   for a default string */
		args->def= *argv++;
		argc--;
	}
	if (argc)  {
		args->args = argv;
		args->argmax = argc;
	}
}
