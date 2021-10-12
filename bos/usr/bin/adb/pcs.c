static char sccsid[] = "@(#)%M  1.14  src/bos/usr/bin/adb/pcs.c, cmdadb, bos411, 9428A410j  6/19/91  18:37:19";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTION:  subpcs
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  Subprocess control
 */

#include "adb_msg.h"
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include "defs.h"

extern long save_pc;
extern int debugflag;

int save_wtflag = O_RDONLY;

#ifdef _NO_PROTO
void subpcs(modif)
char modif;
#else
void subpcs(char modif)
#endif
{
    int    check;
    int    execsig;
    int    runmode;
    int    oldflag;
    BKPTR  bkptr;
    STRING p;
    static char lastmodif=0;

#if KADB
    extern int rdfd;
    if (rdfd) {
	rdpcs(modif);
	return;
    }
#endif

#ifndef CROSSDB
    execsig = 0;
    loopcnt = cntval;

    if (modif) lastmodif = modif; else modif = lastmodif;

    switch (modif) {
    case 'd':
    case 'D':                     /* delete breakpoint */
	if ((bkptr = scanbkpt(dot)) == NULL)
	    error(catgets(scmc_catd,MS_extern,E_MSG_100,NOBKPT));
	bkptr->flag=0;
	return;

    case 'b':
    case 'B':           /* set breakpoint */
	/* First check to see, if at that location breakpoint already exists
	   or not, if not then set the bkpt */
	oldflag = 0;
	if (bkptr = scanbkpt(dot)) {
	    adbpr("Breakpoint already exists at %X\n", dot);
	    if (bkptr->flag == BKPTEXEC)
		oldflag = 1;
		bkptr->flag = 0;
	}
	for (bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt)
	    if (bkptr->flag == 0)
		break;
	if (bkptr == 0) {
	    if ((p = sbrk(sizeof(BKPT))) == (char *)-1)
		error(catgets(scmc_catd,MS_extern,E_MSG_107,SZBKPT));
	    bkptr = (BKPTR)p;
	    bkptr->nxtbkpt = bkpthead;
	    bkpthead = bkptr;
	}
	bkptr->loc = dot;
	bkptr->count = cntval;
	bkptr->initcnt = bkptr->count;
	bkptr->flag = oldflag ? BKPTEXEC : BKPTSET;
	check = MAXCOM - 11;
	p = bkptr->comm;
	(void)rdc();
	lp--;
	do {
	    *p++ = readchar();
	} while (check-- && lastc != '\n');
	*p = 0;
	lp--;
	if (check == 0)
	    error(catgets(scmc_catd,MS_extern,E_MSG_97,EXBKPT));
	return;

    case 'k':
    case 'K':             /* exit */
	if (pid == 0)
	    error(catgets(scmc_catd,MS_extern,E_MSG_105,NOPCS));
	adbpr("%d: killed", pid);
	endpcs();
	reinit(); 
	return;

    case 'r':
    case 'R':                    /* run program */
	save_wtflag = wtflag;
	wtflag = O_RDONLY;
	if ( pid ) {
		endpcs();
		reinit();
	}
	setup();
	runmode = CONTIN;
	save_pc = 1;
	break;

    case 's':
    case 'S':                         /* single step */
	if (pid) {
	    runmode = SINGLE;
	    execsig = getsig(signo);
	} else {
	    setup();
	    loopcnt--;
	}
	break;

    case 'c':
    case 'C':
    case 0:          /* continue with optional signal */
	if (pid == 0)
	    error(catgets(scmc_catd,MS_extern,E_MSG_105,NOPCS));
	runmode = CONTIN;
	execsig = getsig(signo);
	break;

    default:
	error(catgets(scmc_catd,MS_extern,E_MSG_88,BADMOD));
    }
    if (loopcnt > 0 && runpcs(runmode,execsig))
	adbpr("breakpoint%16t");
    else
	adbpr("stopped at%16t");
   delbp();
   printpc();
#endif
}
