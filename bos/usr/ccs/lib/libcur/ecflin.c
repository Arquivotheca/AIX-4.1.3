static char sccsid[] = "@(#)89  1.9  src/bos/usr/ccs/lib/libcur/ecflin.c, libcur, bos411, 9428A410j 6/16/90 01:38:35";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS:   ecflin, NLecflin, split_chars, unsplit_chars, sj_adjust,
 *              markchg, ecflind, updtmsk, decr_max_right,
 *              get_row_col_from_index
 *
 * ORIGINS: 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include        "cur99.h"
#include        "cur05.h"
#include        "cur02.h"

#ifdef  KJI
#include        <NLchar.h>
#endif

/*                                                                      */
/* static arrays for character validity mask initialization             */
/* each mask contains as many entries as necessary with the bit set     */
/* to 1 indicating that the corresponding character is a member of the  */
/* set.  The sets may be combined through use of the validity code      */
/* string argument to ecflin.                                           */

/* Selection of mask sets depends on character set in use.              */
#ifdef  NLS
#include "nlscm.h"
#endif
#ifdef  KJI
#include "kjicm.h"
#endif

/*      The following fields are external to allow access by the        */
/*      application programs and other routines. The application        */
/*      can set the fill character to whatever character it desires     */
/*      note that only the character can be set not the attribute.      */
/*      Insert mode can similarly be reset by an application and is     */
/*      controlled by default by the code in ecpnin.                    */

NLSCHAR ECFILLC = NLSBLANK;	/* fill character used to fill  */
				/* the displayed field when a   */
				/* delete or backspace removes  */
				/* a character                  */

extern char _extended;		/* boolean for shift codes      */

/*      the following are static to allow access by ecflind             */

static
int     maxr;			/* Last row of field (in field) */
static
int     maxc;			/* Last col of filed (in field) */

static
int     r1;			/* utility row index            */
static
int     c1;			/* utility column index         */

static
int     minrx;			/* copy of bottom edge spec     */
static
int     mincx;			/* copy of left edge spec       */
static
int     widthx;			/* copy of width for field      */

static
int     mmaxr;			/* max row  when buffer filled  */
static
int     mmaxc;			/* max col  when buffer filled  */

static
int     crow;			/* current row (next input)     */
static
int     ccol;			/* current column               */

static
	NLSCHAR *nlbuff;        /* temporary buffer to hold     */
static
	NLSCHAR *psbuff;        /* pseudo pointer to buffer     */
				/* - calculated so that subscr  */
				/* - calculation can use window */
				/* - coordinates (saves arith)  */
static
int     shifts;			/* count of # of shifts in buff */
static
int     max_right;		/* rightmost char pos. in buffer */

static
	WINDOW  *pw;            /* pointer to p-space window    */

/*
 * NAME:                ecflin
 *
 * FUNCTION:            Accept input to a field as defined by the
 *                      input parameters.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   PARAMETERS:        pn - pointer to PANE for window containing
 *                           the field.
 *                      ly - begining row for the field in the
 *                           p-space for the pane (not the view)
 *                      lx - begining column for the fiels in the
 *                           p-space for the pane
 *                      dp - depth of the field in number of rows
 *                           range 1 - p-space depth
 *                      wd - width of the field in number of columns
 *                           range 1 to p-space width
 *                      cm - pointer to string that specifies valid
 *                           character sets for this field
 *                      ch - first character to be processed if null
 *                           will read first character
 *                      da - pointer to character array to hold input
 *                           must be at least dp*wd characters long
 *                      ln - length of character array for input
 *                      vm - validity mask - pointer to a character
 *                           array where each bit indicates whether
 *                           the corresponding character is to be
 *                           accepted for the field. 512 bits must be
 *                           defined (32 bytes) though the first 4
 *                           bytes (and the corresponding characters)
 *                           will not be accepted as character data
 *
 *   INITIAL CONDITIONS keypad must be active, noecho must be set,
 *                      the pane should be part of the top panel.
 *                      The cursor should be positioned within the
 *                      field.
 *
 *   FINAL CONDITIONS:
 *     NORMAL:          Field updated on display and data area
 *                      reflects any input characters.
 *
 *     ABNORMAL:        N/A
 *
 * EXTERNAL REFERENCES: ecrfpl, ecpnin, waddch, wmove
 *
 * RETURNED VALUES:     keycode for the first key which was not
 *                      processed or if field was exited by typing to
 *                      the end, the last valid key code.
 */

int     ecflin (pn, minr, minc, depth, width, cset, char1, buff, vmask)
	PANE    *pn;            /* ptr to pane for input data   */
int     minr;			/* first row of field in window */
int     minc;			/* first column of field        */
int     depth;			/* number of rows in field      */
				/* - in range 1 - window depth  */
int     width;			/* number of columns in filed   */
				/* - in range 1 - window width  */
char   *cset;			/* Pointer to string that for   */
				/* - character set validity spec */
int     char1;			/* first input code to be used  */
				/* - if zero code is ignored    */
char   *buff;			/* area to hold input data from */
				/* - terminal, must be at least */
				/* - width*depth characters long */
char   *vmask;			/* validity mask - bits must be */
				/* - set to indicate valid input */
				/* - values, first 32 bytes     */
				/* - correspond to ASCII data   */
				/* - remaining codes map codes  */
				/* - from keypad. Total size    */
				/* - must be 64 bytes. Accessed */
				/* - only if cset contains 'Z'  */

