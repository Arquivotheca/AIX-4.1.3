static char sccsid[] = "@(#)01	1.15  src/bos/usr/ccs/lib/libcur/cr_tty.c, libcur, bos411, 9428A410j 6/10/91 18:37:02";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: gettmode, setterm, packit, packx, zap, getcap
 *
 * ORIGINS: 9, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cur99.h"

/*
 * These next three structurs contain the character codes for the terminal
 * capabilities we are interested in reading from the extended version of
 * the TERMCAP database as well as the address of the variable to contain
 * the value read.
 */

static struct BLNMPT {		/* boolean names pointer pairs  */
    char    nm[2];		/* name is two characters wide  */
    char   *pt;			/* pointer to corresponding var */
}                       bnt[] = {/* booleans name table          */
                            'a', 'm', &AM,
				/* name 'am' variable 'AM'      */
                            'b', 's', &BS,
                            'b', 'w', &BW,
                            'd', 'a', &DA,
                            'd', 'b', &DB,
                            'e', 'o', &EO,
                            'e', 's', &ES,
                            'h', 's', &HS,
                            'h', 'z', &HZ,
                            'i', 'n', &IN,
                            'k', 'm', &KM,
                            'm', 'i', &MI,
                            'm', 's', &MS,
                            'n', 'c', &NC,
                            'o', 's', &OS,
                            'u', 'l', &UL,
                            'x', 'n', &XN,
                            '\0', '\0', NULL
};				/* nulls terminate table        */


static struct NUNMPT {		/* numeric names pointer pairs  */
    char    nm[2];		/* name is two characters wide  */
    int    *pt;			/* pointer to corresponding var */
}                       nnt[] = {/* numerics name table          */
                            'c', 'o', &CO,
                            'i', 't', &IT,
                            'l', 'i', &LI,
                            'l', 'm', &LM,
                            's', 'g', &SG,
                            'p', 'b', &PB,
                            'v', 't', &VT,
                            'w', 's', &WS,
                            '\0', '\0', NULL
};				/* nulls terminate table        */

static char *xPC;

