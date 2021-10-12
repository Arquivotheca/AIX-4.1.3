static char sccsid[] = "@(#)23	1.14  src/bos/usr/ccs/lib/libcur/curses.c, libcur, bos411, 9428A410j 6/10/91 18:37:13";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: curses.c
 *
 * ORIGINS: 10, 27
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

/*
 * NAME:                curses.c
 *
 * FUNCTION: This file contains the global variable
 *      definitions used by the other curses routines.
 *
 */

#include        "cur00.h"

 /* boolean terminal capabilites from /etc/termcap */
char    AM,
        BS,
        BW,
        DA,
        DB,
        EO,
        ES,
        HS,
        HZ,
        IN,
        KM,
        MI,
        MS,
        NC,
        OS,
        UL,
        XN;

 /* numeric terminal capabilites from /etc/termcap */
int     CO,
        IT,
        LI,
        LM,
        SG,
        PB,
        VT,
        WS;

 /* string terminal capabilities from /etc/termcap */
char   *AL,
       *AS,
       *AE,
       *BC,
       *BL,
       *BT,
       *CD,
       *CE,
       *CH,
       *CL,
       *CM,
       *CR,
       *CS,
       *CV,
       *DC,
       *DL,
       *DM,
       *DO,
       *xDO,
       *DS,
       *EC,
       *ED,
       *EI,
       *FS,
       *HO,
       *IC,
       *IM,
       *IP,
       *K0,
       *K1,
       *K2,
       *K3,
       *K4,
       *K5,
       *K6,
       *K7,
       *K8,
       *K9,
       *K10,
       *KS1,
       *KS2,
       *KS3,
       *KS4,
       *KS5,
       *KS6,
       *KS7,
       *KS8,
       *KS9,
       *KS10,
       *KDO,
       *KQUIT,
       *KCMD,
       *KPCMD,
       *KNPN,
       *KPPN,
       *KCPN,
       *KHLP,
       *KSEL,
       *KSCL,
       *KSCR,
       *KTAB,
       *KBTAB,
       *KNL,
       *KEND,
       *KACT,
       *KA,
       *Ka,
       *KB,
       *KC,
       *KA1,
       *KA3,
       *KB2,
       *KC1,
       *KC3,
       *KD,
       *Kd,
       *KE,
       *Ke,
       *KF,
       *KH,
       *Kh,
       *KI,
       *KL,
       *Kl,
       *Km,
       *KN,
       *KO,
       *KP,
       *KR,
       *Kr,
       *KS,
       *Ks,
       *KT,
       *Kt,
       *Ku,
       *L0,
       *L1,
       *L2,
       *L3,
       *L4,
       *L5,
       *L6,
       *L7,
       *L8,
       *L9,
       *L10,
       *LE,
       *xLE,
       *LL,
       *MA,
       *MB,
       *MD,
       *ME,
       *MH,
       *MK,
       *MM,
       *MO,
       *MP,
       *MR,
       *ND,
       *NL,
        PC,
       *xRI,
       *SE,
       *SF,
       *SO,
       *SR,
       *TA,
       *TE,
       *TI,
       *TS,
       *UC,
       *UE,
       *UP,
       *xUP,
       *US,
       *TP,
       *BM,
       *RV,
       *LV,
       *VB,
       *VE,
       *VI,
       *VS,
       *BX,
       *BY,
       *Bx,
       *By,
       *CF[8],
       *CB[8],
       *FO[8],
       *ZA,
       *K11,
       *K12,
       *K13,
       *K14,
       *K15,
       *K16,
       *K17,
       *K18,
       *K19,
       *K20,
       *K21,
       *K22,
       *K23,
       *K24,
       *K25,
       *K26,
       *K27,
       *K28,
       *K29,
       *K30,
       *K31,
       *K32,
       *K33,
       *K34,
       *K35,
       *K36,
       *K37,
       *K38,
       *K39,
       *K40,
       *K41,
       *K42,
       *K43,
       *K44,
       *K45,
       *K46,
       *K47,
       *K48,
       *K49,
       *K50,
       *K51,
       *K52,
       *K53,
       *K54,
       *K55,
       *K56,
       *K57,
       *K58,
       *K59,
       *K60,
       *K61,
       *K62,
       *K63;

 /* the following pairs of strings provide a mapping from          */