{				/* begin ecflin                 */

    return(NLecflin(pn, minr, minc, depth, width, cset,
		char1, buff, width * depth, vmask));

}				/* end ecflin() */


/*
 * NAME:                NLecflin
 */

int
NLecflin (pn, minr, minc, depth, width, cset, char1, buff, leng, vmask)
	PANE    *pn;            /* ptr to pane for input data   */
int     minr;			/* first row of field in window */
int     minc;			/* first column of field        */
int     depth;			/* number of rows in field      */
				/* - in range 1 - window depth  */
int     width;			/* number of columns in filed   */
				/* - in range 1 - window width  */
char   *cset;			/* Pointer to string that for   */
				/* - character set validity spec */
int     char1;			/* first input code to be used  */
				/* - if zero code is ignored    */
char   *buff;			/* area to hold input data from */
				/* - terminal, must be at least */
				/* - width*depth characters long */
int     leng;			/* length of character array    */
				/* REL 3.0 - added for NLS supp */
char   *vmask;			/* validity mask - bits must be */
				/* - set to indicate valid input */
				/* - values, first 32 bytes     */
				/* - correspond to ASCII data   */
				/* - remaining codes map codes  */
				/* - from keypad. Total size    */
				/* - must be 64 bytes. Accessed */
				/* - only if cset contains 'Z'  */