static struct STNMPT {		/* string  names pointer pairs  */
    char    nm[2];		/* name is two characters wide  */
    char  **pt;			/* pointer to corresponding var */
}                       snt[] = {/* strings  name table          */
                            'k', '<', &K11,
				/* function key 11              */
                            'k', '>', &K12,
				/* function key 12              */
                            'k', '!', &K13,
				/* function key 13              */
                            'k', '@', &K14,
				/* function key 14              */
                            'k', '#', &K15,
				/* function key 15              */
                            'k', '$', &K16,
				/* function key 16              */
                            'k', '%', &K17,
				/* function key 17              */
                            'k', '^', &K18,
				/* function key 18              */
                            'k', '&', &K19,
				/* function key 19              */
                            'k', '*', &K20,
				/* function key 20              */
                            'k', '(', &K21,
				/* function key 21              */
                            'k', ')', &K22,
				/* function key 22              */
                            'k', '-', &K23,
				/* function key 23              */
                            'k', '_', &K24,
				/* function key 24              */
                            'k', '+', &K25,
				/* function key 25              */
                            'k', ',', &K26,
				/* function key 26              */
                            'k', ':', &K27,
				/* function key 27              */
                            'k', '?', &K28,
				/* function key 28              */
                            'k', '[', &K29,
				/* function key 29              */
                            'k', ']', &K30,
				/* function key 30              */
                            'k', '{', &K31,
				/* function key 31              */
                            'k', '}', &K32,
				/* function key 32              */
                            'k', '|', &K33,
				/* function key 33              */
                            'k', '~', &K34,
				/* function key 34              */
                            'k', '/', &K35,
				/* function key 35              */
                            'k', '=', &K36,
				/* function key 36              */
			    'F', '@', &K37, /* function key 37 */
			    'F', 'A', &K38, /* function key 38 */
			    'F', 'B', &K39, /* function key 39 */
			    'F', 'C', &K40, /* function key 40 */
			    'F', 'D', &K41, /* function key 41 */
			    'F', 'E', &K42, /* function key 42 */
			    'F', 'F', &K43, /* function key 43 */
			    'F', 'G', &K44, /* function key 44 */
			    'F', 'H', &K45, /* function key 45 */
			    'F', 'I', &K46, /* function key 46 */
			    'F', 'J', &K47, /* function key 47 */
			    'F', 'K', &K48, /* function key 48 */
			    'F', 'L', &K49, /* function key 49 */
			    'F', 'M', &K50, /* function key 50 */
			    'F', 'N', &K51, /* function key 51 */
			    'F', 'O', &K52, /* function key 52 */
			    'F', 'P', &K53, /* function key 53 */
			    'F', 'Q', &K54, /* function key 54 */
			    'F', 'R', &K55, /* function key 55 */
			    'F', 'S', &K56, /* function key 56 */
			    'F', 'T', &K57, /* function key 57 */
			    'F', 'U', &K58, /* function key 58 */
			    'F', 'V', &K59, /* function key 59 */
			    'F', 'W', &K60, /* function key 60 */
			    'F', 'X', &K61, /* function key 61 */
			    'F', 'Y', &K62, /* function key 62 */
			    'F', 'Z', &K63, /* function key 63 */
                            'a', 'l', &AL,
                            'a', 's', &AS,
                            'a', 'e', &AE,
                            'b', 'c', &BC,
                            'b', 'l', &BL,
                            'b', 't', &BT,
                            'c', 'd', &CD,
                            'c', 'e', &CE,
                            'c', 'h', &CH,
                            'c', 'l', &CL,
                            'c', 'm', &CM,
                            'c', 'r', &CR,
                            'c', 's', &CS,
                            'c', 'v', &CV,
                            'd', 'c', &DC,
                            'd', 'l', &DL,
                            'd', 'm', &DM,
                            'd', 'o', &DO,
                            'D', 'O', &xDO,
                            'd', 's', &DS,
                            'e', 'c', &EC,
                            'e', 'd', &ED,
                            'e', 'i', &EI,
                            'f', 's', &FS,
                            'h', 'o', &HO,
                            'i', 'c', &IC,
                            'i', 'm', &IM,
                            'i', 'p', &IP,


                            'k', '0', &K0,
				/* function key 0               */
                            'k', '1', &K1,
				/* function key 1               */
                            'k', '2', &K2,
				/* function key 2               */
                            'k', '3', &K3,
				/* function key 3               */
                            'k', '4', &K4,
				/* function key 4               */
                            'k', '5', &K5,
				/* function key 5               */
                            'k', '6', &K6,
				/* function key 6               */
                            'k', '7', &K7,
				/* function key 7               */
                            'k', '8', &K8,
				/* function key 8               */
                            'k', '9', &K9,
				/* function key 9               */
                            'k', ';', &K10,
				/* function key 10              */

                            'S', '1', &KS1,
				/* Special function key 1       */
                            'S', '2', &KS2,
				/* Special function key 2       */
                            'S', '3', &KS3,
				/* Special function key 3       */
                            'S', '4', &KS4,
				/* Special function key 4       */
                            'S', '5', &KS5,
				/* Special function key 5       */
                            'S', '6', &KS6,
				/* Special function key 6       */
                            'S', '7', &KS7,
				/* Special function key 7       */
                            'S', '8', &KS8,
				/* Special function key 8       */
                            'S', '9', &KS9,
				/* Special function key 9       */
                            'S', 'A', &KS10,
				/* Special function key 10      */


                            'K', '1', &KA1,
				/* keypad top left              */
                            'K', '2', &KA3,
				/* keypad top right             */
                            'K', '3', &KB2,
				/* keypad center                */
                            'K', '4', &KC1,
				/* keypad bottom left           */
                            'K', '5', &KC3,
				/* keypad bottom right          */

                            'K', 'r', &KEXT_S6,
				/* customized key 6             */
                            'K', 'R', &KEXT_R6,
				/* customized return for key 6  */
                            'K', 's', &KEXT_S7,
				/* customized key 7             */
                            'K', 'S', &KEXT_R7,
				/* customized return for key 7  */
                            'K', 't', &KEXT_S8,
				/* customized key 8             */
                            'K', 'T', &KEXT_R8,
				/* customized return for key 8  */
                            'K', 'u', &KEXT_S9,
				/* customized key 9             */
                            'K', 'U', &KEXT_R9,
				/* customized return for key 9  */
                            'K', 'v', &KEXT_S1,
				/* customized key 1             */
                            'K', 'V', &KEXT_R1,
				/* customized return for key 1  */
                            'K', 'w', &KEXT_S2,
				/* customized key 2             */
                            'K', 'W', &KEXT_R2,
				/* customized return for key 2  */
                            'K', 'x', &KEXT_S3,
				/* customized key 3             */
                            'K', 'X', &KEXT_R3,
				/* customized return for key 3  */
                            'K', 'y', &KEXT_S4,
				/* customized key 4             */
                            'K', 'Y', &KEXT_R4,
				/* customized return for key 4  */
                            'K', 'z', &KEXT_S5,
				/* customized key 5             */
                            'K', 'Z', &KEXT_R5,
				/* customized return for key 5  */

                            'k', 'a', &Ka,
				/* clear-all-tabs               */
                            'k', 'A', &KA,
				/* insert line                  */
                            'k', 'b', &KB,
				/* backspace                    */
                            'k', 'c', &KCMD,
				/* command key                  */
                            'k', 'C', &KC,
				/* clear screen                 */
                            'k', 'D', &KD,
				/* delete character             */
                            'k', 'd', &Kd,
				/* cursor down                  */
                            'k', 'e', &Ke,
				/* exit keypad transmit mode    */
                            'k', 'E', &KE,
				/* clear to end of line         */
                            'k', 'F', &KF,
				/* scroll forward               */
                            'k', 'H', &KH,
				/* home-down key                */
                            'k', 'h', &Kh,
				/* home key                     */
                            'k', 'i', &KDO,
				/* DO key                       */
                            'k', 'I', &KI,
				/* ins char/enter insert mode   */
                            'k', 'L', &KL,
				/* delete line                  */
                            'k', 'l', &Kl,
				/* cursor left                  */
                            'k', 'M', &Km,
				/* change insert mode           */
                            'k', 'n', &KNL,
				/* new line                     */
                            'k', 'N', &KN,
				/* next page                    */
                            'k', 'o', &KTAB,
				/* tab key                      */
                            'k', 'O', &KBTAB,
				/* back tab                     */
                            'k', 'p', &KPCMD,
				/* previous command             */
                            'k', 'P', &KP,
				/* previous page                */
                            'k', 'q', &KHLP,
				/* Help key                     */
                            'k', 'Q', &KQUIT,
				/* Quit key                     */
                            'k', 'r', &Kr,
				/* cursor right                 */
                            'k', 'R', &KR,
				/* scroll backward              */
                            'k', 'S', &KS,
				/* clear to end of screen       */
                            'k', 's', &Ks,
				/* enter keypad transmit mode   */
                            'k', 't', &Kt,
				/* clear tab                    */
                            'k', 'T', &KT,
				/* set tab stop                 */
                            'k', 'u', &Ku,
				/* cursor up                    */
                            'k', 'U', &KSEL,
				/* select key                   */
                            'k', 'v', &KNPN,
				/* next pane key                */
                            'k', 'V', &KPPN,
				/* previous pane key            */
                            'k', 'w', &KEND,
				/* end key                      */
                            'k', 'W', &KCPN,
				/* command pane key             */
                            'k', 'z', &KSCL,
				/* scroll left                  */
                            'k', 'Z', &KSCR,
				/* scroll right                 */
                            'k', 'J', &KACT,
				/* action key                   */

                            'l', 'e', &LE,
                            'L', 'E', &xLE,
                            'l', 'l', &LL,
                            'm', 'a', &MA,
                            'm', 'b', &MB,
                            'm', 'd', &MD,
                            'm', 'e', &ME,
                            'm', 'h', &MH,
                            'm', 'k', &MK,
                            'm', 'm', &MM,
                            'm', 'o', &MO,
                            'm', 'p', &MP,
                            'm', 'r', &MR,
                            'n', 'd', &ND,
                            'n', 'l', &NL,
                            'p', 'c', &xPC,
                            'R', 'I', &xRI,
                            's', 'e', &SE,
                            's', 'f', &SF,
                            's', 'o', &SO,
                            's', 'r', &SR,
                            't', 'a', &TA,
                            't', 'e', &TE,
                            't', 'i', &TI,
                            't', 's', &TS,
                            'u', 'c', &UC,
                            'u', 'e', &UE,
                            'u', 'p', &UP,
                            'U', 'P', &xUP,
                            'u', 's', &US,
                            't', 'p', &TP,
                            'b', 'm', &BM,
                            'r', 'v', &RV,
                            'l', 'v', &LV,
                            'v', 'b', &VB,
                            'v', 'e', &VE,
                            'v', 'i', &VI,
                            'v', 's', &VS,
                            'b', 'x', &BX,
				/* primary box forming chars   */
                            'b', 'y', &BY,
				/* alternate box characters    */
                            'B', 'x', &Bx,
				/* attribute names for BX use  */
                            'B', 'y', &By,
				/* attribute names for BY use  */
                            'c', '0', &CF[0],
                            'c', '1', &CF[1],
                            'c', '2', &CF[2],
                            'c', '3', &CF[3],
                            'c', '4', &CF[4],
                            'c', '5', &CF[5],
                            'c', '6', &CF[6],
                            'c', '7', &CF[7],
                            'd', '0', &CB[0],
                            'd', '1', &CB[1],
                            'd', '2', &CB[2],
                            'd', '3', &CB[3],
                            'd', '4', &CB[4],
                            'd', '5', &CB[5],
                            'd', '6', &CB[6],
                            'd', '7', &CB[7],
                            'f', '0', &FO[0],
                            'f', '1', &FO[1],
                            'f', '2', &FO[2],
                            'f', '3', &FO[3],
                            'f', '4', &FO[4],
                            'f', '5', &FO[5],
                            'f', '6', &FO[6],
                            'f', '7', &FO[7],
                            'z', 'a', &ZA,
				/* application character string */
                            '\0', '\0', NULL
};				/* nulls terminate table        */

