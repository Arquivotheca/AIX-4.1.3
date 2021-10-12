
static char sccsid[] = "@(#)00	1.17  src/bos/usr/bin/adb/command.c, cmdadb, bos411, 9428A410j 2/24/94 12:35:04";
/*
 * COMPONENT_NAME: (CMDADB) assembler debugger
 *
 * FUNCTION: command
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
 *  Command decoding
 */

#include "defs.h"
#include "adb_msg.h"
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

extern FILEMAP *FileMap, *SaveMap;
extern int debugflag;
extern int loadcnt;
extern int coredump;
extern int kmem_core;

LOCAL char eqformat[128] = "z";
LOCAL char stformat[128] = "X\"= \"^i";
LOCAL int  locmsk;
BOOL rdwr_core;
int map_changed = 0;

#ifdef _NO_PROTO
void command(buf, defcom)
STRING  buf;
char    defcom;
#else
void command(STRING buf, char defcom)
#endif
{
    int    itype;
    int    ptype;
    int    modifier;
    int    regptr;
    int    cnt;
    int    eqcom;
    char   wformat[1];
    char   savc;
    long    w;
    long   savdot;
    STRING savlp = lp;

    if (buf) {
	if (*buf == '\n')
	    return;
	else
	    lp = buf;
    }
    rdwr_core = FALSE;

    do {
	if (adrflg = expr(0)) {
	    dot = expv;
	    ditto = dot;
	}
	adrval = dot;

	TR1PRINT("Adrflg = %s, ", (adrflg ? "TRUE" : "FALSE"));
	TR1PRINT("adrval = %#x\n", adrval);

	if (rdc() == ',' && expr(0)) {
	    cntflg = TRUE;
	    cntval = expv;
	}
	else {
	    cntflg = FALSE;
	    cntval = 1;
	    lp--;
	}

	TR1PRINT("Cntflg = %s, ", (cntflg ? "TRUE" : "FALSE"));
	TR1PRINT("cntval = %d\n", cntval);

	if ( ! eocmd(rdc()))
	    defcom = lastc;
	else {
	    if (adrflg == 0)
		dot = inkdot(dotinc);
	    lp--;
	}
	if (buf == NULL) lastcom = defcom;

	TR1PRINT("Type of command: %c\n", defcom & ~STARREDCMD);

	switch (defcom & ~STARREDCMD) {
	case '/':
	    itype = DSP;
	    ptype = DSYM;
	    if ( coredump || kmem_core )
  	      rdwr_core = TRUE;
	    goto trystar;

	case '=':
	    itype = NSP;
	    ptype = NSYM;
	    goto trypr;

	case '?':
	    itype = ISP;
	    ptype = ISYM;
	    goto trystar;

trystar:
	    if (rdc() == '*') {
		lastcom |= STARREDCMD;
		itype |= STAR;
	    } else
		lp--;

trypr:
	    eqcom = defcom == '=';

	    switch (rdc()) {
	    case 'm':
		{ /* Reset map data */
		    struct ldinfo *ldmap;
		    union {
			FILEMAP *m;
			MAP  * core_map;
			unsigned long *mp;
		    } amap, smap;

		    short  mapndx = 0;
		    short  fcount;

		    if (eqcom)
			error(catgets(scmc_catd,MS_extern,E_MSG_83,BADEQ));
		    if ( cntflg ) {
			mapndx = cntval;
			if ( mapndx >= loadcnt ) {
			    adbpr("bad file map number,\n");
			    adbpr("use $m to see the corect file map");
			    adbpr(" numbers\n");
		 	    adbpr("File map numbers are enclosed within []\n");
			    while ( rdc() != '\n');
			    lp--;

			    break;
			}
		     }
		    if ( itype & DSP )  {
			map_changed = 1;
			amap.core_map = &slshmap;
		    }
		    else {
			amap.mp = (unsigned long *)&FileMap[mapndx];
		    }
		    ldmap = &loader_info[mapndx];
		    
		    fcount = 3;
		    if (itype & STAR)
			amap.mp += 3;
		    while (fcount-- && expr(0)) {
			*(amap.mp)++ = expv;
		    }
		    if (rdc() == '?')
			slshmap.ufd = FileMap[mapndx].fd ;
		    else if (lastc == '/')
			if ( coredump )
				smap.m->fd = fcor;
			else
			     adbpr("warning: core doesn't exists\n");
		    else
			lp--;
		    /* Update the global map tables */

		    loader_info[mapndx].textorg = FileMap[mapndx].begin_txt;
		    loader_info[mapndx].dataorg = FileMap[mapndx].begin_data;
	    	    SaveMap[mapndx].begin_txt = FileMap[mapndx].begin_txt;
	    	    SaveMap[mapndx].end_text   = FileMap[mapndx].end_text;
	    	    SaveMap[mapndx].begin_data = FileMap[mapndx].begin_data;
	    	    SaveMap[mapndx].end_data   = FileMap[mapndx].end_data;
	    	    SaveMap[mapndx].txt_seekadr = FileMap[mapndx].txt_seekadr;
	    	    SaveMap[mapndx].data_seekadr = FileMap[mapndx].data_seekadr;
		}
		break;

	    case 'l': /* Search for exp */
	    case 'L':
		cnt = lastc=='l' ? 2 : 4;

		if (eqcom)
		    error(catgets(scmc_catd,MS_extern,E_MSG_83,BADEQ));
		dotinc = 1;
		savdot = dot;
		(void) expr(1);
		locval = expv;
		if (expr(0))
		    locmsk = expv;
		else
		    locmsk = -1L;
		if (cnt == 2) {
		    locmsk &= 0xFFFF;
		    locval &= 0xFFFF;
		}
		for ( ; ; ) {
		    w = cnt==4 ? get(dot, itype) : sget(dot, itype);
		    if (errflg != NULL || mkfault ||
			(w & locmsk) == locval)
			break;
		    dot = inkdot(dotinc);
		}
		if (errflg != NULL) {
		    dot = savdot;
		    errflg = catgets(scmc_catd,MS_extern,E_MSG_104,NOMATCH);
		}
		psymoff(dot, ptype, "");
		break;

	    case 'V':
	    case 'w':
	    case 'W':
		if (eqcom)
		    error(catgets(scmc_catd,MS_extern,E_MSG_83,BADEQ));
		if (lastc == 'V') {
		    cnt = 1;
		    wformat[0] = 'b';
		} else if (lastc == 'w') {
		    cnt = 2;
		    wformat[0] = 'q';
		} else {
		    cnt = 4;
		    wformat[0] = 'Q';
		}
		(void) expr(1);
		do {
		    savdot = dot;
		    psymoff(dot, ptype, ":%16t");
		    (void) exform(1, wformat, itype, ptype);
		    errflg = NULL;
		    dot = savdot;
		    if (cnt == 1)
			cput(dot, itype, expv);
		    else if (cnt == 2)
			sput(dot, itype, expv);
		    else
			put(dot, itype, expv);
		    savdot = dot;
		    adbpr("=%8t");
		    (void) exform(1, wformat, itype, ptype);
		    newline();
		} while (expr(0) && errflg == NULL);
		dot = savdot;
		chkerr();
		break;
	    default:
		lp--;
		getformat(eqcom ? eqformat : stformat);
		if (!eqcom)
		   psymoff(dot, IDSYM, ":%16t");
		scanform(cntval, (eqcom ? eqformat : stformat), itype, ptype);
	    }
	    break;

	case '>':
	    lastcom = 0;
	    savc = rdc();

	    if ((regptr = getreg(savc)) != NOTREG) {
		LVADBREG(regptr) = XL(dot);
		(void)ptrace(WDUSER, pid, REGTOSYS(regptr), LVADBREG(regptr));
	    } else if ((modifier = varchk(savc)) != -1)
		var[modifier] = dot;
	    else
		error(catgets(scmc_catd,MS_extern,E_MSG_94,BADVAR));

	    break;

	case '!':
	    lastcom = 0;
	    shell();
	    break;

	case '$':
	    lastcom = 0;
	    dollar(nextchar());
	    break;

	case ':':
	    if ( ! executing) {
		executing = TRUE;
		subpcs(nextchar());
		executing = FALSE;
#if !COLON_MAY_REPEAT
		lastcom = 0;
#endif
	    }
	    break;

	case '\0':
	    /* prints(DBNAME); */
	    break;

	 case '\n' : 
		break;
	default:
	    error(catgets(scmc_catd,MS_extern,E_MSG_81,BADCOM));
	}

	flushbuf();
    } while (rdc() == ';');
    if (buf)
	lp = savlp;
    else
	lp--;
}