{				/* begin NLecflin               */

    register
    int     key;		/* input data code              */

    register
		NLSCHAR *cp;    /* utility char ptr - NLSCHAR   */
    register
		ATTR    *ap;    /* utility char ptr - attributes */
    register
    char   *tp;			/* utility char ptr             */

    int     echo_save;		/* save old echo value          */
    int     extd_save;		/* save old extended value      */
				/*                              */
    NLSCHAR ic;			/* insert character holder      */
    ATTR    ia;                 /* insert attr code holder      */

    NLSCHAR tn;			/* temp NLSCHAR holder          */
    char    sc,
            tc;			/* temp character holder        */
    ATTR    ta;                 /* temp attribute holder        */
    short   prev_bytes;		/* temp holder of byte count    */
				/* input before return to appl  */
    int     i,
            j;

#ifdef  NLS
    char    cm[KEY_MAXNLS / 8];	/* character validity ck mask   */
#endif

#ifdef  KJI
    char    cm[KEY_MAXC / 8];   /* character validity ck mask   */
    int     chsize;             /* input character size         */
    NLSCHAR nlc;                /* scratch for char breakdown   */
    unsigned char   nlcb[2];
    int     adj_dir;            /* cursor adjustment (+ or -)   */
#endif

    int     setsw;		/* switch for set/or/cand       */
#define swset   0
#define swor    1
#define swcand  2

    pw = pn->w_win;		/* get pointer to p-space       */

    echo_save = _echoit;	/* save echo value              */
    extd_save = _extended;	/* save extended value          */

    _extended = 1;		/* turn extended on             */
    _echoit =			/* turn echo off                */
	shifts =		/* initialize counters          */
	max_right = 0;

    minrx = minr;		/* copy minr                    */
    mincx = minc;		/* copy minc for delete routine */
    widthx = width;		/* same for width               */

    maxr = minr + depth - 1;	/* calculate upper bound        */
    maxc = minc + width - 1;	/* - for row and column         */

/* Set the cm mask to indicate the keystrokes which are to be locally   */
/* processed by this routine.  The mask will be initialized to the      */
/* default which allows all graphics and the control keys which are     */
/* processed by this routine.  The string will be interpreted as follows*/
/* If the first character is not a + or - the corresponding character   */
/* set will replace the default and a + will be assumed to follow the   */
/* character.  A + will cause all following sets to be 'added' to the   */
/* mask (logically or'ed). A - will cause all following sets to be      */
/* subtracted (compliment and 'and') from the mask.  The characters     */
/* should be from the following set                                     */
/*      U - Upper Case letters                                          */
/*      L - Lower Case letters                                          */
/*      X - Hexadecimal characters - [0-9], [a-f] and [A-F]             */
/*      N - Numerics [0-9]                                              */
/*      A - Alpha Numerics, A+L+N                                       */
/*      B - The space character 0x20                                    */
/*      G - Graphics 0x21 - 0x7e                                        */
/*      P - Printable characters B+G                                    */
/*      C - Cursor movement keys                                        */
/*      D - Default character set                                       */
#ifdef  KJI
/*      J - Japanese characters - kanji, katakana, hiragana             */
/*      H - Hiragana                                                    */
/*      k - single-byte katakana                                        */
/*      K - all Katakana                                                */
#endif


    setsw = swset;		/* initialize switch            */

    updtmsk(&setsw, cm, cm_D, sizeof(cm_D));
				/* set default mask             */

    setsw = swset;		/* initialize switch            */

    while (cset != NULL && *cset != '\0') {
				/* until end of string          */
	switch (*cset++) {
	    case '+': 		/* add following specifications */
		setsw = swor;
		break;
	    case '-': 		/* remove following specs       */
		setsw = swcand;
		break;
	    case 'D': 		/* default set                  */
		updtmsk(&setsw, cm, cm_D, sizeof(cm_D));
		break;
	    case 'U': 		/* Upper case alphabetic        */
		updtmsk(&setsw, cm, cm_U, sizeof(cm_U));
		break;
	    case 'L': 		/* lower case alphabetic        */
		updtmsk(&setsw, cm, cm_L, sizeof(cm_L));
		break;
	    case 'N': 		/* Numeric                      */
		updtmsk(&setsw, cm, cm_N, sizeof(cm_N));
		break;
	    case 'X': 		/* Hexadecimal                  */
		updtmsk(&setsw, cm, cm_X, sizeof(cm_X));
		break;
	    case 'B': 		/* Space                        */
		updtmsk(&setsw, cm, cm_B, sizeof(cm_B));
		break;
	    case 'G': 		/* graphics                     */
		updtmsk(&setsw, cm, cm_G, sizeof(cm_G));
		break;
	    case 'C': 		/* cursor move etc              */
		updtmsk(&setsw, cm, cm_C, sizeof(cm_C));
		break;
	    case 'A': 		/* alpha numeric                */
#ifdef  KJI
		updtmsk(&setsw,cm,cm_A,sizeof(cm_A)) ;
#else
		updtmsk(&setsw, cm, cm_U, sizeof(cm_U));
		updtmsk(&setsw, cm, cm_L, sizeof(cm_L));
		updtmsk(&setsw, cm, cm_N, sizeof(cm_N));
#endif
		break;
	    case 'P': 		/* printable = G+B              */
		updtmsk(&setsw, cm, cm_G, sizeof(cm_G));
		updtmsk(&setsw, cm, cm_B, sizeof(cm_B));
		break;
#ifdef  KJI
	    case 'J':           /* Japanese */
	      updtmsk(&setsw,cm,cm_J,sizeof(cm_J));
	      break;
	    case 'H':           /* Hiragana */
	      updtmsk(&setsw,cm,cm_H,sizeof(cm_H));
	      break;
	    case 'k':           /* 1-byte Katakana */
	      updtmsk(&setsw,cm,cm_k,sizeof(cm_k));
	      break;
	    case 'K':           /* all Katakana */
	      updtmsk(&setsw,cm,cm_K,sizeof(cm_K));
	      break;
#endif
	    case 'Z': 		/* Caller defined               */
		updtmsk(&setsw, cm, vmask, sizeof(cm));
		break;
	    default: 
		break;
	}
    }

/*
 *  use a buffer of NLSCHAR to store the data
 */

#ifdef  KJI
	/* For SJIS, it's OK to keep the bytes of characters separate
	 * (since it stays one character per column), but we want NLSCHAR
	 * sized units in order to track first/second byte info and
	 * partial-character indicators. */
    i = leng; leng = width*depth;       /* can't use > width*depth anyway */
    if (i < leng ||
	(nlbuff = (NLSCHAR *)calloc(leng, sizeof(NLSCHAR))) == NULL)
	    return (KEY_BAD);
    for (i=0, j=leng-1;  i<j; ) {       /* stop short of last char */
	    sc = buff[i];
	    if (sc>' ')                 /* if printable, non-space */
		    max_right = i;      /* (2-col space is kept) */
	    if (NCisshift(sc)) {
		    nlbuff[i++] = sc | FIRST_BYTE;
		    nlbuff[i] = buff[i] | SECOND_BYTE;
		    i++;
	    } else  nlbuff[i++] = sc;
    }
    if (i==j) {                         /* prevent 1/2 wide char at end */
	    sc = buff[i];
	    if (NCisshift(sc))
		    nlbuff[i] = PART_CHAR;
	    else {
		    nlbuff[i] = sc;
		    max_right = i;
	    }
    }
    shifts = 0;                     /* unused for SJIS */
    /* Verify that there aren't any characters "hanging over" the
     * edges of the field--if there are, we can't push characters
     * around and obey field boundaries.  (Ecflin will also maintain
     * this condition through editing.)  This condition is also necessary
     * for consistent screen updating when we split characters across
     * lines of the field. */
    for (r1=minr; r1<=maxr; r1++)
	if (is_first_byte(pw->_y[r1][maxc])
	||  is_second_byte(pw->_y[r1][minc]) )
		return (KEY_BAD);

#else

    if (!(nlbuff = (NLSCHAR *) calloc(leng, sizeof(NLSCHAR))))
	return (KEY_BAD);

    for (i = 0, j = 0;		/* initialize data buffer       */
	    i < leng;
	    ++i, ++j) {
	if (j < leng - 1) {	/* are there 2 bytes left in buff */
	    sc = buff[j];	/* get one byte from buffer     */
	    tc = buff[j + 1];	/* get next byte from buffer    */
	}
	else {
	    if (j == leng - 1) {/* one byte left in buffer      */
		if ((buff[j] & 0xfc) != 0x1c) {
				/* not a shift character        */
		    nlbuff[i] = buff[j];
				/* assign last byte from buff   */
		    if (buff[j] > 0x20)/* is a printable char          */
			max_right = i;
		}
		else		/* add blank at end since shift */
		    nlbuff[i] = NLSBLANK;
	    }
	    break;		/* exit for loop                */
	}
	if (((sc & 0xfc) == 0x1c) && (tc & 0x80)) {
				/* have hit a 2-byte NLS char   */
	    tn = (NLSCHAR) sc;
	    nlbuff[i] = (NLSCHAR)((tn << 8) | tc);
	    ++shifts;		/* increment count of shift bytes */
	    ++j;
	    max_right = i;
	}
	else {			/* assign one byte char         */
	    nlbuff[i] = (NLSCHAR) sc;
	    if (sc > 0x20)
		max_right = i;	/* maintain position of rightmost */
	}			/* printable character          */
    }


    if (shifts) {
	int     row,
	        col;
	get_row_col_from_index(&row, &col, leng - shifts);
	mmaxr = row;
	mmaxc = col;
    }
    else
#endif  KJI

    {
	mmaxr = maxr;
	mmaxc = maxc;
    }

    j = minr * width + minc;	/* calculate offset constants   */

    psbuff = nlbuff - j;	/* calculate pseudo origin for  */
				/* - data buffer. with this     */
				/* - value location in buff for */
				/* - row r and column c relative */
				/* - to the field origin i.e.   */
				/* - buff+(r*width +c) can be   */
				/* - calculated using window    */
				/* - coordinates rw and cw by   */
				/* - psbuff+(rw*width+c)        */
				/* - saves subtracting minc and */
				/* - minr each time before doing */
				/* - offset calculation         */

    getyx(pw, crow, ccol);	/* get current cursor position  */
				/* - if not in field will soon  */
				/* - exit with default          */

#ifdef  KJI
sj_adjust(-1);                  /* fix initial cursor           */
#endif

    key = (char1 ? char1 : KEY_NOKEY);/* set default return code      */

    while (crow >= minr &&	/* while cursor remains in field */
	    crow <= maxr &&
	    ccol >= minc &&
	    ccol <= maxc) {
#ifdef  NLS
	if (shifts &&		/* if shifts, use other max. vals */
		((ccol > mmaxc && crow == mmaxr) || crow > mmaxr))
	    break;
#endif
#ifdef  KJI
	split_chars();          /* get edges right to display   */
#endif

	wmove(pw, crow, ccol);	/* move cursor to proper posn   */
	ecrfpn(pn);		/* update this pane             */

	key = ecpnin(pn, FALSE, char1);/* get first char from term     */
	char1 = NULL;		/* ensure that next call gets   */
				/* - new terminal data          */

	getyx(pw, crow, ccol);	/* get cursor in case moved by  */
				/* - the locator                */
#ifdef  KJI
	unsplit_chars();        /* in case we move them         */
	sj_adjust(-1);          /* re-adjust after locator      */
#endif

	if (crow < minr ||	/* if cursor now outside field  */
		crow > maxr ||
		ccol < minc ||
		ccol > maxc)
	    break;		/* exit from read loop          */

#ifdef  NLS
	if (shifts &&
		((ccol > mmaxc && crow == mmaxr) || crow > mmaxr))
	    break;
#endif

	if (key < KEY_MAXC)	/* if code can be checked       */
	    i = key;
#ifdef  NLS
	else
	    if (key >= 0x1c80 && key <= 0x1fff) {
		switch ((key & 0x1f00) >> 8) {
		    case (P1LO_SHIFT): 
			i = P1LO_PACK;
			break;
		    case (P1HI_SHIFT): 
			i = P1HI_PACK;
			break;
		    case (P2LO_SHIFT): 
			i = P2LO_PACK;
			break;
		    case (P2HI_SHIFT): 
			i = P2HI_PACK;
			break;
		}
		i = key - i;
	    }
#endif
#ifdef  KJI
	else if ( key >= 0x8000) {
	    if (width == 1)             /* 2-byte char in 1-col field? */
		goto FLIN_END;
	    if (key >= 0x8800)
		i = key>>8;             /* use upper byte as index */
	    else if (key >= 0x829f) {   /* use split-ward encoding */
		i = 0x82 + (((key>>8)-0x82)*2);
	    /* The way that JIS is encoded into SJIS places two
	     * classes of characters into some wards, with the second
	     * class starting at 9f. */
		if ((key&0xff) >= 0x9f)
		    i++;
	    } else if (key < 0x8200)    /* punctuation below romaji */
		    i = 0x80;
	    else if (key < 0x8260)      /* split digits, roman */
		    i = 0x81;
	    else    i = 0x82;
	}
#endif
	else
	    goto FLIN_END;

	if (cm[i >> 3] & (0x80 >> (i & 0x07))) {
				/* - and mask indicates key ok, if
				   'simple' data key         */

	/* *******  data key     **************** */
#ifdef NLS
	    if ((key <= 0xff) || (key >= 0x1c80)) {
		i = crow * width + ccol;
				/* index into psbuff            */

		if (ECINSMD)	/* if inserting, don't care     */
		    prev_bytes = 0;/* about overlaying a shift byte */
		else {
		    if (winch(pw) > 0x1c7f)
				/* 2 byte char in this position */
			prev_bytes = 1;
		    else	/* 1 byte char in this position */
			prev_bytes = 0;
		}
				/* is being replaced by a  :    */
		if ((NLSCHAR) key > 0x1c7f)
				/* 2 byte code   OR             */
		    shifts += 1 - prev_bytes;
		else		/* 1 byte code                  */
		    shifts += 0 - prev_bytes;

		if ((key == 0x0020) && !ECINSMD && (i - j == max_right))
		    decr_max_right();/* if adding blank at end       */
		else {
		    if (i - j > max_right) {
				/* adding at end of buffer      */
			if (key > 0x0020)
				/* adding a non-blank           */
			    max_right = i - j;
		    }
		    else {	/* if inserting into buffer     */
			if (ECINSMD && (max_right < leng - 1))
			    ++max_right;
		    }
		}

		if (ECINSMD) {	/* ******* if insert mode is active ***** 
				*/
		    ic = (NLSCHAR) key;/* initialize insert char       */
		    ia = pw->_csbp;/* init insert attribute        */

		    for (r1 = crow, c1 = ccol;
				/* init to current row/col      */
			    r1 <= maxr;/* - while row is in field      */
			    r1++, c1 = minc) {
				/* step row pointer             */
				/* - after 1st row start at left */
				/* update change vectors        */

			if (pw->_firstch[r1] = _NOCHANGE) {
				/* if no prior change in row    */
			    pw->_firstch[r1] = minc;
				/* then set change for field    */
			    pw->_lastch[r1] = maxc;
				/* - start/end columns          */
			}
			else {  /* if prior change consider old -
				   range in updating ptrs     */
			    if (pw->_firstch[r1] > minc)
				pw->_firstch[r1] = minc;
				/* set first change to leftmost */

			    if (pw->_lastch[r1] < maxc)
				pw->_lastch[r1] = maxc;
				/* set last change to rightmost */
			}
				/* shift window data to make rm */
			for (cp = &(pw->_y[r1][c1]),
				/* set ptr to first data and    */
				ap = &(pw->_a[r1][c1]);
				/* - attribute strings          */
				cp <= &(pw->_y[r1][maxc]);
				/* - until end of field in row  */
			    ) {	/* - increment in loop          */
			    tn = *cp;/* get current char             */
			    *cp++ = ic;/* set insert char              */
			    ic = tn;/* save curr char as next insert */

			    ta = *ap;/* get attr character           */
			    *ap++ = ia;/* set insert attribute         */
			    ia = ta;/* save curr attr as next ins   */

			}	/* end - shift char in row right */
		    }		/* end - process rows of field  */

				/* now shift buffer to right    */

		    ic = (NLSCHAR) key;/* initialize insert character  */
		    for (cp = &psbuff[i];
				/* start at current field posn  */
				/* step to end of field         */
			    cp <= &psbuff[maxr * width + maxc];
			) {	/* stepping performed in loop   */
			tn = *cp;/* get current char             */
			*cp++ = ic;/* insert character             */
			ic = tn;/* set for next insert          */
		    }

		}               /* ********* end insert processing **********
				*/

		else {          /* ****** else simple char, no insert *******
				*/
		    waddch(pw,(NLSCHAR) key);
				/* add new char to window       */
				/* also add to data buffer      */
		    psbuff[i] = (NLSCHAR) key;
		}               /* ****** end - simple char, no insert ******
				*/

	    /* here, delete character from end of string if */
	    /* adding character within the string fills up  */
	    /* the buffer.                                  */
		while (max_right + 1 + shifts > leng) {
		    if (nlbuff[max_right] >= 0x1c80)
			--shifts;
		    nlbuff[max_right] = NLSBLANK;
		    {
			int     row,
			        col;
			get_row_col_from_index(&row, &col, max_right + 1);
			mvwaddch(pw, row, col, ECFILLC);
		    }
		    decr_max_right();
		}
				/* now adjust row/col pointers  */
		if (++ccol > maxc) {/* if at right edge             */
		    if (++crow <= maxr) {
				/* if not at end of field       */
			ccol = minc;/* set col back to left edge    */
		    }
		    else {
			wmove(pw, --crow, ccol);
				/* position cursor (outside fld) */
			key = KEY_NOKEY;
				/* set default return code      */
		    }
		}
	    }                   /* ******** end simple data key *********/
#endif
#ifdef  KJI
	/* Pushing chars from line to line causes problems.  We use partial
	 * char indicators when characters split, BUT use a form which
	 * allows re-joining after another push.  These "broken" characters
	 * must be turned into partial characters on exit from ecflin,
	 * since field boundaries need not align with window or pane boun-
	 * daries. */
	    if (key<0xff || key>KEY_MAXC) {     /* KJI data key? */
		i = crow*width + ccol;
		nlc = key;
		chsize = NCenc(&nlc,nlcb);
		if (chsize==2 && crow==maxr && ccol==maxc) {
		    key = PART_CHAR;            /* won't fit */
		    chsize = 1;
		}
		if (ECINSMD) {
		    for (r1=maxr; r1>crow; ) {  /* flow rows below cursor */
			for (cp = pw->_y[r1], ap = pw->_a[r1],
			     c1 = maxc-chsize; c1>=minc; c1--) {
			    cp[c1+chsize] = cp[c1];
			    ap[c1+chsize] = ap[c1];
			}
			markchg(r1,minc,maxc);
			r1--;
			cp[c1+chsize] = pw->_y[r1][maxc];       /* wrap down */
			ap[c1+chsize] = pw->_a[r1][maxc];
			if (chsize==2) {
			    cp[minc] = pw->_y[r1][maxc-1];
			    ap[minc] = pw->_a[r1][maxc-1];
			}
		    }
		    /* adjust row containing cursor */
		    for (cp = pw->_y[crow], ap = pw->_a[crow], c1 = maxc-chsize;
			 c1>=ccol; c1--) {
			cp[c1+chsize] = cp[c1];
			ap[c1+chsize] = ap[c1];
		    }
		    if (is_first_byte(pw->_y[maxr][maxc]))
					/* pushed 2nd byte off */
			pw->_y[maxr][maxc] = PART_CHAR;
		    markchg(crow,ccol,maxc);
		    j = maxr*width+maxc-chsize;
					/* kill leftover half at end */
		    if (is_first_byte(psbuff[j]))
			psbuff[j] = PART_CHAR;
		    for ( ; j>=i; j--)  /* push user buff, insert below */
			psbuff[j+chsize] = psbuff[j];
		} else {                /* not insert mode */
		    if (ccol>=maxc && chsize==2) {
			mark_change(pw, crow, ccol);
			mark_change(pw, crow+1, minc);
		    } else
			markchg(crow, ccol, ccol+1);
		}
		/* Everything is adjusted; now put in the character.  We know
		 * there's room for it, from prior checks. */
		pw->_a[crow][ccol] = pw->_csbp;
		if (chsize==1) {
		    pw->_y[crow][ccol] = key;
		    ccol++;
		} else {
		    pw->_y[crow][ccol] = nlcb[0] | FIRST_BYTE;
		    if (++ccol > maxc) {/* wrap; guaranteed to have room */
			ccol = minc;
			crow++;
		    }
		    pw->_a[crow][ccol] = pw->_csbp;
		    pw->_y[crow][ccol] = nlcb[1] | SECOND_BYTE;
		    ccol++;
		}
		if (!ECINSMD) {
		    /* if next char (after wrapping, if needed and possible) is
		     * second half, we overwrote the first half, so get rid of
		     * the fragment.  This is where things will go wrong if
		     * psbuff doesn't match window for character sizes. */
		    if (ccol<=maxc &&
			is_second_byte(*(cp = &pw->_y[crow][ccol])) ||
			crow<maxr &&
			is_second_byte(*(cp =&pw->_y[crow+1][minc]))) {
			*cp = psbuff[i+chsize] = PART_CHAR;
		    }
		}
		if (chsize==1)
		    psbuff[i] = key;
		else {
		    psbuff[i] = nlcb[0];/* add new char - must have room */
		    psbuff[i+1] = nlcb[1];
		}

		if (ccol > maxc) {      /* adjust cursor position */
		    if (++crow <= maxr)
			ccol = minc;
		    else {              /* else out of field */
			wmove(pw, --crow, ccol);
			key = KEY_NOKEY;
		    }
		}
	    }
				 /******* end simple KJI data key *******/
#endif

	    else {              /* ******** else coded input key ********/
#ifdef  KJI
      /* Default adjustment (in case we land on second half of a wide char)
       * is leftward.  */
		adj_dir = -1;
#endif
		switch (key) {	/* switch based on key type     */
		    case KEY_LEFT: /* cursor left                  */
			--ccol;	/* shift cursor position left   */
			break;
		    case KEY_RIGHT: /* cursor right                 */
			++ccol;	/* cursor to right              */
#ifdef  KJI
			adj_dir = 1;
#endif
			break;
		    case KEY_UP: /* cursor up                    */
			--crow;	/* decrement current row        */
			break;
		    case KEY_DOWN: /* cursor down                  */
			++crow;	/* increment current row        */
			break;
		    case KEY_SCR: /* Scroll right                 */
			ecscpn(pn, 0, 5);
				/* scroll the pane              */
			ccol += 5;/* make cursor follow it        */
#ifdef  KJI
			adj_dir = 1;
#endif
			break;
		    case KEY_SCL: /* Scroll left                  */
			ecscpn(pn, 0, -5);
				/* scroll the pane              */
			ccol -= 5;/* make cursor follow it        */
			break;
		    case KEY_SR: /* Scroll up                    */
			ecscpn(pn, -5, 0);
				/* scroll the pane              */
			crow -= 5;/* make cursor follow it        */
			break;
		    case KEY_SF: /* Scroll down                  */
			ecscpn(pn, 5, 0);
				/* scroll the pane              */
			crow += 5;/* make cursor follow it        */
			break;
		    case KEY_DC: /* delete character             */
			ecflind();/* invoke delete routine        */
			decr_max_right();
				/* decrement rightmost index    */
			break;
		    case KEY_BTAB: /* back tab                     */
			if (crow == minr &&
				/* if currently at top left     */
				ccol == minc) {
				/*                              */
			    goto FLIN_END;
				/* exit now                     */
			}
			else {
			    crow = minr;
				/* else set cursor to top left  */
			    ccol = minc;
			}
		    case KEY_BACKSPACE: 
				/* backspace/rubout             */
			if (crow != minr ||
				/* if at top left, ignore rubout */
				ccol != minc) {
			    if (--ccol < minc) {
				/* if at left edge              */
				ccol = maxc;
				/* wrap back to previous line   */
				--crow;/*                              */
			    }
#ifdef  KJI
			    sj_adjust(-1);      /* be sure we're on char*/
#endif
			    ecflind();  /* now delete that character    */
			    decr_max_right();
			}
			break;
		    case KEY_NEWL:      /* new line                     */
			crow++;         /* move to next row, first col  */
			ccol = minc;
#ifdef  KJI
			adj_dir = 1;
#endif
			break;
		    case KEY_EOS:       /* erase to end of field        */
			decr_max_right();
			for (r1 = crow, c1 = ccol;
					/* step thruough rows           */
				r1 <= maxr;
					/* remaining in the field       */
				r1++, c1 = minc) {
			    for (; c1 <= maxc; c1++) {
					/* step through space in row, if
					   deleted char is 2 bytes   */
				wmove(pw, r1, c1);
#ifdef  NLS
				if (winch(pw) & 0x1c00)
				    --shifts;
#endif
				waddch(pw, ECFILLC);
				/* fill the displayed field     */
			    /* fill the applications buffer */
				psbuff[r1 * width + c1] = NLSBLANK;
			    }
			}
			break;
		}		/* end switch based on key code */

#ifdef  KJI
	    sj_adjust(adj_dir);
#endif
	    }                   /* ********* end coded input key ********/

	    {
		int     row,
		        col;
		get_row_col_from_index(&row, &col, leng - shifts);
		mmaxr = row;
		mmaxc = col;
	    }
	}			/* ******* end if data or key to echo**** 
				*/

	else			/* **** else not data or key to echo **** 
				*/
	    goto FLIN_END;

    }				/* ******** end while in field ********** 
				*/

FLIN_END: 			/* exit from routine thru here  */

#ifdef  KJI
	/* If characters were split during editing, freeze them into
	 * partial-char indicators on exit.  Ugly - loses data - but
	 * necessary to maintain consistency of window data once field
	 * is gone.  All paths to FLIN_END are from places where the
	 * partial markings are gone (unsplit called most recently). */
    for (r1 = minr; r1<=maxr; r1++) {
	if (is_second_byte(pw->_y[r1][minc]))
		pw->_y[r1][minc] = PART_CHAR;
	if (is_first_byte(pw->_y[r1][maxc]))
		pw->_y[r1][maxc] = PART_CHAR;
    }
#endif

    wmove(pw, crow, ccol);	/* update cursor position       */
    ecrfpn(pn);			/* refresh the pane             */

    _echoit = echo_save;	/* restore echo                 */
    _extended = extd_save;	/* restore extended             */

#ifdef  NLS
    for (cp = nlbuff,		/* return input data in buff    */
	    tp = buff;
	    tp < (buff + leng);
	    ++tp, ++cp) {
	if (*cp < 256)
	    *tp = (char) * cp;
	else {			/* decode into two bytes of data */
	    *tp = (char)(*cp >> 8);
	    ++tp;
	    *tp = (char)((*cp << 8) >> 8);
	}
    }
#endif

#ifdef  KJI
    for (cp = nlbuff+leng, tp = buff+leng;  tp > buff; )
	*--tp = *--cp;
#endif

    free(nlbuff);		/* free up temporary buffer     */
    return(key);		/* return ending code to caller */
}				/* end ecflin                   */

