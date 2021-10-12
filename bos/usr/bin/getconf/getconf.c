static char sccsid[] = "@(#)78  1.11  src/bos/usr/bin/getconf/getconf.c, cmdposix, bos41J, 9522A_all 5/30/95 10:15:39";
/*
 * COMPONENT_NAME: (CMDPOSIX) new commands required by Posix 1003.2
 *
 * FUNCTIONS: getconf
 *
 * ORIGINS: 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 *
 */

#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#include "getconf_msg.h"

static nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_GETCONF,num,str)  /*MSG*/

/*
 *
 *  NOTE:  This table must remain sorted 
 *
 */
#define CONSTANT	1
#define SYSCONF		2 
#define PATHCONF	3 
#define CONFSTR		4 
#define UNDEFINED       5
#define UCONSTANT       6

static struct values{
	char *name;
	int kind;
	int numb;
} value[] = 
	{
	"ARG_MAX",		CONSTANT,	ARG_MAX,	
	"BC_BASE_MAX",		SYSCONF,	_SC_BC_BASE_MAX,
	"BC_DIM_MAX",		SYSCONF,	_SC_BC_DIM_MAX,
	"BC_SCALE_MAX",		SYSCONF,	_SC_BC_SCALE_MAX,
	"BC_STRING_MAX", 	SYSCONF,	_SC_BC_STRING_MAX,
	"CHARCLASS_NAME_MAX", 	CONSTANT,	CHARCLASS_NAME_MAX,
	"CHAR_BIT",		CONSTANT,	CHAR_BIT,
	"CHAR_MAX",		CONSTANT,	CHAR_MAX,	
	"CHAR_MIN",		CONSTANT,	CHAR_MIN,
	"CHILD_MAX",		CONSTANT, 	CHILD_MAX,	
	"CLK_TCK",		CONSTANT,	CLK_TCK,
	"COLL_WEIGHTS_MAX",	CONSTANT,	COLL_WEIGHTS_MAX,
	"CS_PATH",		CONFSTR,	_CS_PATH,
	"EXPR_NEST_MAX",	SYSCONF,	_SC_EXPR_NEST_MAX,
	"INT_MAX",		CONSTANT,	INT_MAX,
	"INT_MIN",		CONSTANT,	INT_MIN,
	"LINE_MAX",		SYSCONF,	_SC_LINE_MAX,
	"LINK_MAX",		PATHCONF,	_PC_LINK_MAX,
	"LONG_BIT",		CONSTANT,	LONG_BIT,
	"LONG_MAX",		CONSTANT,	LONG_MAX,
	"LONG_MIN",		CONSTANT,	LONG_MIN,
	"MAX_CANON",		PATHCONF,	_PC_MAX_CANON,
	"MAX_INPUT",		PATHCONF,	_PC_MAX_INPUT,
	"MB_LEN_MAX",		CONSTANT,	MB_LEN_MAX,
	"NAME_MAX",		PATHCONF,	_PC_NAME_MAX,
	"NGROUPS_MAX",		CONSTANT,	NGROUPS_MAX,
	"NL_ARGMAX",		CONSTANT,	NL_ARGMAX,
	"NL_LANGMAX",		CONSTANT,	NL_LANGMAX,
	"NL_MAX",		CONSTANT,	NL_NMAX, 
	"NL_MSGMAX",		CONSTANT,	NL_MSGMAX,
	"NL_NMAX",		CONSTANT,	NL_NMAX, 
	"NL_SETMAX",		CONSTANT,	NL_SETMAX,
	"NL_TEXTMAX",		CONSTANT,	NL_TEXTMAX,
	"NZERO",		CONSTANT,	NZERO,
	"OPEN_MAX",		CONSTANT,	OPEN_MAX,
	"PATH",		  	CONFSTR,	_CS_PATH,
	"PATH_MAX",		PATHCONF,	_PC_PATH_MAX,
	"PIPE_BUF",		PATHCONF,	_PC_PIPE_BUF,
	"POSIX2_BC_BASE_MAX", 	CONSTANT,	_POSIX2_BC_BASE_MAX,	
	"POSIX2_BC_DIM_MAX", 	CONSTANT,	_POSIX2_BC_DIM_MAX,	
	"POSIX2_BC_SCALE_MAX",	CONSTANT,	_POSIX2_BC_SCALE_MAX,
	"POSIX2_BC_STRING_MAX", CONSTANT,	_POSIX2_BC_STRING_MAX,
	"POSIX2_CHAR_TERM",	CONSTANT,	_POSIX2_CHAR_TERM,
	"POSIX2_COLL_WEIGHTS_MAX", CONSTANT,	_POSIX2_COLL_WEIGHTS_MAX,
	"POSIX2_C_BIND",	CONSTANT,	_POSIX2_C_BIND,
	"POSIX2_C_DEV",		CONSTANT,	_POSIX2_C_DEV,
	"POSIX2_C_VERSION",	CONSTANT,	_POSIX2_C_VERSION,
	"POSIX2_EXPR_NEST_MAX",	CONSTANT,	_POSIX2_EXPR_NEST_MAX,
	"POSIX2_FORT_DEV",	CONSTANT,	_POSIX2_FORT_DEV,
	"POSIX2_FORT_RUN",	CONSTANT,	_POSIX2_FORT_RUN,
	"POSIX2_LINE_MAX",	CONSTANT,	_POSIX2_LINE_MAX,
	"POSIX2_LOCALEDEF",	CONSTANT,	_POSIX2_LOCALEDEF,
	"POSIX2_RE_DUP_MAX",	CONSTANT,	_POSIX2_RE_DUP_MAX,
	"POSIX2_SW_DEV",	CONSTANT,	_POSIX2_SW_DEV,
	"POSIX2_UPE",		CONSTANT,	_POSIX2_UPE,
	"POSIX2_VERSION",	CONSTANT,	_POSIX2_VERSION,
	"RE_DUP_MAX",		CONSTANT,	RE_DUP_MAX,
	"SCHAR_MAX",		CONSTANT,	SCHAR_MAX,
	"SCHAR_MIN",		CONSTANT,	SCHAR_MIN,
	"SHRT_MAX",		CONSTANT,	SHRT_MAX,
	"SHRT_MIN",		CONSTANT,	SHRT_MIN,
	"SSIZE_MAX",		CONSTANT,	SSIZE_MAX,
	"STREAM_MAX",		CONSTANT,	STREAM_MAX,
	"TMP_MAX",		CONSTANT,	TMP_MAX,
	"TZNAME_MAX",		CONSTANT,	TZNAME_MAX,
	"UCHAR_MAX",		CONSTANT,	UCHAR_MAX,
	"UINT_MAX",		UCONSTANT,	UINT_MAX,
	"ULONG_MAX",		UCONSTANT,	ULONG_MAX,
	"USHRT_MAX",		CONSTANT,	USHRT_MAX,
	"WORD_BIT",		CONSTANT,	WORD_BIT,
	"_CS_PATH",		CONFSTR,	_CS_PATH,
	"_POSIX_ARG_MAX",	CONSTANT,	_POSIX_ARG_MAX,
	"_POSIX_CHILD_MAX",	CONSTANT,	_POSIX_CHILD_MAX,
	"_POSIX_CHOWN_RESTRICTED", PATHCONF,	_PC_CHOWN_RESTRICTED,
	"_POSIX_JOB_CONTROL",	CONSTANT,	_POSIX_JOB_CONTROL,
	"_POSIX_LINK_MAX",	CONSTANT,	_POSIX_LINK_MAX,
	"_POSIX_MAX_CANON",	CONSTANT,	_POSIX_MAX_CANON,
	"_POSIX_MAX_INPUT",	CONSTANT,	_POSIX_MAX_INPUT,
	"_POSIX_NAME_MAX",	CONSTANT,	_POSIX_NAME_MAX,
	"_POSIX_NGROUPS_MAX",	CONSTANT,	_POSIX_NGROUPS_MAX,
	"_POSIX_NO_TRUNC",	PATHCONF,	_PC_NO_TRUNC,
	"_POSIX_OPEN_MAX",	CONSTANT,	_POSIX_OPEN_MAX,
	"_POSIX_PATH_MAX",	CONSTANT,	_POSIX_PATH_MAX,
	"_POSIX_PIPE_BUF",	CONSTANT,	_POSIX_PIPE_BUF,
	"_POSIX_SAVED_IDS",	CONSTANT,	_POSIX_SAVED_IDS,
	"_POSIX_SSIZE_MAX",	CONSTANT,	_POSIX_SSIZE_MAX,
	"_POSIX_STREAM_MAX",	CONSTANT,	_POSIX_STREAM_MAX,
	"_POSIX_TZNAME_MAX",	CONSTANT,	_POSIX_TZNAME_MAX,
	"_POSIX_VDISABLE",	PATHCONF,	_PC_VDISABLE,
	"_POSIX_VERSION",	CONSTANT,	_POSIX_VERSION,
	"_XOPEN_CRYPT",		CONSTANT,	_XOPEN_CRYPT,
	"_XOPEN_ENH_I18N",	CONSTANT,	_XOPEN_ENH_I18N,
	"_XOPEN_SHM",		CONSTANT,	_XOPEN_SHM,
	"_XOPEN_VERSION",	CONSTANT,	_XOPEN_VERSION,
	"_XOPEN_XCU_VERSION",	CONSTANT,	_XOPEN_XCU_VERSION,
#ifndef _XOPEN_XPG2
	"_XOPEN_XPG2",		UNDEFINED,	0,
#else
	"_XOPEN_XPG2",		CONSTANT,	_XOPEN_XPG2,
#endif
	"_XOPEN_XPG3",		CONSTANT,	_XOPEN_XPG3,
	"_XOPEN_XPG4",		CONSTANT,	_XOPEN_XPG4,
	(char *)0,		  0,
	};

