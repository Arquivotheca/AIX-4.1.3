static char sccsid[] = "@(#)11	1.21  src/bos/usr/ccs/lib/libcur/keypad.c, libcur, bos411, 9428A410j 4/28/93 17:20:31";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: keych, convex, keypad, extended, flttyin
 *
 * ORIGINS: 3, 10, 27
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

#include <sys/time.h>
#include        <errno.h>
#include        "cur99.h"
#include        "cur02.h"

#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>

/*      global entry point pointer                                      */
int     _keych ();

int     (*_KEY_ACC)() = _keych;	/* transfer address for real    */
				/* keyboard access routine      */

/*      global flag for local buffer state - to be used to determine    */
/*      if there is more data waiting for processing                    */
extern
char    _mtkeybf;		/* True if buffer is empty      */
extern
char    _extended;		/* True if shift codes are to   */
				/* combined with data on input  */

/*      static local storage areas                                      */

static
unsigned char   buffer[200];	/* local buffer area            */
static
unsigned char  *nxt = {
    buffer
};				/* next available character     */
static
unsigned char  *freep = {
    buffer
};				/* first unused char in buffer  */

static
char   *carry_ov = {
    NULL
};				/* carryover keycode pointer    */

static
struct {
    char  **ptr;		/* address of pointer to string */
    int     code;		/* code for string if found     */
}       KP_tb[] = {

/*  The following 9 entries provide the root for a limited customize    */
/*  keyboard function. The key identified by the termcap strings        */
/*  Kv - Kz and pointed to by KEXT_S1 - S5 can be mapped to a seqeunce  */
/*  of returns from this function. The return codes are specified by    */
/*  termcap strings KV-KZ and are pointed to by the externs KEXTR1-R5   */
/*  the second entry here is an index into the table exttab used to     */
/*  access the return code strings.                                     */

    {
	       &KEXT_S1, -1
    }      ,			/* customized key 1             */
    {
	       &KEXT_S2, -2
    }      ,			/* customized key 2             */
    {
	       &KEXT_S3, -3
    }      ,			/* customized key 3             */
    {
	       &KEXT_S4, -4
    }      ,			/* customized key 4             */
    {
	       &KEXT_S5, -5
    }      ,			/* customized key 5             */
    {
	       &KEXT_S6, -6
    }      ,			/* customized key 6             */
    {
	       &KEXT_S7, -7
    }      ,			/* customized key 7             */
    {
	       &KEXT_S8, -8
    }      ,			/* customized key 8             */
    {
	       &KEXT_S9, -9
    }      ,			/* customized key 9             */

/* The following set of key definitions for the usability keys should */
/* remain in the table ahead of the function keys. When the usability */
/* keys are defined in the Termcap entry they will be recognized then */
/* even if they are the same as some of the function keys (f1-f9)     */
/* Specific termcap file entries are present to define these keys and */
/* then 'chain' to the normal full terminal description.              */

    {
	       &KDO, KEY_DO
    }      ,			/* DO key                       */
    {
	       &KQUIT, KEY_QUIT
    }      ,			/* QUIT key                     */
    {
	       &KCMD, KEY_CMD
    }      ,			/* COMMAND  key                 */
    {
	       &KPCMD, KEY_PCMD
    }      ,			/* Previous command key         */
    {
	       &KNPN, KEY_NPN
    }      ,			/* next pane key                */
    {
	       &KPPN, KEY_PPN
    }      ,			/* previous pane key            */
    {
	       &KCPN, KEY_CPN
    }      ,			/* command pane key             */
    {
	       &KHLP, KEY_HLP
    }      ,			/* Help key                     */
    {
	       &KSEL, KEY_SEL
    }      ,			/* select key                   */
    {
	       &KSCL, KEY_SCL
    }      ,			/* Scroll left                  */
    {
	       &KSCR, KEY_SCR
    }      ,			/* Scroll right                 */
    {
	       &KEND, KEY_END
    }      ,			/* end key                      */
    {
	       &KACT, KEY_ACT
    }      ,			/* action key                   */

    {
	       &KTAB, KEY_TAB
    }      ,			/* tab character                */
    {
	       &KBTAB, KEY_BTAB
    }      ,			/* back tab character           */
    {
	       &KNL, KEY_NEWL
    }      ,			/* new line character           */
    {
	       &Kd, KEY_DOWN
    }      ,			/* cursor down                  */
    {
	       &Ku, KEY_UP
    }      ,			/* cursor up                    */
    {
	       &Kl, KEY_LEFT
    }      ,			/* cursor left                  */
    {
	       &Kr, KEY_RIGHT
    }      ,			/* cursor right                 */
    {
	       &Kh, KEY_HOME
    }      ,			/* home - top left              */
    {
	       &KB, KEY_BACKSPACE
    }      ,			/* backspace - unreliable       */
    {
	       &K0, KEY_F0
    }      ,			/* function key - 100 values    */
    {
	       &K1, KEY_F (1)
    }      ,			/*    reserved, 37 supported    */
    {
	       &K2, KEY_F (2)
    }      ,			/*                              */
    {
	       &K3, KEY_F (3)
    }      ,			/*                              */
    {
	       &K4, KEY_F (4)
    }      ,			/*                              */
    {
	       &K5, KEY_F (5)
    }      ,			/*                              */
    {
	       &K6, KEY_F (6)
    }      ,			/*                              */
    {
	       &K7, KEY_F (7)
    }      ,			/*                              */
    {
	       &K8, KEY_F (8)
    }      ,			/*                              */
    {
	       &K9, KEY_F (9)
    }      ,			/*                              */
    {
	       &K10, KEY_F (10)
    }      ,			/*                              */
    {
	       &K11, KEY_F (11)
    }      ,			/*                              */
    {
	       &K12, KEY_F (12)
    }      ,			/*                              */
    {
	       &K13, KEY_F (13)
    }      ,			/*                              */
    {
	       &K14, KEY_F (14)
    }      ,			/*                              */
    {
	       &K15, KEY_F (15)
    }      ,			/*                              */
    {
	       &K16, KEY_F (16)
    }      ,			/*                              */
    {
	       &K17, KEY_F (17)
    }      ,			/*                              */
    {
	       &K18, KEY_F (18)
    }      ,			/*                              */
    {
	       &K19, KEY_F (19)
    }      ,			/*                              */
    {
	       &K20, KEY_F (20)
    }      ,			/*                              */
    {
	       &K21, KEY_F (21)
    }      ,			/*                              */
    {
	       &K22, KEY_F (22)
    }      ,			/*                              */
    {
	       &K23, KEY_F (23)
    }      ,			/*                              */
    {
	       &K24, KEY_F (24)
    }      ,			/*                              */
    {
	       &K25, KEY_F (25)
    }      ,			/*                              */
    {
	       &K26, KEY_F (26)
    }      ,			/*                              */
    {
	       &K27, KEY_F (27)
    }      ,			/*                              */
    {
	       &K28, KEY_F (28)
    }      ,			/*                              */
    {
	       &K29, KEY_F (29)
    }      ,			/*                              */
    {
	       &K30, KEY_F (30)
    }      ,			/*                              */
    {
	       &K31, KEY_F (31)
    }      ,			/*                              */
    {
	       &K32, KEY_F (32)
    }      ,			/*                              */
    {
	       &K33, KEY_F (33)
    }      ,			/*                              */
    {
	       &K34, KEY_F (34)
    }      ,			/*                              */
    {
	       &K35, KEY_F (35)
    }      ,			/*                              */
    {
	       &K36, KEY_F (36)
    }      ,			/*                              */
    { &K37, KEY_F (37) },
    { &K38, KEY_F (38) },
    { &K39, KEY_F (39) },
    { &K40, KEY_F (40) },
    { &K41, KEY_F (41) },
    { &K42, KEY_F (42) },
    { &K43, KEY_F (43) },
    { &K44, KEY_F (44) },
    { &K45, KEY_F (45) },
    { &K46, KEY_F (46) },
    { &K47, KEY_F (47) },
    { &K48, KEY_F (48) },
    { &K49, KEY_F (49) },
    { &K50, KEY_F (50) },
    { &K51, KEY_F (51) },
    { &K52, KEY_F (52) },
    { &K53, KEY_F (53) },
    { &K54, KEY_F (54) },
    { &K55, KEY_F (55) },
    { &K56, KEY_F (56) },
    { &K57, KEY_F (57) },
    { &K58, KEY_F (58) },
    { &K59, KEY_F (59) },
    { &K60, KEY_F (60) },
    { &K61, KEY_F (61) },
    { &K62, KEY_F (62) },
    { &K63, KEY_F (63) },
    {
	       &KL, KEY_DL
    }      ,			/* delete line                  */
    {
	       &KA, KEY_IL
    }      ,			/* insert line                  */
    {
	       &KD, KEY_DC
    }      ,			/* delete character             */
    {
	       &KI, KEY_IC
    }      ,			/* insert character mode start  */
    {
	       &Km, KEY_EIC
    }      ,			/* exit insert character mode   */
    {
	       &KC, KEY_CLEAR
    }      ,			/* clear screen                 */
    {
	       &KS, KEY_EOS
    }      ,			/* clear to end of screen       */
    {
	       &KE, KEY_EOL
    }      ,			/* clear to end of line         */
    {
	       &KF, KEY_SF
    }      ,			/* scroll forward               */
    {
	       &KR, KEY_SR
    }      ,			/* scroll backward (reverse)    */
    {
	       &KN, KEY_NPAGE
    }      ,			/* next page                    */
    {
	       &KP, KEY_PPAGE
    }      ,			/* previous page                */
    {
	       &KT, KEY_STAB
    }      ,			/* set tab stop                 */
    {
	       &Kt, KEY_CTAB
    }      ,			/* clear tab stop               */
    {
	       &Ka, KEY_CATAB
    }      ,			/* clear all tab stops          */
    {
	       &KH, KEY_LL
    }      ,			/* lower left (last line)       */

    {
	       &KS1, KEY_SF1
    }      ,			/* special function key 1       */
    {
	       &KS2, KEY_SF2
    }      ,			/* special function key 2       */
    {
	       &KS3, KEY_SF3
    }      ,			/* special function key 3       */
    {
	       &KS4, KEY_SF4
    }      ,			/* special function key 4       */
    {
	       &KS5, KEY_SF5
    }      ,			/* special function key 5       */
    {
	       &KS6, KEY_SF6
    }      ,			/* special function key 6       */
    {
	       &KS7, KEY_SF7
    }      ,			/* special function key 7       */
    {
	       &KS8, KEY_SF8
    }      ,			/* special function key 8       */
    {
	       &KS9, KEY_SF9
    }      ,			/* special function key 9       */
    {
	       &KS10, KEY_SF10
    }      ,			/* special function key 10      */


				/* assume the following layout  */
				/* of the keypad for the next 5 */
				/* definitions                  */
				/*                              */
				/*   A1     up     A3           */
				/*   left   B2     right        */
				/*   C1     down   C3           */

    {
	       &KA1, KEY_A1
    }      ,			/* pad top left                 */
    {
	       &KA3, KEY_A3
    }      ,			/* pad top right                */
    {
	       &KB2, KEY_B2
    }      ,			/* pad center                   */
    {
	       &KC1, KEY_C1
    }      ,			/* pad lower left               */
    {
	       &KC3, KEY_C3
    }      ,			/* pad lower right              */
    {
	        (char **) 0, 0
    }
};				/* null terminator for table    */