#ifdef  KJI
/*
 * NAME:                split_chars
 *
 * FUNCTION:    Mark chars split across lines.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Called to mark characters split from line to line within a field.
 *      The general approach to updating the screen is unsplit_chars,
 *      do the update, split_chars.  This is cheaper than analyzing all
 *      of the special cases within the screen-updating code.  N.B.:
 *      Unsplit is NOT the same as the cleanup at exit from ecflin.
 */

split_chars()
{       register int    r;
	register NLSCHAR *cp;

	for (r = minrx; r <= maxr; r++) {
		cp = pw->_y[r];
		if (is_first_byte(cp[maxc]))
			cp[maxc] = (cp[maxc]&~BYTE_MASK) | FIRST_PART;
		if (is_second_byte(cp[mincx]))
			cp[mincx] = (cp[mincx]&~BYTE_MASK) | SECOND_PART;
	}
}

/*
 * NAME:                unsplit_chars
 *
 * FUNCTION:    Remove split-char markers
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Remove the split-character markers from the field, by marking
 *      them as FIRST_BYTE and SECOND_BYTE.  The sequence
 *              split_chars();
 *              unsplit_chars();
 *      is an identity.
 */

unsplit_chars()
{       register int r;
	register NLSCHAR *cp;

	for (r = minrx; r <= maxr; r++) {
		cp = pw->_y[r];
		if (is_first_part(cp[maxc]))
			cp[maxc] = (cp[maxc]&~BYTE_MASK) | FIRST_BYTE;
		if (is_second_part(cp[mincx]))
			cp[mincx] = (cp[mincx]&~BYTE_MASK) | SECOND_BYTE;
	}
}

