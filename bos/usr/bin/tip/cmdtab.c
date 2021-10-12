static char sccsid[] = "@(#)35	1.4  src/bos/usr/bin/tip/cmdtab.c, cmdtip, bos411, 9428A410j 4/10/91 09:06:15";
/* 
 * COMPONENT_NAME: UUCP cmdtab.c
 * 
 * FUNCTIONS: MSGSTR 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "cmdtab.c	5.3 (Berkeley) 5/5/86"; */

#include "tip.h"
#include <nl_types.h>
#include "tip_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

extern	int shell(), getfl(), sendfile(), chdirectory();
extern	int finish(), help(), pipefile(), pipeout(), consh(), variable();
extern	int cu_take(), cu_put(), dollar(), genbrk(), suspend();

esctable_t etable[] = {
	{ '!',	NORM,	"shell",			 shell }, 
	{ '<',	NORM,	"receive file from remote host", getfl }, 
	{ '>',	NORM,	"send file to remote host",	 sendfile }, 
	{ 't',	NORM,	"take file from remote UNIX",	 cu_take }, 
	{ 'p',	NORM,	"put file to remote UNIX",	 cu_put }, 
	{ '|',	NORM,	"pipe remote file",		 pipefile }, 
	{ '$',	NORM,	"pipe local command to remote host", pipeout }, 
#ifdef CONNECT
	{ 'C',  NORM,	"connect program to remote host",consh }, 
#endif
	{ 'c',	NORM,	"change directory",		 chdirectory }, 
	{ '.',	NORM,	"exit from tip",		 finish }, 
	{CTRL(d),NORM,	"exit from tip",		 finish }, 
	{CTRL(y),NORM,	"suspend tip (local+remote)",	 suspend }, 
	{CTRL(z),NORM,	"suspend tip (local only)",	 suspend }, 
	{ 's',	NORM,	"set variable",			 variable }, 
	{ '?',	NORM,	"get this summary",		 help }, 
	{ '#',	NORM,	"send break",			 genbrk }, 
	{ 0, 0, 0 }
};