main(int argc, char **argv)
{
	int i;		/* index */
	size_t size;    /* size of buffer necessary for confstr */
	char *buf;      /* buffer necessary for confstr */
	int ret;	/* return value */

	(void) setlocale(LC_ALL,"");

	catd = catopen(MF_GETCONF, NL_CAT_LOCALE);

	errno = 0;

	while((i = getopt(argc, argv, "")) != EOF)
		switch(i) {

		case '?':
			usage();
			break;
		}

	argc -= optind;
	argv += optind;

       /*  
	*  Getconf only expects the following two formats:
	*
	*      getconf system_var
	*      getconf path_var pathname
	*
	*  Give usage and exit if not correct format.
	*/

	if ((argc < 1) || (argc > 2))
		usage();

       /*
	* The first argument is either the system or path variable
	* to be queried.  Convert to the correct index via the table.
	*/

    	for (i = 0; i < sizeof(value)/sizeof(value[0]) - 1; i++) {
		ret = strcmp(argv[0], value[i].name);

	       /*
		*  If match is found, break the loop and call correct
		*  interpreting routine.
		*/
		if (ret == 0)
			break;

	       /*
		*  If a string bigger than the argument is found in the
		*  table (which is sorted), then we know that it is an
		*  invalid argument.  Exit with error status
		*/
		if (ret < 0)   {
			fprintf(stderr, MSGSTR(NOTVAL, "getconf: specified variable is not valid on this system\n"));
			exit(2);
		}
	}	/* end of for */

	/* If did not find argument in table, exit with error status */
	if (value[i].name == (char *)0) {
			fprintf(stderr, MSGSTR(NOTVAL, "getconf: specified variable is not valid on this system\n"));
			exit(2);
		}

       /*
	*  When there is only one argument to getconf, it is assumed
	*  to be a system variable.  If the variable is valid and is
	*  defined on the system, print the associated value.  If it is 
	*  valid, but not defined, print "undefined"; otherwise exit
	*  with 2.
	*/

	if (argc == 1) { 

		switch (value[i].kind) {
			case CONSTANT:
				printf("%d\n", value[i].numb);
				exit(0);
				break;

			case UCONSTANT:
				printf("%u\n", value[i].numb);
				exit(0);
				break;

			case SYSCONF:
				ret = sysconf(value[i].numb);
				if (ret != -1) {
					printf("%d\n", ret);
					exit(0);
				}
				else if (errno == 0) {
					printf(MSGSTR(NOTDEF, "undefined\n"));
					exit(0);
				}
				/* else, specified variable is not valid on
				   system, but the program should have reported
				   the error earlier by searching the table */ 
				break;

			case CONFSTR:
				size = confstr(value[i].numb, (char *)0, 0);
				if (size == 0) {  /* Argument is not defined */
					if (errno == 0) {
						printf(MSGSTR(NOTDEF, "undefined\n"));
						exit(0);
					}
					/* else, specified variable is not
					   valid on system, but the program
		 			   should have reported the error
					   earlier by searching the table */ 
				}
				if ((buf = malloc(size)) == NULL) 
					perror("getconf");
				confstr(value[i].numb, buf, size);
				printf("%s\n", buf);
				exit(0);
				break;

			case UNDEFINED:
				printf(MSGSTR(NOTDEF, "undefined\n"));
				exit(0);

			default:       /* specified path_var without pathname */
				fprintf(stderr, MSGSTR(NOPATH, "getconf: specify pathname with path_var\n"));
				exit(2);
		}	/* switch */
	}

       /*
	*  When there are two argument to getconf, it is assumed
	*  to be a path variable.  If the variable is valid and is
	*  defined on the system, print the associated value.  If it is 
	*  valid, but not defined, print "undefined", else exit with 2.
	*/

	else if (value[i].kind == PATHCONF) { 
		ret = pathconf(argv[1], value[i].numb);
		if (ret != -1) {
			printf("%d\n", ret);
			exit(0);
		}
		else if (errno == 0) {
			printf(MSGSTR(NOTDEF, "undefined\n"));
			exit(0);
                }
		else {
			perror("getconf");
			exit(2);
		}
	}
	else {			/* specified system_var with pathname */
		fprintf(stderr, MSGSTR(USAGE1, "Usage: getconf system_var\n"));
		exit(2);	
	}
}
	
static usage(void)
{
	fprintf(stderr, MSGSTR(USAGE1, "Usage: getconf system_var\n"));
	fprintf(stderr, MSGSTR(USAGE2, "       getconf path_var pathname\n"));
	exit(2);
}
