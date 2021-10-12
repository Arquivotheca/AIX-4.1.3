/* @(#)93	1.11.1.5  src/bos/usr/include/userconf.h, cmdsauth, bos41J, 9512A_all 3/14/95 15:52:09 */
/*
 * COMPONENT_NAME: CMDSAUTH 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#ifndef _H_USERCONF    
#define _H_USERCONF

#ifdef _NO_PROTO
	extern	int	getconfattr ();
	extern  int	endconfattr ();
#else  /* _NO_PROTO */
	extern	int	getconfattr (char *, char *, void *, int);
	extern  int	endconfattr (void);
#endif /* _NO_PROTO */

/* system attribute values */
#define SC_SYS_LOGIN		"usw"
#define SC_SYS_USER		"user"
#define SC_SYS_ADMUSER		"admin"

#define SC_SYS_AUDIT		"classes"
#define SC_SYS_AUSERS		"users"
#define SC_SYS_ASYS		"system"
#define SC_SYS_ABIN		"bin"
#define SC_SYS_ASTREAM		"stream"

/* attributes associated with logins */
#define	SC_YPDOMAIN		"YPdomain"		
#define	SC_PASSWDGEN		"passwdgen"		
#define	SC_MAXBAD		"maxbad"		
#define	SC_SHELLS		"shells"		
#define	SC_GECOS		"gecos"		
#define	SC_MAXLOGINS		"maxlogins"		

/* user default attribute values */
#define	SC_UID			"uid"			
#define	SC_GROUP		"pgrp"		
#define	SC_GROUPS		"groups"		
#define	SC_ACCT			"acct"			
#define	SC_ACCTS		"accounts"		
#define	SC_AUDIT       		"auditclasses"		
#define	SC_AUTH1 		"auth1"		
#define	SC_AUTH2 		"auth2"		
#define	SC_PROG			"shell"			
#define	SC_HOME			"home"			
#define	SC_FSIZE		"fsize"		
#define	SC_CPU			"cpu"			
#define	SC_DATA			"data"			
#define	SC_STACK		"stack"		
#define	SC_CORE			"core"			
#define	SC_RSS			"rss"			
#define SC_SYSENV		"sysenv"
#define SC_USRENV		"usrenv"
#define SC_LOGINCHK		"login"
#define SC_SUCHK		"su"
#define SC_RLOGINCHK		"rlogin"
#define SC_TELNETCHK		"telnet"
#define SC_DAEMONCHK		"daemon"
#define SC_TPATH		"tpath"
#define SC_TTYS			"ttys"
#define SC_SUGROUPS		"sugroups"
#define SC_ADMGROUPS		"admgroups"
#define SC_EXPIRATION		"expires"
#define SC_AUTHPROGRAM		"program"
#define SC_AUTHRETRY		"retry"
#define SC_AUTHTIMEOUT		"timeout"
#define SC_AUTHRETRYDELAY	"retry_delay"
#define SC_LASTUPDATE		"lastupdate"
#define SC_FLAGS		"flags"
#define SC_ADMIN		"admin"
#define SC_UMASK		"umask"
#define	SC_REGISTRY		"registry"
#define SC_AUTHSYSTEM		"SYSTEM"
#define	SC_MINALPHA		"minalpha"
#define	SC_MINOTHER		"minother"
#define	SC_MINDIFF		"mindiff"
#define	SC_MAXREPEAT		"maxrepeats"		
#define	SC_MINLEN		"minlen"
#define	SC_MINAGE		"minage"		
#define	SC_MAXAGE		"maxage"		
#define	SC_MAXEXPIRED		"maxexpired"
#define SC_HISTEXPIRE           "histexpire"
#define SC_HISTSIZE             "histsize"
#define	SC_PWDCHECKS		"pwdchecks"
#define	SC_DICTION		"dictionlist"
#define	SC_PWDWARNTIME		"pwdwarntime"
#define	SC_USREXPORT		"dce_export"


#endif /* _H_USERCONF */