/*  The strings pointed to by the KEXT_Rx variables must be sequences   */
/*  of integers to be used as return codes when the corresponding key   */
/*  is pressed. The integers must be ASCII and must be separated by one */
/*  non-digit character with no such character after the last number.   */
/*  The numbers may be any valid integer value but if codes for escapes */
/*  are used, there will be no corresponding string in ESCSTR.          */

static
char  **exttab[] =
{
    &KEXT_R1,			/* string for customized key 1  */
    &KEXT_R2,			/* string for customized key 2  */
    &KEXT_R3,			/* string for customized key 3  */
    &KEXT_R4,			/* string for customized key 4  */
    &KEXT_R5,			/* string for customized key 5  */
    &KEXT_R6,			/* string for customized key 6  */
    &KEXT_R7,			/* string for customized key 7  */
    &KEXT_R8,			/* string for customized key 8  */
    &KEXT_R9
};				/* string for customized key 9  */



/*
 * NAME:                keych
 *
 * FUNCTION: character or keypad code, returned as integer, if no
 *      data and _nodelay, return  KEY_NOKEY
 */

int     keych () {		/* normal keych access point    */
            return ((*_KEY_ACC)());/* indirect access to routine   */
}


#define rdfdes  0		/* file descriptor for read     */

/*
 * The location of a valid ESC sequence uses the following defined
 * values to control the limits of the sequence. ESC (K_ESC) must be
 * the first character. If the second character is '[' (K_ESCBR) the
 * sequence is terminated by the first character in the range 0x30 -
 * 0x7f (K_ESCL2 to K_ESCU2) otherwise the second character must be in
 * the range of 0x40 - 0x7f (K_ESCL1 to K_ESCU1). The value returned
 * for the sequence is KEY_ESC1 (for short strings) or KEY_ESC2 (for
 * long strings) plus the value of the ending character.
 */

