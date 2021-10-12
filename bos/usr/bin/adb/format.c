static char sccsid[] = "@(#)%M  1.9.1.1  src/bos/usr/bin/adb/format.c, cmdadb, bos411, 9428A410j  10/6/93  15:12:27";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTIONS: UNCTRL, exform, inkdot, is_cntrl, is_print, printesc, rdfp,
 * 	      scanform, shell
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*              include file for message texts          */
#include "adb_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include "defs.h"
#ifdef _NOPROTO
LOCAL void printesc();
#else
LOCAL void printesc(int);
#endif /* _NO_PROTO */

extern int filndx;
#ifdef BELL
LOCAL STRING fphack;

LOCAL char rdfp()
{
    return (lastc = *fphack++);
}
#endif

void scanform(icount, ifp, itype, ptype)
int    icount;
STRING ifp;
int    itype;
int    ptype;
{
    STRING fp;
    int    fcount;
#ifdef BELL
    BOOL   init = TRUE;
#else
    BOOL   init = FALSE;
#endif
    long   savdot;

    while (icount) {
	if (!init && maxoff != 0 && findsym(dot, ptype) == 0) {
	   if ( filndx != -1 )
	    adbpr("\n%s:%16t", flexstr(&symbol, filndx));
	}

	fp = ifp;
	savdot = dot;
	init = FALSE;

	/*now loop over format*/
	while (*fp && errflg == NULL) {
#ifdef BELL
	    fphack = fp;
	    (void)rdfp();
	    if (getnum(rdfp) && lastc == '*') {
		fcount = expv;
		fp = fphack;
		peekc = 0;
	    } else
		fcount = 1;
#else
	    if (*fp >= '0' && *fp <= '9') {
		fcount = 0;
		while (*fp >= '0' && *fp <= '9') {
		    fcount = fcount * 10 + *fp++ - '0';
		}
	    } else {
		fcount = 1;
	    }
#endif

	    if (*fp == 0)
		break;
	    fp = exform(fcount, fp, itype, ptype);
	}
	dotinc = dot - savdot;
	dot = savdot;

	if (errflg != NULL) {
	    if (icount < 0) {
		errflg = NULL;
		break;
	    } else
		error(errflg);
	}
	if (--icount)
	    dot = inkdot(dotinc);
	if (mkfault)
	    error((STRING) NULL);
    }
}

/*
 * execute single format item `fcount' times
 * sets `dotinc' and moves `dot'
 * returns address of next format item
 */
