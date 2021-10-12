static char sccsid[] = "@(#)28	1.5  src/bos/usr/bin/uucp/uucpdefs.c, cmduucp, bos411, 9428A410j 6/17/93 14:25:55";
/* 
 * COMPONENT_NAME: CMDUUCP uucpdefs.c
 * 
 * FUNCTIONS: UerrorText
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	uucp:uucpdefs.c	1.6
*/
#include "uucp.h"
/* VERSION( uucpdefs.c	5.2 -  -  ); */

int	Ifn, Ofn;
int	Debug=0;
int	Uid, Euid;		/* user-id and effective-uid */
int	Ulimit;
ushort	Dev_mode;		/* save device mode here */
char	Progname[NAMESIZE];
char	Pchar;
char	Rmtname[MAXFULLNAME];
char	RemSpool[MAXFULLNAME];	/* spool subdirectory for remote system */
char	User[MAXFULLNAME];
char	Uucp[NAMESIZE];
char	Loginuser[NAMESIZE];
char	Myname[MAXBASENAME+1];
char	Wrkdir[MAXFULLNAME];
char	Logfile[MAXFULLNAME];
char	*Spool = SPOOL;
char	*Pubdir = PUBDIR;
char	**Env;

long	Retrytime = 0;
struct	nstat Nstat;
char	Dc[15];			/* line name				*/
int	Seqn;			/* sequence #				*/
int	Role;
char	*Bnptr;			/* used when BASENAME macro is expanded */
char	Jobid[NAMESIZE] = "";	/* Jobid of current C. file */
int	Uerror;			/* global error code */

int     (*genbrk)();

int	Verbose = 0;	/* for cu and ct only */

extern nl_catd catd;

/* used for READANY and READSOME macros */
struct stat __s_;

char *UerrorText(uerr) 
	int uerr;
{
	char *result ;

	switch (uerr) {
		case SS_OK:
			result = MSGSTR(MSG_OK, "SUCCESSFUL"); 
			break;
  		case SS_NO_DEVICE:
			result = MSGSTR(MSG_NO_DEVICE, "NO DEVICES AVAILABLE");
			break;
		case SS_TIME_WRONG:
			result = MSGSTR(MSG_TIME_WRONG, "WRONG TIME TO CALL");
			break;
		case SS_INPROGRESS:
			result = MSGSTR(MSG_INPROGRESS, "TALKING"); 
			break;
		case SS_CONVERSATION:
			result = MSGSTR(MSG_CONVERSATION,"CONVERSATION FAILED");
			break;
		case SS_SEQBAD:
			result = MSGSTR(MSG_SEQBAD, "BAD SEQUENCE CHECK");
			break;
		case SS_LOGIN_FAILED:
			result = MSGSTR(MSG_LOGIN_FAILED, "LOGIN FAILED");
			break;
  		case SS_DIAL_FAILED:
			result = MSGSTR(MSG_DIAL_FAILED, "DIAL FAILED");
			break;
		case SS_BAD_LOG_MCH:
			result = MSGSTR(MSG_BAD_LOG_MCH, 
					"BAD LOGIN/MACHINE COMBINATION");
			break;
  		case SS_LOCKED_DEVICE:
			result = MSGSTR(MSG_LOCKED_DEVICE,"DEVICE LOCKED");
			break;
  		case SS_ASSERT_ERROR:
			result = MSGSTR(MSG_ASSERT_ERROR, "ASSERT ERROR");
			break;
		case SS_BADSYSTEM: 
			result = MSGSTR(MSG_BADSYSTEM, 
					"SYSTEM NOT IN Systems FILE"); 
			break;
  		case SS_CANT_ACCESS_DEVICE:
			result = MSGSTR(MSG_CANT_ACCESS,"CAN'T ACCESS DEVICE"); 
			break;
  		case SS_DEVICE_FAILED:
			result = MSGSTR(MSG_DEVICE_FAILED,"DEVICE FAILED");
			break;
  		case SS_WRONG_MCH:
			result = MSGSTR(MSG_WRONG_MCH, "WRONG MACHINE NAME");
			break;
  		case SS_CALLBACK:
			result = MSGSTR(MSG_CALLBACK, "CALLBACK REQUIRED");
			break;
		case SS_RLOCKED:
			result = MSGSTR(MSG_RLOCKED, 
				"REMOTE HAS A LCK FILE FOR ME"); 
			break;
		case SS_RUNKNOWN:
			result = MSGSTR(MSG_RUNKNOWN,
				"REMOTE DOES NOT KNOW ME"); 
			break;
  		case SS_RLOGIN:
			result = MSGSTR(MSG_RLOGIN,
					"REMOTE REJECT AFTER LOGIN"); 
			break;
  		case SS_UNKNOWN_RESPONSE:
			result = MSGSTR(MSG_UNKNOWN_RESPONSE,
					"REMOTE REJECT, UNKNOWN MESSAGE");
			break;
		case SS_STARTUP:
			result = MSGSTR(MSG_STARTUP, "STARTUP FAILED"); 
			break;
  		case SS_CHAT_FAILED:
			result = MSGSTR(MSG_CHAT_FAILED,
					"CALLER SCRIPT FAILED"); 
			break;
 		case SS_NOSUCH_HOST:
			result = MSGSTR(MSG_NOSUCH_HOST,
					"HOSTNAME NOT RESOLVED"); 
			break;
		default:
			result = "NO SPECIFIC ERROR";
			break;
	}
	return(result);
} 
