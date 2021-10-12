static char sccsid [] = "@(#)38  1.29  src/bos/usr/bin/ex/ex_tty.c, cmdedit, bos41J, 9508A 2/13/95 16:01:28";
/*
 * COMPONENT_NAME: (CMDEDIT) ex_tty.c
 *
 * FUNCTIONS: WCkpadd, WCkpboth, cost, countnum, fkey, gettmode, kpadd, kpboth,
 * setterm, WCkpboth1
 *
 * ORIGINS: 3, 10, 13, 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1981 Regents of the University of California
 * 
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

#include "ex.h"
#include "ex_tty.h"
#include <values.h>

void	addmac(register wchar_t *, register wchar_t *, register wchar_t *, register struct maps *);
int setupterm(char *, int, int *);
int tgetnum(char *);

#define CNOSTR ((char *) 0)

static void kpboth(struct maps *, struct maps *, char *, char *, char *, char);
static void WCkpboth(struct maps *, struct maps *, wchar_t *, wchar_t *, wchar_t *); 
static void WCkpboth1(struct maps *, struct maps *, wchar_t *, wchar_t *, wchar_t *); 
static void WCkpadd(struct maps *, wchar_t *, wchar_t *, wchar_t *);
static void kpadd(struct maps *, char *, char *, char *);
static wchar_t allocspace[2048];
static wchar_t *freespace;

/* D99366 */
int SuspendCharRemapped=0;

/*
 * Terminal type initialization routines,
 * and calculation of flags at entry or after
 * a shell escape which may change them.
 */
static short GT;

void gettmode(void)
{

	GT = 1;
#ifdef _POSIX_SOURCE
	if (tcgetattr(2, &tty) < 0)
		return;
#else
	if (ioctl(2, TCGETA, &tty) < 0)
		return;
#endif
	if (ospeed != (tty.c_cflag & CBAUD))	/* mjm */
		value(SLOWOPEN) = (tty.c_cflag & CBAUD) < B1200;
	ospeed = tty.c_cflag & CBAUD;
	normf = tty;
	UPPERCASE = (tty.c_iflag & IUCLC) != 0;
	if ((tty.c_oflag & TABDLY) == TAB3 || teleray_glitch)
		GT = 0;
	NONL = (tty.c_oflag & ONLCR) == 0;
}