/*
 * NAME:                sj_adjust
 *
 * FUNCTION:    Adjust cursor position for SJIS.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Check the cursor position:  If it is within the field but on the
 *      second byte of a two-byte character, move it in the indicated
 *      direction (+ = right, - = left) and update it.  This may move
 *      cursor out of the field.
 */

static
sj_adjust(dir)
int     dir;
{
	if (is_second_byte(pw->_y[crow][ccol])  /* most likely to fail  */
	&& crow>=minrx && crow<=maxr && ccol>=mincx && ccol<=maxc) {
		if (dir>0)
			ccol++;
		else    ccol--;
	}
}
#endif

/*
 * NAME:                markchg
 *
 * FUNCTION:    Mark changes on a line.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Mark (or extend) the changed region on row r of pw to include
 *      columns c1 through c2.
 */

static
markchg(r, c1,c2)
int     r;
register int    c1, c2;
{       register short  *fp = &pw->_firstch[r],
			*lp = &pw->_lastch[r];

	if (*fp == _NOCHANGE) {
		*fp = c1;
		*lp = c2;
	} else {
		if (*fp > c1)
			*fp = c1;
		if (*lp < c2)
			*lp = c2;
	}
}

/*
 * NAME:                ecflind
 *
 * FUNCTION: utilizes global variables to identify current location in
 *      the field and display.
 */

