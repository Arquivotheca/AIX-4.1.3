static char sccsid[] = "@(#)59	1.18  src/bos/usr/bin/mesg/mesg.c, cmdwrite, bos411, 9428A410j 12/1/93 09:39:32";
/*
 * COMPONENT_NAME: (CMDWRITE) user to user communication
 *
 * FUNCTIONS: mesg
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 */
/*
 *                                                                    
 * mesg -- set current tty to accept or
 *	forbid write permission.
 *
 *	mesg [-y] [-n]
 *		y allow messages
 *		n forbid messages
 *	return codes
 *		0 if messages are ON or turned ON
 *		1 if messages are OFF or turned OFF
 *		2 if usage error
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/access.h>
#include <IN/standard.h>

#include <nl_types.h>
#include "mesg_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_MESG,Num,Str)

struct stat sbuf;

char *tty;

char *progname;

main(argc, argv)
char *argv[];
{
	int i, c, r=0, errflag=0;
	extern int optind;
	char *ystr = "y";
	char *nstr = "n";
	char errmsg[MAXLINE];

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_MESG, NL_CAT_LOCALE);
	progname = argv[0];

	for(i = 0; i <= 2; i++) {
		if ((tty = ttyname(i)) != NULL)
			break;
	}
	if (tty == NULL)
		ERcmderr(254, MSGSTR(NOTERM,"Cannot find terminal.")); /*MSG*/

	if (stat(tty, &sbuf) < 0) {
		sprintf(errmsg, MSGSTR(NOSTATUS,"Cannot get status of \"%s\"."), tty); /*MSG*/
		ERcmderr(254,errmsg); 
	}

	if (argc < 2) {
		if ( accessx(tty,W_ACC,ACC_ALL) == 0 )
			printf(MSGSTR(IS,"is %s\n"), ystr); /*MSG*/
		else  {
			r = 1;
			printf(MSGSTR(IS,"is %s\n"), nstr); /*MSG*/
		}
	}

	/* search the args that begin with '-' for a 'y' or 'n' */
	for(i=1; i<argc && argv[i][0] == '-';++i) {
		if(!strcmp(argv[i]+1,ystr)) 
			newmode(ACC_PERMIT, W_ACC, ACC_ALL);
		else if(!strcmp(argv[i]+1,nstr)) 
		{	/* 600 */
			newmode(ACC_PERMIT, R_ACC|W_ACC, ACC_OBJ_OWNER);
			newmode(ACC_SPECIFY, NO_ACC, ACC_OTHERS);
			r = 1;
		} 
		else if (!strcmp(argv[i]+1,"-"))
			continue;
		else 
			errflag++;
	}

	if (errflag) {
		fprintf(stderr, MSGSTR(USAGE,"usage: mesg [y|n]\n")); /*MSG*/
		exit(2);
	}

/* This allows for "mesg y" or "mesg n" to be entered from command line. */
/* added for temporary compat. */
	if(argc > i) {
		if(!strcmp(argv[i],ystr)) 
			newmode(ACC_PERMIT, W_ACC, ACC_ALL);
		else if(!strcmp(argv[i],nstr)) 
		{	/* 600 */
			newmode(ACC_PERMIT, R_ACC|W_ACC, ACC_OBJ_OWNER);
			newmode(ACC_SPECIFY, NO_ACC, ACC_OTHERS);
			r = 1;
		}
		else 
			errflag++;
	}

	if (errflag) {
		fprintf(stderr, MSGSTR(USAGE,"usage: mesg [y|n]\n")); /*MSG*/
		exit(2);
	}
/* added to here */
	exit(r);
}


/* 
 * NAME: newmode
 * FUNCTION: change permission mode on tty
 */

/* AIX security enhancement */
/* newmode() uses acl_chg() instead of chmod() */
/* replaced:	if (chmod(tty, (sbuf.st_mode & S_ISVTX) | m) < 0) {	*/
newmode(how,mode,who)
int	how;
int	mode;
int	who;
{
	char errmsg[MAXLINE];

	if ( acl_chg(tty, how, mode, who) < 0 ) {
		sprintf(errmsg, MSGSTR(CANTCHG,"Cannot change mode of \"%s\"."), tty); /*MSG*/
		ERcmderr(254,errmsg); 
	}
}

/*
 * NAME: ERcmderr
 * FUNCTION: display error message and exit
 */
ERcmderr(code,str)
int code;
char *str;
{
	fprintf(stderr,"%s\n",str);
	exit(code);
}