void setterm(char *type)
{
	register int unknown;
/* AIX security enhancement */
#if !defined (TVI)
	int errret;
#endif
/* TCSEC Division C Class C2 */
	extern char termtype[];

	unknown = 0;
	if (cur_term && exit_ca_mode)
		putpad(exit_ca_mode);
	cur_term = NULL;
	strcpy(termtype, type);
#ifdef TRACE
	if (trace) fprintf(trace, "before setupterm & ioctl, termtype %s ,lines %d, columns %d, clear_screen '%s', cursor_address '%s'\n", termtype, lines, columns, clear_screen, cursor_address);
#endif
/* AIX security enhancement */
#if defined (TVI)
	setupterm(type, 2, 0);
	resetterm();
#else
	setupterm(type, 2, &errret);
	switch(errret) {
	    case -1:			/* Terminfo database not found */
		merror(MSGSTR(M_513, "vi: Terminfo database missing  unable to continue.\n"), DUMMY_INT);
		unknown++;
		/* FALLTHROUGH */
	    case 0:			/* Terminal description not found */
		if (strcmp(type, "dumb") != 0) unknown++;
		cur_term = NULL;
		setupterm("unknown", 1, &errret);
		break;
	    case 1:			/* terminal description found */
		break;
	}
	resetterm();
#ifdef TRACE
	if (trace) fprintf(trace, "after setupterm, termtype %s, lines %d, columns %d, clear_screen '%s', cursor_address '%s'\n", termtype, lines, columns, clear_screen, cursor_address);
#endif
#endif
/* TCSEC Division C Class C2 */
	setsize();
	/*
	 * Initialize keypad arrow keys.
	 */
	freespace = allocspace;

	kpadd(arrows, key_ic, "i", "inschar");
	kpadd(immacs, key_ic, "\033", "inschar");
	kpadd(arrows, key_eic, "i", "inschar");
	kpadd(immacs, key_eic, "\033", "inschar");

	kpboth(arrows, immacs, key_up, "k", "up  ", 0);
	kpboth(arrows, immacs, key_down, "j", "down", 0);
	kpboth(arrows, immacs, key_left, "h", "left", 0);
	kpboth(arrows, immacs, key_right, "l", "right", 0);
	kpboth(arrows, immacs, key_home, "H", "home", 0);
	kpboth(arrows, immacs, key_il, "o\033", "insline", 0);
	kpboth(arrows, immacs, key_dl, "dd", "delline", 0);
	kpboth(arrows, immacs, key_clear, "\014", "clear", 0);
	kpboth(arrows, immacs, key_eol, "d$", "clreol", 0);
	kpboth(arrows, immacs, key_sf, "\005", "scrollf", 0);
	kpadd(arrows, key_dc, "x", "delchar");
	kpadd(immacs, key_dc, "\010", "delchar");
	kpboth(arrows, immacs, key_npage, "\006", "npage", 0);
	kpboth(arrows, immacs, key_ppage, "\002", "ppage", 0);
	kpboth(arrows, immacs, key_sr, "\031", "sr  ", 0);

	kpboth(rmmacs1, rmmacs, key_up, "k", "up  ", 1);
	kpboth(rmmacs1, rmmacs, key_down, "j", "down", 1);
	kpboth(rmmacs1, rmmacs, key_left, "h", "left", 1);
	kpboth(rmmacs1, rmmacs, key_right, "l", "right", 1);
	kpboth(rmmacs1, rmmacs, key_home, "H", "home", 1);
	kpboth(rmmacs1, rmmacs, key_il, "o\033", "insline", 1);
	kpboth(rmmacs1, rmmacs, key_dl, "dd", "delline", 1);
	kpboth(rmmacs1, rmmacs, key_clear, "\014", "clear", 1);
	kpboth(rmmacs1, rmmacs, key_eol, "d$", "clreol", 1);
	kpboth(rmmacs1, rmmacs, key_sf, "\005", "scrollf", 1);
	kpboth(rmmacs1, rmmacs, key_dc, "x", "delchar", 1);
	kpboth(rmmacs1, rmmacs, key_npage, "\006", "npage", 1);
	kpboth(rmmacs1, rmmacs, key_ppage, "\002", "ppage", 1);
	kpboth(rmmacs1, rmmacs, key_sr, "\031", "sr  ", 1);

	/*
	 * key_eos is mapped to the clear to end of file command "dG".
	 * key_eos for the IBM 3100 series of terminals is identified
	 * by the key sequence ESC J. Because this sequence may be
	 * entered for returning to command mode and the joining of lines,
	 * problems may occur. i.e. If the sequence is entered fast by the
	 * user, it is interpreted as the key_eos sequence.
         *
	 * Therefore, for the IBM 3100 series key_eos is not mapped. If
	 * the escape sequence is received for this series of terminals
	 * it is treated as two separate key strokes.
	 */
	if (strncmp(termtype, "ibm31", 5) != 0) {
		kpboth(arrows, immacs, key_eos, "dG", "clreos", 0);
		kpboth(rmmacs1, rmmacs, key_eos, "dG", "clreos", 1);
	}

	/*
	 * Handle funny termcap capabilities
	 */
	if (change_scroll_region && save_cursor && restore_cursor) insert_line=delete_line="";
	if (parm_insert_line && insert_line==NULL) insert_line="";
	if (parm_delete_line && delete_line==NULL) delete_line="";
	if (insert_character && enter_insert_mode==NULL) enter_insert_mode="";
	if (insert_character && exit_insert_mode==NULL) exit_insert_mode="";
	if (GT == 0)
		tab = back_tab = CNOSTR;

#ifdef SIGTSTP
	/*
	 * Now map users susp char to ^Z, being careful that the susp
	 * overrides any arrow key, but only for new tty driver.
	 */
	{
		static wchar_t sc[2];
		int ii; 

#ifdef NTTYDISC
		ioctl(2, TIOCGETD, (char *)&ldisc);
#endif
		if (!value(NOVICE)) {
# if 	defined(_POSIX_JOB_CONTROL)
			if(mbtowc(sc, &tty.c_cc[VSUSP], 1) <= 0){
# else	/*TIOCLGET Berkeley 4BSD */
			if(mbtowc(sc, &olttyc.t_suspc, 1) <= 0){
# endif
				sc[0] = 0;   /* the conversion has failed so set it to null */
			}
			sc[1] = 0;		 
# if	defined(_POSIX_JOB_CONTROL)
			if (tty.c_cc[VSUSP] == Ctrl('Z')) {
# else	/*TIOCLGET Berkeley 4BSD */
			if (olttyc.t_suspc == Ctrl('Z')) {
#endif
				for (ii=0; ii<=4; ii++)
					if (arrows[ii].cap != NULL &&
					    arrows[ii].cap[0] == Ctrl('Z'))
						addmac(sc, (wchar_t *)NULL, (wchar_t *)NULL, arrows);
			} 
			else if (sc[0] != 0) {
				wchar_t addm_tmp[8], addm_tmp2[8]; 
				char *addm_c;	
			        addm_c = "\32";	
				(void) mbstowcs(addm_tmp, addm_c, strlen(addm_c) + 1);	
				addm_c = "susp";
				(void) mbstowcs(addm_tmp2, addm_c, strlen(addm_c) + 1);	
/* D99366 */
				SuspendCharRemapped=1;
				addmac(sc, addm_tmp, addm_tmp2, arrows);
			}
		}
	}
#endif				/* SIGTSTP */

#ifdef TRACE
		if (trace) fprintf(trace, "before OOPS , termtype %s ,lines %d, columns %d, clear_screen '%s', cursor_address '%s'\n", termtype, lines, columns, clear_screen, cursor_address);
#endif
	costCM = cost(tparm(cursor_address, 10, 8));
	if (costCM >= 10000)
		cursor_address = NULL;
	costSR = cost(scroll_reverse);
	costAL = cost(insert_line);
	costDP = cost(tparm(parm_down_cursor, 10));
	costLP = cost(tparm(parm_left_cursor, 10));
	costRP = cost(tparm(parm_right_cursor, 10));
	costCE = cost(clr_eol);
	costCD = cost(clr_eos);
	/* proper strings to change tty type */
	termreset();
	gettmode();
	value(REDRAW) = insert_line && delete_line;
	value(OPTIMIZE) = !cursor_address && !tab;
	if (ospeed == B1200 && !value(REDRAW))
		value(SLOWOPEN) = 1;	/* see also gettmode above */
	if (unknown)
		serror(MSGSTR(M_202, "%s: Unknown terminal type"), type);
#ifdef TRACE
		if (trace) fprintf(trace, "exit setterm , termtype %s ,lines %d, columns %d, clear_screen '%s', cursor_address '%s'\n", termtype, lines, columns, clear_screen, cursor_address);
#endif
}

void setsize(void)
{
	register int l, i;
	int olines = lines, ocols = columns; /* NO clobbering by bad ioctl */
	char *str;
#ifdef TIOCGWINSZ
	struct winsize win;
#endif

	lines = columns = 0;
	str = getenv("LINES");
	if (str)
		lines = atoi(str);
	if (lines < 0)
		lines = 0;
	str = getenv("COLUMNS");
	if (str)
		columns = atoi(str);
	if (columns < 0)
		columns = 0;
#ifdef TIOCGWINSZ
	if (ioctl(0, TIOCGWINSZ, (char *)&win) < 0) {
#endif
		if (!lines)
			lines = tgetnum("li");
		if (!columns)
			columns = tgetnum("co");
#ifdef TIOCGWINSZ
	} else {
		if (!lines && ((lines = winsz.ws_row = win.ws_row) == 0))
			lines = tgetnum("li");
		if (!columns && ((columns = winsz.ws_col = win.ws_col) == 0))
			columns = tgetnum("co");
	}
#endif
	i = lines;
	if (!lines && olines)	/* Prevent clobbering by bad ioctl */
		i = lines = olines;
	if (!columns && ocols)
		columns = ocols;
	if (lines > TUBELINES)
		lines = TUBELINES;
	l = lines;
	if (ospeed < B1200)
		l = 9;	/* including the message line at the bottom */
	else if (ospeed < B2400)
		l = 17;
	if (l > lines)
		l = lines;
	if (columns <= 4)
		columns = 1000;
	value(WINDOW) = options[WINDOW].odefault = l - 1;
	if (defwind) value(WINDOW) = defwind;
	value(SCROLL) = (options[SCROLL].odefault =
		hard_copy ? 11 : (value(WINDOW) / 2));
	if (i <= 0)
		lines = 2;
}

static void kpboth(struct maps *m1, struct maps *m2, char *k, char *m, char *d, char rmacro)
{
	if (k) {
		wchar_t *key, *mapto, *descr;
		int str_len;
		if ((str_len = mbstowcs(freespace, k, strlen(k)+1)) > 0){
			key = freespace; 
			freespace += str_len + 1;
		}
		else
			error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);
		if ((str_len = mbstowcs(freespace, m, strlen(m)+1)) > 0){
			mapto = freespace; 
			freespace += str_len + 1;
		}
		else
			error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);



		if ((str_len = mbstowcs(freespace, d, strlen(d)+1)) > 0){
			descr = freespace; 
			freespace += str_len + 1;
		}
		else
			error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);


		if (rmacro)
			WCkpboth1(m1, m2, key, mapto, descr);
		else
			WCkpboth(m1, m2, key, mapto, descr);
	}
}