static
int     ecflind()                       /* local function for delete    */
{                                       /* begin function ecflind       */

register
NLSCHAR *ocp, *cp;                      /* old, new char pointers */
register
ATTR    *oap, *ap;                      /* old, new attribute pointers */
register
int     r1, c1;                         /* row, column indices */
NLSCHAR *oclim, *clim;                  /* old, new line ptr limits */
int     chsize;

    for (r1 = crow, c1 = ccol;          /* start at current row/col     */
	 r1 <= maxr ;                   /* until last line of field     */
	 r1++ , c1 = mincx )            /* on each line start at left   */
	markchg(r1,c1,maxc);            /* ...mark changed regions      */

				/* Shift data in field, flowing upward. */
    r1 = crow;
    ocp = &pw->_y[r1][ccol];            /* start copy to current position */
    oap = &pw->_a[r1][ccol];
    clim = oclim = &pw->_y[r1][maxc];   /* limit on current line */

#ifdef  NLS
    if ( *ocp >= 0x1c80 )               /* if deleted char is 2 bytes   */
      --shifts ;
#endif

    chsize = 1;
#ifdef  KJI
    if (is_first_byte(*ocp))            /* if deleted char is 2 bytes   */
	    chsize = 2;
#endif
    cp = ocp+chsize;                    /* set source of copy */
    ap = oap+chsize;

    if (cp>clim) {                      /* may need initial line wrap */
	    register int d = mincx+cp-clim-1;
	    r1++;
	    cp = &pw->_y[r1][d];
	    ap = &pw->_a[r1][d];
	    clim = &pw->_y[r1][maxc];
    }
    while (r1<=maxr) {
	    *ocp++ = *cp++;
	    *oap++ = *ap++;
	    if (ocp>oclim) {            /* wrap old (dest) pointer? */
		    ocp = &pw->_y[r1][mincx];
		    oap = &pw->_a[r1][mincx];
		    oclim = clim;       /* (it wraps to current line) */
	    }
	    if (cp>clim) {              /* wrap new (source) pointer? */
		    r1++;
		    clim = &pw->_y[r1][maxc];
		    cp = &pw->_y[r1][mincx];
		    ap = &pw->_a[r1][mincx];
	    }
    }
#ifdef  KJI
    if (chsize==2) {                    /* filling two slots? */
	    *ocp++ = ECFILLC;
	    *oap++ = pw->_csbp;
    }
#endif
    *ocp = ECFILLC;
    *oap = pw->_csbp;
					/* now adjust output buffer */
    for (ocp = &psbuff[crow*widthx + ccol],
	cp = ocp+chsize,
	clim = &psbuff[maxr*widthx + maxc];
	cp <= clim ; )
	    *ocp++ = *cp++;
#ifdef  KJI
    if (chsize==2)
	    *ocp++ = '\0';
#endif
    *ocp = '\0';                        /* end buffer with null */

}                                       /* end function ecflind         */