#define K_ESC   0x1b
#define K_ESCBR '['
#define K_ESCL1 0x30
#define K_ESCU1 0X7f
#define K_ESCL2 0x40
#define K_ESCU2 0x7f

/*
 * adjusting this value sets wait time when a partial key string is
 * found in the input
 */
#define max_reads 500

static int _keych ()
{



    static  int     timesToRead = 0;	/* times to read                */
    int     tix;                        /* index into keypad ptr table  */
    int     rc;                         /* working return code value    */
    short   rtcode;                     /* function return code         */
    int     limit;                      /* limit counter                */
    char    *wp;                        /* scratch pointer to buffer    */
    char    *escDelay;                  /* ENV var. ESCDELAY            */

    unsigned char  *kp;                 /* work pointer to keypad str   */
    unsigned char  *p;                  /* work pointer to input string */

    char    saved = FALSE;              /* saved status for _extended   */
    char    extnd_save;                 /* save _extended here          */

    char    wasdelay = FALSE;           /* track original nodelay state */
    int     mbl = 0;			/* if _extended, this gets set  */
					/* the length of char found     */

    if (carry_ov != 0) {                /* if carryover code set         */
	rtcode = convex();              /* get pseudo return code        */
	goto got_code;                  /* go process that code          */
    }                                   /*                               */

    if (nxt >= freep) {                 /* if there is no data in buffer */
	nxt = freep = buffer;		/* reset pointers to buffer strt */
	rc = _ndelay_read(rdfdes, nxt, sizeof(buffer));
	if (rc < 0)
	    if (errno == EAGAIN || errno == EWOULDBLOCK)
		rc = 0;
	    else
		return(rc);		/* if error on read return now  */
	freep += rc;			/* step free space pointer      */
    }

    if (nxt >= freep)
	return(KEY_NOKEY);	/* if nodata return no key code */

    /* process data that is in buffer */
if (_keypad) {              /* if keypad processing active then
				   consider table data     */

	if (!timesToRead)               /* if first time here.          */
					/* if ESCDELAY exists.          */
	    if (escDelay = getenv ("ESCDELAY"))
					/* convert it to numeric.       */
		timesToRead = atoi (escDelay);
	    else
		timesToRead = max_reads ;

	limit = timesToRead;    /* set upper limit on reads     */
				/* this limit acts as a time    */
				/* restriction and will force   */
				/* the data to be returned after */
				/* a short time. This is used   */
				/* instead of sleep to provide  */
				/* better tunability.           */

	for (tix = 0;		/* for each defined keypad str  */
		KP_tb[tix].ptr != NULL;
				/* until end of table (null ptr) */
		tix++) {	/* step through table           */
	    if ((kp = (unsigned char *) * KP_tb[tix].ptr) != NULL) {
				/* if keypad string is defined  */
		for (p = nxt;;) {/* compare to input data        */
		/* only loop exit is explicit   */
		    if (*kp == '\0') {
				/* if at end of keypad string have
				   match, exit with code   */
			rtcode = KP_tb[tix].code;
				/* extract code from table      */

			if (rtcode < 0) {
				/* if returning custom key      */
			    carry_ov = *exttab[-rtcode - 1];
				/* get pointer to custom retrns */
			    if (carry_ov == NULL) {
				/* if no returns specified      */
				goto kp_nul_x;
				/* go process as not keypad     */
			    }
			    rtcode = convex();
				/* get return code              */
			    goto end_kp_srch;
				/* go process - code found      */
			}
			goto end_kp_srch;
				/* exit from the both loops     */
		    }		/* end match found              */

		    while (p >= freep) {
				/* if at end of input data have
				   prefix - get more data  */
			if (*nxt == K_ESC) {
				/* only timeout if ESC as start */
			    if (--limit <= 0)
				/* if limit reached (timeout)   */
				goto kp_nomatch;
				/* nomatch in keypads - exit    */

			    if (!_nodelay) {
				/* if delay mode is active      */
				wasdelay = TRUE;
				/* remember old state           */
				nodelay(TRUE);
				/* set state to no delay        */
			    }
			}

			rc = _ndelay_read(rdfdes, freep, sizeof(buffer) -
						(int)(freep - nxt));
				/* read more data from terminal */
			if (wasdelay)
			    nodelay(FALSE);
			       /* if needed reset delay flag   */
			if (rc < 0)
			    if (errno == EAGAIN || errno == EWOULDBLOCK)
				rc = 0;
				/* if error on read return now  */
			    else return(rc);
			freep += rc;/* else adjust end of data ptr  */
		    }		/* end - get more input data    */

		    if (*p++ != *kp++)
			break;	/* if mismatch - exit, try next */
		}		/* end input data compare       */
	    }			/* end - keypad string defined  */
	}			/* end loop through table       */

kp_nul_x: 			/* transfer here if null return */
				/* - string for customized key  */

				/* check input for ESC sequence */

	p = nxt;		/* init pointer to buffer       */
	if (*p++ == K_ESC) {	/* if first char is ESC ck more */
				/* search for complete ESC str. */
				/* see elplanation above for    */
				/* valid form and return value  */

	    unsigned char  *p2;	/* pointer to char after ESC    */
	    int     rtbase;	/* base value for return code   */
	    unsigned char   llimit;/* lower bound for ender char   */


	    p2 = p;		/* init pointer to 2nd char     */
	    rtbase = KEY_ESC1;	/* init return base for ESC x   */
				/* or ESC y x with 0x20<=y<=0x2f */
	    llimit = K_ESCL1;	/* set lower limit for same case */

	    if (!_noesckey) {   /* if noesck processing active  */
				/* then consider each other entry */
				/* as a single event            */

		for (;; p++) {  /* repeat until explicit exit, step
				   ptr if not ending char  */

		    while (p >= freep) {
				/* if at end of input data have
				   prefix - get more data  */
				/* if limit reached (timeout)   */
			if (--limit <= 0)
				/* not completed ESC  - exit    */
			    goto kp_nomatch;
				/* if delay mode is active      */
			if (!_nodelay) {
				/* remember old state           */
			    wasdelay = TRUE;
				/* set state to no delay        */
			    nodelay(TRUE);
			}

				/* read more data from terminal */
			rc = _ndelay_read(rdfdes, freep, sizeof(buffer) -
						(int)(freep - nxt));
			if (wasdelay)
				/* if needed reset delay flag   */
			    nodelay(FALSE);
			if (rc < 0)
			    if (errno == EAGAIN || errno == EWOULDBLOCK)
				rc = 0;
				/* if error on read return now  */
			    else return(rc);
				/* else adjust end of data ptr  */
			freep += rc;
		    }           /* end - get more input data    */
				/* if this is char after ESC    */
		    if (p == p2 &&
				/* and that char is [           */
			*p == K_ESCBR) {
				/* change base for return code  */
			rtbase = KEY_ESC2;
				/* change lower limit           */
			llimit = K_ESCL2;
		    }           /* not end, continue read       */
		    else {      /* either past 2nd char or it is
				   not [, look for ender      */
				/* if char is in ender range    */
			if (*p >= llimit &&
			    *p <= K_ESCU1) {
				/* calculate return code        */
			    rtcode = rtbase + *p++;
				/* move the string              */
			    for (wp = ESCSTR; nxt < p;) {
				/* move a character at a time   */
				*wp++ = *nxt++;
			    }   /* put in a null marker         */
			    *wp = '\0';
				/* go to return                 */
			    goto end_kp_srch;
			}       /* end - if ender character     */
		    }           /* end - check for ender        */
		}               /* end - search for ESC end     */
				/* -- step ptr and get next char */
	    }                   /* end - if noesck processing is*/
				/* -- active                    */
	}			/* end - if first is ESC        */
    }				/* end - if keypad is active    */

kp_nomatch: 			/* exit from loop either with   */
				/* mismatch on all or limit     */

    p = nxt + 1;		/* force pointer for single char */
    rtcode = *nxt;		/* return code is character val */

    if (_extended) { /* generate S-JIS if _extended on */
	if (mblen(nxt, MB_CUR_MAX) != 1) { /* if first byte of two byte character */
	    if (p == freep) {           /* end of input */
		nxt = freep = buffer ;
		rc = _ndelay_read(rdfdes,nxt,sizeof(buffer));
		if (rc < 0)
		    if (errno == EAGAIN || errno == EWOULDBLOCK)
			rc = 0;
		    else
			return(rc);
		freep += rc ;       /* step free space pointer */
		p = nxt ;
	    }

	    if (freep > nxt) {          /* if data was read */
		char c[8];
		int  ci;

		c[0] = rtcode;
		c[1] = *p++;
		c[2] = '\0';
		ci = 2;
		while (((mbl = mblen(c, MB_CUR_MAX)) <= 0) &&
				(ci < MB_CUR_MAX)){
			c[ci++] = *p++;
			c[ci] = '\0';
		}
		if (mbl > 0) {
			mbtowc (&rtcode, c, MB_CUR_MAX);
		} else {
			p -= (ci - 1);
		}
	    } else {                    /* _nodelay must be set */
		*freep++ = rtcode; /* save away the first byte of two byte */
		return (KEY_NOKEY) ; /* character and return no key code */
	    }
	}
    }

end_kp_srch: 			/* search done, p indicates char */
				/* after string, rtcode is code */
				/* to return for string or char */

    nxt = buffer;		/* shift data back in buffer    */
    while (p < freep) {		/* loop through remaining data  */
	*nxt++ = *p++;
    }				/* shifting characters to start */
    freep = nxt;		/* correct limits pointers      */
    nxt = buffer;		/*                              */

/*      If code indicates that binary data follows, read that data into */
/*      the hold area for the escape string                             */

    if ( mbl <= 1 ) {
	switch (rtcode) {                   /* test return code for possible */
					    /* - binary data                */
		int     ix,
			binlen;             /* work integers for switch     */
		char    ch;                 /* work character code          */
		char   *plm;                /* work character pointer       */


	    case KEY_VTDESC:                /* VTD escape sequence          */
		binlen = 0;                 /* clear length value           */
		keypad(FALSE);              /* turn off translate of input  */
		extnd_save = _extended;     /* save _extended               */
		saved = TRUE;               /* are saving _extended value   */
		_extended = FALSE;          /* turn off extended processing */
		for (ix = 0; ix < KEY_VTDl; ix++) {
										    /* get length field value       */
			    ch = keych();   /* get next byte                */
			    binlen = binlen << 8 + ch;
										    /* accumulate length code       */
			    *wp++ = ch;     /* add byte to ESC hold area    */
		}
		binlen -= KEY_VTDl;         /* adjust length for size of    */
					    /* - length field itself        */
		goto get_bin;               /* go get rest of data          */

	    case KEY_LOCESC:                /* Locator escape sequence      */
		binlen = KEY_LOCl;          /* locator data size            */
		goto get_bin;               /* go get rest of data          */

	    case KEY_NXKESC:                /* Non-Translate key sequence   */
		binlen = KEY_NXKl;          /* key code length              */
get_bin:                                    /* begin to get rest of bin data */
		keypad(FALSE);              /* ensure translation off       */
		if (!saved) {               /* have not saved _extended     */
		    extnd_save = _extended; /* save _extended flag          */
		    saved = TRUE;
		    _extended = FALSE;      /* turn off extended processing */
		}
		plm = &ESCSTR[127];         /* set upper hold area limit    */
		for (ix = 0; ix < binlen; ix++) {
										    /* loop to get remainder of data */
		    ch = keych();           /* get next byte of data        */
		    if (wp < plm)
			*wp++ = ch;         /* move byte to hold area       */
		}
		keypad(TRUE);               /* re-establish translation     */
		_extended = extnd_save;     /* restore _extended flag       */
		break;

	    default:                        /* no binary data follows       */
		break;
	}                                   /* end switch on return code    */
    }


got_code:                       /* exit through here            */

    _mtkeybf = (nxt == freep);	/* set empty buffer flag        */

    return(rtcode);		/* return code as determined    */
}				/* end keypad()                 */