static void kpadd(struct maps *map, char *k, char *m, char *d)
{
	if (k) {
		wchar_t *key, *mapto, *descr;
		int str_len;

		if ((str_len = mbstowcs(freespace, k, strlen(k)+1)) > 0){
			key = freespace; 
			freespace += str_len + 1;
		}
		else
			error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);


		if ((str_len = mbstowcs(freespace, m, strlen(m)+1)) > 0){
			mapto = freespace; 
			freespace += str_len + 1;
		}
		else
			error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);


		if ((str_len = mbstowcs(freespace, d, strlen(d)+1)) > 0){
			descr = freespace; 
			freespace += str_len + 1;
		}
		else
			error(MSGSTR(M_650, "Invalid multibyte string, conversion failed."), DUMMY_INT);

		WCkpadd(map, key, mapto, descr);
	}
}

/*
 * Map both map1 and map2 as below.  map2 surrounded by esc and a.
 */
static void WCkpboth(struct maps *map1, struct maps *map2, wchar_t *key, wchar_t *mapto, wchar_t *desc)
{
	wchar_t *p;

	WCkpadd(map1, key, mapto, desc);
	if (any(*key, "\b\n "))
		return;
	p = freespace;
	*freespace++ = '\033';
	wcscpy(freespace, mapto);
	freespace = WCstrend(freespace);
	*freespace++ = 'a';
	*freespace++ = '\0';
	WCkpadd(map2, key, p, desc);
}