STRING exform(fcount, ifp, itype, ptype)
int    fcount;
STRING ifp;
int    itype;
int    ptype;
{
    long      savdot;
    STRING   fp;
    char     c;
    char     modifier = '\0';
    long     w;
#if FLOAT
    struct {
	int   sa;
	short sb;
	short sc;
    } fw;
#endif

    for (; fcount > 0; --fcount) {
	fp = ifp;
	modifier = *fp++;
	if (strchr("Cbc", modifier) != NULL) {
	    dotinc = 1;
	    w = cget(dot, itype);
	} else if (strchr("doqux", modifier) != NULL) {
	    dotinc = 2;
	    w = sget(dot, itype);
	} else if (strchr("DKOQUXYfp", modifier) != NULL) {
	    dotinc = 4;
	    w = get(dot, itype);
	} else if (modifier != '^') {
	    dotinc = 0;
	}
	if (errflg != NULL)
	    return (fp);
	if (mkfault)
	    error((STRING)NULL);

	var[0] = w;
	if (charpos() == 0 && modifier != 'a')
	    adbpr("%16m");

	switch (modifier) {
	case ' ':
	case '\t':
	    break;

	case 't':
	    adbpr("%T", fcount);
	    return (fp);

	case 'r':
	    adbpr("%M", fcount);
	    return (fp);

	case 'n':
	    printc('\n');
	    break;

	case '"':
	    while (*fp != '"' && *fp != '\0')
		printc(*fp++);
	    if (*fp != '\0')
		fp++;
	    break;

	case '^':
	    dot = inkdot(-dotinc * fcount);
	    return (fp);

	case '+':
	    dot = inkdot(fcount);
	    return (fp);

	case '-':
	    dot = inkdot(-fcount);
	    return (fp);

	case 'a':
	    psymoff(dot, ptype, ":%16t");
	    break;

	case 'p':
	    psymoff(w, ptype, "%16t");
	    break;

	case 'u':
	    adbpr("%-8u", w);
	    break;

	case 'U':
	    adbpr("%-16U", w);
	    break;

	case 'c':
	case 'C':
	    if (modifier == 'C')
		printesc((int)w);
	    else
		printc((char)w);
	    break;

	case 'b':
	    adbpr("%-8r", w);
	    break;

	case 'q':
	    adbpr("%-8r", w);
	    break;

	case 'Q':
	    adbpr("%-16R", w);
	    break;

	case 's':
	case 'S':
	    if (itype == NSP)
		error(catgets(scmc_catd,MS_extern,E_MSG_88,BADMOD));
	    savdot = dot;
	    while ((c = cget(dot, itype)) != '\0' && errflg == NULL) {
		dot = inkdot(1);
		if (modifier == 'S')
		    printesc(c);
		else
		    printc(c);
		if (charpos() >= maxpos)
		    printc('\n');
	    }
	    dotinc = dot - savdot + 1;
	    dot = savdot;
	    break;

	case 'i':
	    if (itype == NSP)
		error(catgets(scmc_catd,MS_extern,E_MSG_88,BADMOD));
	    savdot = dot;
	    printins();
	    printc('\n');
	    dotinc = dot - savdot;
	    dot = savdot;
	    break;

	case 'x':
	    adbpr("%-8x", w);
	    break;

	case 'X':
	    adbpr("%-16X", w);
	    break;

	case 'Y':
	    adbpr("%-24Y", w);
	    break;

	case 'o':
	    adbpr("%-8o", w);
	    break;

	case 'O':
	    adbpr("%-16O", w);
	    break;

	case 'd':
	    adbpr("%-8d", w);
	    break;

	case 'D':
	    adbpr("%-16D", w);
	    break;

#if FLOAT
	case 'f':
	    fw.sa = w;
	    fw.sb = fw.sc = 0;
	    adbpr("%-16.9f", fw);
	    break;

	case 'F':
	    fw.sa = w;
	    fw.sb = sget(inkdot(4), itype);
	    fw.sc = sget(inkdot(6), itype);
	    adbpr("%-32.18F", fw);
	    dotinc = 8;
	    break;
#endif

#ifdef aiws
	case 'K':
	    kcsflags((unsigned long)w);
	    break;
#endif

	default:
	    error(catgets(scmc_catd,MS_extern,E_MSG_88,BADMOD));
	}
	if (itype != NSP)
	    dot = inkdot(dotinc);
	if (charpos() >= maxpos)
	    printc('\n');
    }

    return (fp);
}

void shell()
{
    int    rc;
    int    status;
    int    unixpid;
    STRING argp = lp;

    while (lastc != '\n')
	(void)rdc();
    if ((unixpid = fork()) == 0) {
	if (SIG_ERR == signal(SIGINT, sigint))
	    perror( "shell: signal()");
	if (SIG_ERR == signal(SIGQUIT, sigqit))
	    perror( "shell: signal()");
	*lp=0;
	if (-1 == execl("/bin/sh", "sh", "-c", argp, (char *)0))
	    perror( "shell: execl(/bin/sh)");
	_exit(127);
    }
    if (unixpid == -1)
	error(catgets(scmc_catd,MS_extern,E_MSG_103,NOFORK));
    if (SIG_ERR == signal(SIGINT, SIG_IGN))
	perror( "shell" );
    do rc = wait(&status); while (rc != unixpid && rc != -1);
    if (SIG_ERR == signal(SIGINT, sigint))
	perror( "shell" );
    prints("!");
    lp--;
}

/* NOTE: don't use ctype.h unless it can handle 8-bit chars */
#define is_cntrl(c)     ((c) < 0x20 || (c) == 0x7f)
#define UNCTRL(c)       ((c)^0x40)
#define is_print(c)     ((c) >= 0x20 && (c) < 0x7f)

LOCAL void printesc(c)
int c;
{
    if (c == '~')
	adbpr("~~");
    else if (is_print(c))
	printc(c);
    else if (is_cntrl(c))
	adbpr("~%c", UNCTRL(c));
    else
	adbpr("~<%x>", c);
}

long inkdot(incr)
{
    long newdot;

    newdot = dot + incr;
    if ((dot ^ newdot) >> 24 )
	error(catgets(scmc_catd,MS_extern,E_MSG_80,ADWRAP));
    return (newdot);
}
