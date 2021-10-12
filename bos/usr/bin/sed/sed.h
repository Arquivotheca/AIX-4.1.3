/* @(#)02	1.12.1.1  src/bos/usr/bin/sed/sed.h, cmdedit, bos41J, 9508A 2/2/95 15:09:08 */
/*
 * COMPONENT_NAME: (CMDEDIT) sed.h
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 3, 10, 18, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1984, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1975 Bell Telephone Laboratories, Incorporated
 *
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

#define _ILS_MACROS
#include	<stdlib.h>
#include 	<stdio.h>
#include 	<regex.h>

/* Include the following for prototype information */
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>


/*
 *	Define variables and macros for message catalog.
 */
#include	<locale.h>
#include 	"sed_msg.h"
extern	nl_catd	catd;
#define	MSGSTR(num,str)	catgets(catd, MS_SED, num, str)
#define USAGE_MSG(num) {\
	(void)fprintf(stderr, MSGSTR(USAGE, "Usage:\tsed [-n] Script [File ...]\n\tsed [-n] [-e Script] ... [-f Script_file] ... [File ...]\n")); \
	exit(num); \
		 }

extern char    CGMES[];
extern char    MALLOCMES[];

#define CEND    16
#define CLNUM   14

/* The maximum number of commands that you can have that specify line nums  */
#define NLINES  	1000
/* The maximum depth that sed will allow functions to be nested using {}    */
#define DEPTH   	20
/* The maximum number of commands allowed by sed.  This area of memory is   */
/* static.  Sed could dynamically allocate this area and keep a mapping for */
/* labels and functions, but this seems somewhat of an overkill.            */
#define PTRSIZE 	1000
#define ABUFSIZE        20
#define LBSIZE  	4000
/* The maximum number of labels that sed allows the user to define.         */
#define LABSIZE 	50
#define GLOBAL_SUB -1	/* global substitution */

extern union reptr     *abuf[];
extern union reptr **aptr;

/* linebuf start, end, and size */
char    *linebuf;
char    *lbend;
int     lsize;

/* holdsp start, end, and size */
char    *holdsp;
char    *hend;
int     hsize;
char    *hspend;

/* gen buf start, end, and size */
char    *genbuf;
char    *gend;
int     gsize;

long    lnum;
char    *spend;
int     nflag = 0;
long    tlno[NLINES];

/*
 *	Define command flags.
 */
#define ACOM    01
#define BCOM    020
#define CCOM    02
#define CDCOM   025
#define CNCOM   022
#define COCOM   017
#define CPCOM   023
#define DCOM    03
#define ECOM    015
#define EQCOM   013
#define FCOM    016
#define GCOM    027
#define CGCOM   030
#define HCOM    031
#define CHCOM   032
#define ICOM    04
#define LCOM    05
#define NCOM    012
#define PCOM    010
#define QCOM    011
#define RCOM    06
#define SCOM    07
#define TCOM    021
#define WCOM    014
#define CWCOM   024
#define YCOM    026
#define XCOM    033

/*
 *	Define some error conditions.
 */
#define REEMPTY     01    /* An empty regular expression */
#define NOADDR      02    /* No address field in command */
#define BADCMD      03    /* Fatal error !! */
#define MORESPACE   04    /* Need to increase size of buffer */

/*
 *	Define types an address can take.
 */
#define STRA	10	/* string */
#define REGA	20	/* regular expression */

/*
 *	Structure to hold address information.
 */
struct 	addr {
	int	afl;		/* STRA or REGA */
	union	{
		char	*str;
		regex_t *re;
	} ad;
};

/*
 *	Structure to hold sed commands.
 */
union   reptr {
	struct {
		struct addr    *ad1;
		struct addr    *ad2;
		struct addr    *re1;
		char    *rhs;
		wchar_t *ytxt;
		FILE    *fcode;
		char    command;
		short   gfl;
		char    pfl;
		char    inar;
		char    negfl;
	} r1;
	struct {
		struct addr    *ad1;
		struct addr    *ad2;
		union reptr     *lb1;
		char    *rhs;
		wchar_t *ytxt;
		FILE    *fcode;
		char    command;
		short   gfl;
		char    pfl;
		char    inar;
		char    negfl;
	} r2;
};
union reptr ptrspace[PTRSIZE];

struct label {
	char    asc[9];
	union reptr     *chain;
	union reptr     *address;
};
extern int     eargc;
extern int     exit_status;

extern union reptr     *pending;
extern char	*badp;

void 	execute(char *);
void	growbuff(int *, char **, char **, char **);

/* XPG4 - The maximum number of wfiles which can be created by sed is 10 */
#define MAXWFILES 10