/*
 * NAME:                convex
 *
 * FUNCTION: returns next integer pointed to by carry_ov
 *      leaves carry_ov set to null if end of string reached
 *      else after the delimiter character.
 */

convex() {			/* convert customized strings   */

    int     rc = 0;		/* return code                  */
    char    ch;			/* next input character         */

    while (((ch = *carry_ov++) >= '0')/* continue while digits found  */
	    &&(ch <= '9')) {
	rc = rc * 10 + ch - '0';
    }				/* acumulate code for return    */
    if (ch == '\0')
	carry_ov = NULL;	/* at end string clear pointer  */
    return(rc);			/* return with number           */

}				/* end function convex          */

/*
 * NAME:                keypad
 *
 * FUNCTION: This routine reads data from the terminal
 *      if that data is a keypad function (defined in the termcap) then
 *      the corresponding code is returned to the caller. All such codes
 *      have a value greater than 0xff. Strings do not have to be escape
 *      sequences to be recognized.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Side effects - _keypad is set to reflect argument, will in turn
 *                     affect processing in keych()
 *
 * EXTERNAL REFERENCES: read(),
 *
 * DATA STRUCTURES:     local static data areas
 *
 * RETURNS =    normal -> <the character> in range 0 - 0xff
 *                        RELEASE 3 - NLS support :
 *                              in ranges  0x1c80 - 0x1cff
 *                                         0x1d80 - 0x1dff
 *                                         0x1e80 - 0x1eff
 *                                         0x1f80 - 0x1fff
 *                        <keypad code)   in range 0x100 to 0x7fff
 *              error  -> -1 if read error
 */

