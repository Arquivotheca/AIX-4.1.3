/* @(#)09	1.19  src/bos/usr/include/cur00.h, libcurses, bos411, 9428A410j 6/10/91 18:36:39 */
#ifndef _H_CUR00
#define _H_CUR00

/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: cur00.h
 *
 * ORIGINS: 3, 10, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:                cur00.h
 *
 * FUNCTION: This file contains or calls for the
 *      inclusion of all the defines and extern definitions needed by
 *      most Ecurses routines. This include may be used to replace the
 *      include of curses.h when converting from curses to Ecurses.
 */

#undef TS
#ifndef WINDOW

#include        "cur01.h"

/* The semi-colon at the end of this macro is an error which  */
/* cannot be corrected because of compatibility restrictions. */

#define _puts(s)        tputs(s, 0, eciopc);

#if (IS1|IS2|V7)
typedef struct sgttyb   SGTTY;
#else
typedef struct termio   SGTTY;
#endif


/* the following #ifndef will allow an application to suppress the      */
/* termcap variable names from this expansion to avoid conflicts and    */
/* reduce symbol table requirements                                     */

#ifndef NO_TCAP

/*
 * Capabilities from termcap
 */

extern				/* boolean terminal capabilites from
				   /etc/termcap */
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

extern				/* numeric terminal capabilites from
				   /etc/termcap */
int     CO,
        IT,
        LI,
        LM,
        SG,
        PB,
        VT,
        WS;

extern				/* string terminal capabilities from
				   /etc/termcap */
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


extern				/* the following pairs of strings provide
				   a mapping from	  */
char				/* keyboard string to sequence of return
				   codes - see keypad	  */
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

extern int  Bxa,
            Bya;		/* attr mask for attrs named in Bx & By   
				*/

#endif


/*
 * From the tty modes...
 */
extern char CA,
            GT,
            NONL,
            UPPERCASE,
            normtty,
            _pfast;

extern char My_term,
            _echoit,
            _rawmode,
	    _endwin,
	    _noesckey;          /* see if more esc key mapping is needed*/

extern char _keypad,
	    _nodelay,
	    _trackloc;          /* activate the mouse.                  */

extern char *Def_term;

extern int  _tty_ch,
            CE_cost,
            EC_cost;

extern  SGTTY _tty,
	    _res_flg,
	    _ctty;
extern int  _fcntl,
            _cfcntl;

#endif /* WINDOW */

#endif				/* _H_CUR00 */
