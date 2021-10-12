static char sccsid[] = "@(#)24 1.3  src/bos/usr/bin/learn/mem.c, cmdlearn, bos411, 9428A410j 3/22/93 13:23:33";
/*
 * COMPONENT_NAME: (CMDLEARN) provides computer-aided instruction courses
 *
 * FUNCTIONS: action, setdid, unsetdid, already, tellwhich, load_keybuff
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
*/

# include "stdio.h"
# include "lrnref.h"

#include "learn_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s)

# define SAME 0

#define KWDLEN 30  /* length of the k_wd field in the keys struct below */
static int keybuff_loaded = 1;  /* have we loaded the array yet? */
#define load_array(string) strncpy(keybuff[keybuff_loaded++].k_wd, string, KWDLEN-1)

struct keys {
	char k_wd[KWDLEN];
	int k_val;
} keybuff[] = {
	{"ready",	READY},
	{"answer",	READY},
	{"#print",	PRINT},
	{"#copyin",	COPYIN},
	{"#uncopyin",	UNCOPIN},
	{"#copyout",	COPYOUT},
	{"#uncopyout",	UNCOPOUT},
	{"#pipe",	PIPE},
	{"#unpipe",	UNPIPE},
	{"#succeed",	SUCCEED},
	{"#fail",	FAIL},
	{"bye",		BYE},
	{"chdir",	CHDIR},
	{"cd",		CHDIR},
	{"learn",	LEARN},
	{"#log",	LOG},
	{"yes",		YES},
	{"no",		NO},
	{"again",	AGAIN},
	{"#mv",		MV},
	{"#user",	USER},
	{"#next",	NEXT},
	{"skip",	SKIP},
	{"where",	WHERE},
	{"#match",	MATCH},
	{"#bad",	BAD},
	{"#create",	CREATE},
	{"#cmp",	CMP},
	{"hint",	HINT},
	{"#once",	ONCE},
	{"#",		NOP},
	{NULL,		0}
};

int *action(s)
char *s;
{
	struct keys *kp;
	if (!keybuff_loaded) /* load array with national language? */
		load_keybuff();
	for (kp=keybuff; *(kp->k_wd); kp++)
		if (STRCMP(kp->k_wd, s) == SAME)
			return(&(kp->k_val));
	return(NULL);
}

# define NW 100
# define NWCH 800
struct whichdid {
	char *w_less;
	int w_seq;
} which[NW];
int nwh = 0;
char whbuff[NWCH];
char *whcp = whbuff;
static struct whichdid *pw;

setdid(lesson, sequence)
char *lesson;
int sequence;
{
	if (already(lesson)) {
		pw->w_seq = sequence;
		return;
	}
	pw = which+nwh++;
	if (nwh >= NW) {
		fprintf(stderr, MSGSTR(LTOOMNYLESS, "Setdid:  too many lessons\n"));
		tellwhich();
		wrapup(1);
	}
	pw->w_seq = sequence;
	pw->w_less = whcp;
	while (*whcp++ = *lesson++);
	if (whcp >= whbuff + NWCH) {
		fprintf(stderr, MSGSTR(LLESSNMTOOLNG, "Setdid:  lesson names too long\n"));
		tellwhich();
		wrapup(1);
	}
}

unsetdid(lesson)
char *lesson;
{
	if (!already(lesson))
		return;
	nwh = pw - which;	/* pretend the rest have not been done */
	whcp = pw->w_less;
}

already(lesson)
char *lesson;
{
	for (pw=which; pw < which+nwh; pw++)
		if (STRCMP(pw->w_less, lesson) == SAME)
			return(1);
	return(0);
}

tellwhich()
{
	for (pw=which; pw < which+nwh; pw++)
		PRINTF(MSGSTR(LLESSSEQ, "%3d lesson %7s sequence %3d\n"),
			pw-which, pw->w_less, pw->w_seq);
}

load_keybuff() 
{
	load_array(MSGSTR(LREADY, "ready"));
	load_array(MSGSTR(LANSWER, "answer"));
	load_array(MSGSTR(LPPRINT, "#print"));
	load_array(MSGSTR(LPCOPYIN, "#copyin"));
	load_array(MSGSTR(LPUNCOPYIN, "#uncopyin"));
	load_array(MSGSTR(LPCOPYOUT, "#copyout"));
	load_array(MSGSTR(LPUNCOPYOUT, "#uncopyout"));
	load_array(MSGSTR(LPPIPE, "#pipe"));
	load_array(MSGSTR(LPPUNPIPE, "#unpipe"));
	load_array(MSGSTR(LPPSUCCEED, "#succeed"));
	load_array(MSGSTR(LPFAIL, "#fail"));
	load_array(MSGSTR(LBYE, "bye"));
	load_array(MSGSTR(LCHDIR, "chdir"));
	load_array(MSGSTR(LCD, "cd"));
	load_array(MSGSTR(LLEARN, "learn"));
	load_array(MSGSTR(LPLOG, "#log"));
	load_array(MSGSTR(LYES, "yes"));
	load_array(MSGSTR(LNO, "no"));
	load_array(MSGSTR(LAGAIN, "again"));
	load_array(MSGSTR(LPMV, "#mv"));
	load_array(MSGSTR(LPUSER, "#user"));
	load_array(MSGSTR(LPNEXT, "#next"));
	load_array(MSGSTR(LSKIP, "skip"));
	load_array(MSGSTR(LWHERE, "where"));
	load_array(MSGSTR(LPMATCH, "#match"));
	load_array(MSGSTR(LPBAD, "#bad"));
	load_array(MSGSTR(LPCREATE, "#create"));
	load_array(MSGSTR(LPCMP, "#cmp"));
	load_array(MSGSTR(LHINT, "hint"));
	load_array(MSGSTR(LPONCE, "#once"));
}