extern char
           *tgoto ();

static char tspace[1024],	/* Space for terminal capability strings 
				*/
           *aoftspace;		/* Address of tspace for relocation */

short   ospeed = -1;

/*
 * NAME:                gettmode, setterm, zap, getcap
 *
 * FUNCTION: These routines are called by initscr() to
 *      get the state of the terminal and to set it to something else if
 *      necessary; they also read the /etc/termcap database.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      setterm(type), where type is a pointer to a string which
 *      contains the name of the terminal from $TERM (if available).
 *
 * EXTERNAL REFERENCES: Gtty(), savetty(), tgetent(), tgetnum(), tgoto(),
 *                      longname(), tgetflag(), tgetstr()
 *
 * DATA STRUCTURES:     aoftspace
 *
 * RETURNS:             normal -> OK            error -> ERR
 */

/*
 * NAME:                gettmode
 *
 * FUNCTION:  do terminal type initialization routines, and calculation
 *            of flags at entry.
 */

gettmode() {
#if (IS1|IS2|V7)
    if (gtty(_tty_ch, &_tty) < 0)
	return;
    savetty();
    if (stty(_tty_ch, &_tty) < 0)
	_tty.sg_flags = _res_flg;
    ospeed = _tty.sg_ospeed;
    _res_flg = _tty.sg_flags;
    GT = ((_tty.sg_flags & XTABS) == 0);
    NONL = ((_tty.sg_flags & CRMOD) == 0);
    _tty.sg_flags &= ~XTABS;
    stty(_tty_ch, &_tty);
#else
    if (Gtty(_tty_ch, &_tty) < 0)
	return;
    savetty();
    ospeed = _tty.c_cflag & CBAUD;
    UPPERCASE = ((_tty.c_iflag & IUCLC) != 0);
    GT = ((_tty.c_oflag & TABDLY) != TAB3 && TA);
    NONL = ((_tty.c_oflag & ONLCR) == 0) ||
	((_tty.c_oflag & OPOST) == 0);
				/* NONL indicates that terminal */
				/* will process \n as a CR and  */
				/* an LF rather than as just LF */
				/* ONLCR indicates this is being */
				/* assumed by the system if off */
				/* and OPOST off also implies   */
				/* ONLCR is inactive            */

#endif
}