char				/* keyboard string to sequence of return
				   codes - see keypad       */
       *KEXT_S1,
       *KEXT_S2,
       *KEXT_S3,
       *KEXT_S4,
       *KEXT_S5,
       *KEXT_R1,
       *KEXT_R2,
       *KEXT_R3,
       *KEXT_R4,
       *KEXT_R5,
       *KEXT_S6,
       *KEXT_S7,
       *KEXT_S8,
       *KEXT_S9,
       *KEXT_R6,
       *KEXT_R7,
       *KEXT_R8,
       *KEXT_R9;


int				/* these are the global variable which
				   will be used to specify attrs */
        NORMAL,
        REVERSE,
        BOLD,
        BLINK,
        UNDERSCORE,
        TOPLINE,
        BOTTOMLINE,
        RIGHTLINE,
        LEFTLINE,     
        DIM,
        INVISIBLE,
        PROTECTED,
        STANDOUT,
        F_BLACK,
        F_RED,
        F_GREEN,
        F_BROWN,
        F_BLUE,
        F_MAGENTA,
        F_CYAN,
        F_WHITE,
        B_BLACK,
        B_RED,
        B_GREEN,
        B_BROWN,
        B_BLUE,
        B_MAGENTA,
        B_CYAN,
        B_WHITE,
        FONT0,
        FONT1,
        FONT2,
        FONT3,
        FONT4,
        FONT5,
        FONT6,
        FONT7;


char    CA,
        GT,
        NONL,
        UPPERCASE,
        normtty,
        _pfast;

ATTR    sw_mask = '\0';         /* mask for all switch type attrubutes */
int     Bxa,
        Bya;			/* mask for attrs named in Bx & By     */

char    _echoit = TRUE,		/* set if stty indicates ECHO          */
        _rawmode = FALSE,	/* set if stty indicates RAW mode      */
        My_term = FALSE,	/* set if user specifies terminal type */
	_endwin = FALSE,        /* set if endwin has been called       */
	_noesckey = FALSE;      /* set if needs to map more esc key    */

char    _keypad = FALSE;	/* true if input is to be scanned for  */
				/* strings defined for keypad codes    */

char    _nodelay = FALSE;	/* true if read should not wait for    */
				/* data but return a code if none ready */

char    _curwin = FALSE;	/* flag: are we dealing with curscr? */

char   *Def_term = "unknown";	/* default terminal type        */

int     _tty_ch = 1,		/* file channel which is a tty          */
        LINES,			/* number of lines allowed on screen    */
        COLS,			/* number of columns allowed on screen  */
        EC_cost = 500,		/* init cost of erase_chars capability  */
        CE_cost = 500;		/* init cost of clear to eol capability */

WINDOW  *stdscr = NULL,         /* the default standard screen          */
	*_win = NULL,           /* shared info between refresh() & mvcur()*/
	*curscr = NULL;         /* curses' idea of what's on the glass  */

short   _ly = 0,		/* where curses thinks the cursor is now 
				*/
        _lx = 0;

SGTTY _tty;			/* tty modes                            */
SGTTY _res_flg;			/* sgtty flags for reseting later       */
SGTTY _ctty;			/* curses ttymode flags                 */

int     _fcntl;			/* application nodelay mode             */
int     _cfcntl;		/* curses nodelay mode save             */

char    ESCSTR[128];		/* hold escape sequence from keypad     */

char    _mtkeybf = TRUE;	/* True if keyboard read buffer is empty 
				*/
				/* after call to keych                  */

char    _trackloc = FALSE;      /* default is not to track locator.     */
				/* else, make TRUE for appl to do       */
				/* locator tracking; accessed in ecpnin */

char    _extended = TRUE;	/* default is combine shifts and data   */
				/* codes on input; used only in _keych  */
				/* and in ecflin()                      */

char    SHOW_LOC = TRUE;	/* for tracking locator while */
				/* running within an xterm window       */

char    _dounctrl = FALSE;	/* support printing of control */
				/* as printable chars in addch().       */
				/* use unctrl array for printable ctrls */

char    do_colors = FALSE;	/* if do_colors is true, the */
				/* screen colors will be saved in       */
				/* initscr() and restored in endwin()   */
char    do_cursor = TRUE;       /* If do_cursor is false, the cursor    */
				/* shape will not be set to normal (i.e.*/
				/* underscor)                           */