keypad(keysw)			/* function definition          */
char    keysw;			/* argument is boolean variable */

{
    _keypad = keysw;		/* set global switch            */
    if (_keypad) {		/* based on keypad switch       */
	_puts(Ks)
    }				/* activate if needed by device */
    else {
	_puts(Ke)
    }				/* else deactivate if needed    */
    return(OK);			/* return to caller             */
}				/* end of function              */


/*
 * NAME:                extended
 *
 * FUNCTION: _extended is set to reflect argument, will in
 *      turn affect processing in keych()
 */

extended(keysw)			/* function definition          */
char    keysw;			/* argument is boolean variable */

{
    _extended = keysw;		/* set global switch            */

    return(OK);			/* return to caller             */
}				/* end of function              */



/*
 * NAME:                flttyin
 *
 * FUNCTION: flush input from tty.
 */

flttyin() {			/* reset curses tty state       */
    int     rc;
    char    old_delay;		/* hold no-delay state on entry */
    int     (*old_keych)();	/* hold old vector ptr to keych */

    old_delay = _nodelay;	/* save old delay status        */
    old_keych = _KEY_ACC;	/* save old vector ptr to keych */

    nodelay(TRUE);		/* set no-delay mode            */
    _KEY_ACC = _keych;		/* set vector ptr to keych      */

    while ((rc = keych()) != KEY_NOKEY &&
	   !(rc < 0 && errno == EIO))
	continue;		/* read until no input pending  */

    _KEY_ACC = old_keych;       /* reset vector ptr to keych    */
    nodelay(old_delay);		/* reset delay state            */

}

/*
 * if _nodelay is set, we select on fd (and we assume that fd is less
 * than 32).  If the select returns 0, then there are no bytes to read
 * so we return 0.  Otherwise we do the normal read.
 */
static int _ndelay_read(int fd, char *buf, int len)
{
    static struct timeval ztime;	/* 0 time */
    static int rfd, wfd, xfd;		/* as portable as anything else */
    struct stat statbuf;

    /* since select does not fail on bad file descriptors, check it first */
    if (fstat(fd, &statbuf) < 0) return 0;
    rfd = 1 << fd;
    if (_nodelay && (select(32, &rfd, &wfd, &xfd, &ztime) <= 0))
	return 0;
    return read(fd, buf, len);
}