/*
 * NAME:                setterm
 */

setterm(type)
register char  *type;
{
    register int    unknown;
    static char defbox[] = "+-+|++-|-|+";
				/* default box characters       */
				/* must be static since pointer */
				/*   is saved in external       */

    unknown = FALSE;
    if (type[0] == '\0')
	type = "xx";
    if (tgetent(type, type) != 1) {/* first parameter not used at all */
	unknown++;
    }

    aoftspace = tspace;

    zap();			/* get terminal description */

				/* check for cursor addressability */
    if (tgoto(CM, 0, 0)[0] == 'O')
				/* Note: this comparison is with a cap O, 
				*/
	CA = FALSE, CM = 0;	/* not a 0 (zero).  This is because tgoto 
				*/
    else
	CA = TRUE;		/* returns "OOPS" when it fails.          
				*/

    PC = xPC ? xPC[0] : FALSE;	/* pad character */

    if (LINES == 0)		/* lines on screen */
	LINES = LI;

    if (LINES <= 5)
	LINES = 24;

    if (COLS == 0)		/* columns on screen */
	COLS = CO;
    if (COLS <= 4)
	COLS = 80;

    if (SG < 0)			/* magic cookie glitch terminal? */
	SG = 0;

    if (IT < 0)			/* if no tab spacing default to 8 */
	IT = 8;

/*
 * The line/box drawing sequences should be exactly
 * 11 characters long. if not force them to the
 * default box/line characters.
 */
    if (strlen(BX) != 11)
	BX = defbox;
    if (strlen(BY) != 11)
	BY = defbox;

    aoftspace = tspace;		/* probably not necessary */

    if (unknown)
	return ERR;

    return OK;
}