/*
 * Map both map1 and map2 as below.  map1 surrounded by 'esc' and 'R' and map2
 * surrounded by 'esc l' and 'R'.
 */
static void WCkpboth1(struct maps *map1, struct maps *map2, wchar_t *key, wchar_t *mapto, wchar_t *desc)
{
	wchar_t *p;

	p = freespace;
	*freespace++ = '\033';
	wcscpy(freespace, mapto);
	freespace = WCstrend(freespace);
	*freespace++ = 'R';
	*freespace++ = '\0';
	WCkpadd(map1, key, p, desc);
	if (any(*key, "\b\n "))
		return;
	p = freespace;
	*freespace++ = '\033';
	*freespace++ = 'l';
	wcscpy(freespace, mapto);
	freespace = WCstrend(freespace);
	*freespace++ = 'R';
	*freespace++ = '\0';
	WCkpadd(map2, key, p, desc);
}

/*
 * Define a macro.  mapstr is the structure (mode) in which it applies.
 * key is the input sequence, mapto what it turns into, and desc is a
 * human-readable description of what's going on.
 */
static void WCkpadd(struct maps *mapstr, wchar_t *key, wchar_t *mapto, wchar_t *desc)
{
	int i;

	for (i=0; i<MAXNOMACS; i++)
		if (mapstr[i].cap == 0)
			break;
	if (i >= MAXNOMACS)
		return;
	mapstr[i].cap = key;
	mapstr[i].mapto = mapto;
	mapstr[i].descr = desc;
}

char *
fkey(int i)
{
	switch (i) {
	case 0: return key_f0;
	case 1: return key_f1;
	case 2: return key_f2;
	case 3: return key_f3;
	case 4: return key_f4;
	case 5: return key_f5;
	case 6: return key_f6;
	case 7: return key_f7;
	case 8: return key_f8;
	case 9: return key_f9;
	case 10: return key_f10;
	case 11: return key_f11;
	case 12: return key_f12;
	case 13: return key_f13;
	case 14: return key_f14;
	case 15: return key_f15;
	case 16: return key_f16;
	case 17: return key_f17;
	case 18: return key_f18;
	case 19: return key_f19;
	case 20: return key_f20;
	case 21: return key_f21;
	case 22: return key_f22;
	case 23: return key_f23;
	case 24: return key_f24;
	case 25: return key_f25;
	case 26: return key_f26;
	case 27: return key_f27;
	case 28: return key_f28;
	case 29: return key_f29;
	case 30: return key_f30;
	case 31: return key_f31;
	case 32: return key_f32;
	case 33: return key_f33;
	case 34: return key_f34;
	case 35: return key_f35;
	case 36: return key_f36;
	default: return CNOSTR;
	}
}

/*
 * cost figures out how much (in characters) it costs to send the string
 * str to the terminal.  It takes into account padding information, as
 * much as it can, for a typical case.	(Right now the typical case assumes
 * the number of lines affected is the size of the screen, since this is
 * mainly used to decide if insert_line or scroll_reverse is better, and
 * this always happens at the top of the screen.  We assume cursor motion
 * (cursor_address) has little * padding, if any, required, so that case,
 * which is really more important than insert_line vs scroll_reverse,
 * won't be really affected.)
 */
static int costnum;
static int cost(char *str)
{
	/*int countnum(void);*/

	if (str == NULL || *str=='O')	/* OOPS */
		return 10000;	/* infinity */
	costnum = 0;
	tputs(str, lines, (int(*)(int))countnum);
	return costnum;
}

/* ARGSUSED */
static void countnum(char ch)
{
	costnum++;
}