/*
 * NAME:                updtmsk
 *
 * FUNCTION: combine the mask specified by the third and fourth parameters
 *      with the mask specified by the second parameter based on the
 *      first parameter.
 */

int     updtmsk (sw, cm, sm, ss)/* update the mask              */
int    *sw;			/* switch set/or/cand           */
register
char   *cm;			/* mask array to be updated     */
register
char   *sm;			/* mask array to supply updates */
int     ss;			/* size of array sm must not be */
				/* - larger than cm             */

{

    register
    char   *me;			/* work pointer to test end     */
    int     fc;			/* fill count for set and cand  */

    me = sm + ss;		/* calculate end of set mask    */
#ifdef  NLS
    fc = (KEY_MAXNLS / 8) - ss;	/* calculate fill count         */
				/* - assumes mask is 512 bits   */
#else
    fc = (KEY_MAXC / 8) - ss;   /* calculate fill count         */
#endif

    switch (*sw) {
	case swset: 		/* case is set to new mask      */
	    while (sm < me)
		*cm++ = *sm++;	/* move data                    */
	    while ((fc--) > 0)
		*cm++ = '\0';	/* fill as needed               */
	    *sw = swor;		/* after set next is or'ed in   */
	    break;
	case swor: 		/* make additional char valid   */
	    while (sm < me)
		*cm++ |= *sm++;	/* or in new mask characters    */
	    break;
	case swcand: 		/* make characters invalid      */
	    while (sm < me)
		*cm++ &= ~(*sm++);/* remove bits from mask        */
	    break;
	default: 
	    break;
    }

    return 0;			/* return to caller             */
}


/*
 * NAME:                decr_max_right
 *
 * FUNCTION: Local function to decrement position of rightmost char.
 */

int     decr_max_right () {

	    NLSCHAR *ptr;

    if (max_right)		/* max_right at minimum value   */
	--max_right;
    else
	return;

    ptr = &nlbuff[max_right];

    while (!(*ptr > 0x0020) && (ptr > nlbuff)) {
	--ptr;
	--max_right;
    }

    return;
}


/*
 * NAME:                get_row_col_from_index
 *
 * FUNCTION: Local function to calculate row & col from buffer index.
 */

get_row_col_from_index(row, col, index)
int    *row,
       *col,
        index;

{

    int     i,
            k;

    i = index % widthx;
    k = index / widthx;
    if (i) {
	*row = k;
	*col = i - 1;
    }
    else {
	*row = k - 1;
	*col = widthx - 1;
    }
    *row += minrx;
    *col += mincx;

}