/*
 * NAME:                packit
 *
 * FUNCTION: This routine will test the argument string for starting with a
 *      0x00 or 0x80, if true the first character will be set to 0x00 and
 *      the remainder of the string will be translated from hexadecimal to
 *      binary. This is used to provide arbitrary binary strings which
 *      terminfo will not (it translates all 0x00's to 0x80.
 */

packit(sp)
char   *sp;			/* pointer to string to trans   */

{
    if (sp != NULL &&
	    (*sp == (unsigned char)(0x80) || *sp == (unsigned char)(0x00))) {
				/* if string needs translation  */
	char   *np;		/* pointer to next new char spot */
	char   *cp;		/* pointer to next hex char     */

	np = sp;		/* initialize output ptr        */
	*np++ = '\0';		/* change leading char to null  */

	for (cp = np; *cp != '\0';) {/* step thru remainder of string */
	    *np++ = packx(*cp) * 16 + packx(*(cp + 1));
				/* convert each pair of char    */
	    cp += 2;		/* step over pair               */
	}			/* end loop to convert          */
    }				/* end if translate needed      */
    return;			/* return to caller             */
}				/* end function packit          */

/*
 * NAME:                packx
 */

packx(c)			/* convert hex char to number   */
char    c;			/* arg is character             */
{
    return((c >= '0' && c <= '9') ? (c - '0') :
				/* in range 0-9 return 0-9      */
	    ((c >= 'a' && c <= 'f') ? (c - 'a' + 10) :
				/* in range a-f return 10-15    */
		((c >= 'A' && c <= 'F') ? (c - 'A' + 10) :
				/* in range A-F return 10-15    */
		    (0))));	/* default return 0             */

}				/* end function packx           */


/*
 * NAME:                zap
 *
 * FUNCTION:  This routine gets all the terminal information from the
 *      termcap database.
 */

zap() {
    register struct BLNMPT *nampb;
    register struct NUNMPT *nampn;
    register struct STNMPT *namps;
    extern char *tgetstr ();

/*
 * get boolean flags
 */
    nampb = &bnt[0];

    while (nampb->nm[0] != '\0') {
	*(nampb->pt) = tgetflag(&(nampb->nm[0]));
#ifdef DEBUG
	fprintf(stderr, "\nBOOL: %.2s = %d", &(nampb->nm[0]), *(nampb->pt));
#endif
	nampb++;
    }

/*
 * get numeric values
 */
    nampn = &nnt[0];

    while (nampn->nm[0] != '\0') {
	*(nampn->pt) = tgetnum(&nampn->nm[0]);
#ifdef DEBUG
	fprintf(stderr, "\nNUMS: %.2s = %d", nampn, *(nampn->pt));
#endif
	nampn++;
    }

/*
 * get string values
 */
    namps = &snt[0];

    while (namps->nm[0] != '\0') {
	*(namps->pt) = tgetstr(&namps->nm[0], &aoftspace);

    /* 
     * Terminfo translates any binary zeros in the control strings
     * to 0x80, to allow binary zeros in the TI and TE strings the
     * following will translate the string from hexadecimal to
     * binary if needed. Translate will be done if the first char
     * of the string is 0x00 or 0x80.
     */

	packit(*(namps->pt));	/* pack the string if needed    */


#ifdef DEBUG
	{			/* <--- begin debug test block */
	    char   *debugpt;
	    debugpt = *(namps->pt);
	    if (debugpt) {
		fprintf(stderr, "\nSTRG: %.2s = ", &(namps->nm[0]));
		while (*debugpt != '\0')
		    fprintf(stderr, " %.2X",(int)(*debugpt++));
	    }
	    else {
		fprintf(stderr, "\nstng: %.2s = NULL", &(namps->nm[0]));
	    }
	}			/* <--- end debug test block  */
#endif
	namps++;
    }
}


/*
 * NAME:                getcap
 *
 * FUNCTION: get a string capability from the entry.
 */

char   *
        getcap (name)
char   *name;
{
    extern char *tgetstr ();
    return tgetstr(name, &aoftspace);
}
