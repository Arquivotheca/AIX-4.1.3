static char sccsid[] = "@(#)75	1.10.1.4  src/bos/usr/lib/nls/loc/jim/jed/jedProc.c, libKJI, bos411, 9428A410j 2/10/94 07:24:43";
/*
 * COMPONENT_NAME : (LIBKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : jedProc
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#define	XK_KATAKANA
#include <X11/keysym.h>
#include "im.h"
#include "imP.h"
#include "jedexm.h"
#include "jed.h"
#include "jedint.h"

extern int  get_state();
extern void set_imode();

/*
 *	keysym classes
 */
#define	isspacekeysym(k)	((k) == XK_space)

#define	isspspacekeysym(k)	((k) == XK_KP_Space || (k) == XK_nobreakspace)

#define	isalphakeysym(k) \
	(XK_A <= (k) && (k) <= XK_Z || XK_a <= (k) && (k) <= XK_z)

#define	isnumkeysym(k)	(XK_0 <= (k) && (k) <= XK_9)

#define	isspnumkeysym(k)	(XK_KP_0 <= (k) && (k) <= XK_KP_9)

#define	ispunctkeysym(k) ( \
		XK_space < (k) && (k) <= XK_at || \
		XK_bracketleft <= (k) && (k) <= XK_grave || \
		XK_braceleft <= (k) && (k) <= XK_asciitilde \
	)

#define	issppunctkeysym(k)	( \
		(k) == XK_KP_Equal || \
		(k) == XK_KP_Multiply || \
		(k) == XK_KP_Add || \
		(k) == XK_KP_Separator || \
		(k) == XK_KP_Subtract || \
		(k) == XK_KP_Decimal || \
		(k) == XK_KP_Divide \
	)

#define	iskanakeysym(k)	(XK_kana_fullstop <= (k) && (k) <= XK_semivoicedsound)

#define	isspcharkeysym(k)	( \
		(k) == XK_yen || \
		(k) == XK_cent || \
		(k) == XK_overline || \
		(k) == XK_macron || \
		(k) == XK_sterling || \
		(k) == XK_notsign \
	)

#define	isasciikeysym(k)	(XK_space < (k) && (k) <= XK_asciitilde)

#define	ischarkeysym(k)	( \
		isspacekeysym(k) || \
		isasciikeysym(k) || \
		iskanakeysym(k) || \
		issppunctkeysym(k) || \
		isspspacekeysym(k) || \
		isspnumkeysym(k) || \
		isspcharkeysym(k) \
	)

#define	isfunckeysym(k)	(get_func_type(k) != KEY_OTHERS)

/*
 *	suppress beep if modifier key.
 */
#define	IsNoBeep(k)	(IsModifierKey(k))

/*
 *	convert special keysym to normal one.
 */
static unsigned int	mapspecial(unsigned int keysym)
{
	switch (keysym) {
	case XK_KP_Space:	keysym = XK_space; break;
	case XK_KP_Equal:	keysym = XK_equal; break;
	case XK_KP_Multiply:	keysym = XK_asterisk; break;
	case XK_KP_Add:		keysym = XK_plus; break;
	case XK_KP_Separator:	keysym = XK_comma; break;
	case XK_KP_Subtract:	keysym = XK_minus; break;
	case XK_KP_Decimal:	keysym = XK_period; break;
	case XK_KP_Divide:	keysym = XK_slash; break;
	case XK_KP_0:		keysym = XK_0; break;
	case XK_KP_1:		keysym = XK_1; break;
	case XK_KP_2:		keysym = XK_2; break;
	case XK_KP_3:		keysym = XK_3; break;
	case XK_KP_4:		keysym = XK_4; break;
	case XK_KP_5:		keysym = XK_5; break;
	case XK_KP_6:		keysym = XK_6; break;
	case XK_KP_7:		keysym = XK_7; break;
	case XK_KP_8:		keysym = XK_8; break;
	case XK_KP_9:		keysym = XK_9; break;
	case XK_nobreakspace:	keysym = XK_space; break;
	case XK_yen:		keysym = XK_backslash; break;
	case XK_overline:	keysym = XK_asciitilde; break;
	case XK_cent:		keysym = XK_space; break;
	case XK_sterling:	keysym = XK_space; break;
	case XK_notsign:	keysym = XK_space; break;
	case XK_macron:		keysym = XK_asciitilde; break;
	}
	return keysym;
}

/*
 *	definition of CodeMap table structure.
 *
 *	this table defines the relationship between keysyms and pseudo
 *	codes.  also the keysyms are categoried.
 */

typedef struct {
	unsigned int	keysym;		/* keysym */
	int	code;			/* pseudo code */
	int	type;			/* key type */
}       CodeMap;

static const CodeMap	codemap[] = {
	{ XK_Katakana,		PKATAKANA,	KEY_SHIFT },
	{ XK_Eisu_toggle,	PALPHANUM,	KEY_SHIFT },
	{ XK_Hiragana,		PHIRAGANA,	KEY_SHIFT },
	{ XK_Romaji,		PRKC,		KEY_SHIFT },
	{ XK_Zenkaku_Hankaku,	PSGLDBL,	KEY_SHIFT },
	{ XK_Kanji,		PCONVERT,	KEY_CONV },
	{ XK_Henkan,		PCONVERT,	KEY_CONV },
	{ XK_Muhenkan,		PNOCONV,	KEY_NOCONV },
	{ XK_BunsetsuYomi,	PYOMI,		KEY_NOCONV },
	{ XK_MaeKouho,		PPRECAND,	KEY_PRECAND },
	{ XK_ZenKouho,		PALLCAND,	KEY_ALLCAND },
	{ XK_KanjiBangou,	PKJNUM,		KEY_KJNUM },
	{ XK_HenkanMenu,	PCONVSW,	KEY_CONVSW },
	{ XK_Touroku,		PREGST,		KEY_REGST },
	{ XK_Return,		PENTER,		KEY_ENTER },
	{ XK_Linefeed,		PENTER,		KEY_ENTER },
	{ XK_Execute,		PENTER,		KEY_ENTER },
	{ XK_KP_Enter,		PENTER,		KEY_ENTER },
	{ XK_BackSpace,		PBACKSPACE,	KEY_BACKSP },
	{ XK_Left,		PCURLEFT,	KEY_CURSOR },
	{ XK_Right,		PCURRIGHT,	KEY_CURSOR },
	{ XK_LeftDouble,	PCURDLEFT,	KEY_CURSOR },
	{ XK_RightDouble,	PCURDRIGHT,	KEY_CURSOR },
	{ XK_LeftPhrase,	PCTRCURLEFT,	KEY_CURSOR },
	{ XK_RightPhrase,	PCTRCURRIGHT,	KEY_CURSOR },
	{ XK_End,		PEREOF,		KEY_ERASE },
	{ XK_ErInput,		PERINPUT,	KEY_ERASE },
	{ XK_Delete,		PDELETE,	KEY_ERASE },
	{ XK_Reset,		PRESET,		KEY_RESET },
	{ XK_Up,		PCURUP,		KEY_CURUP },
	{ XK_Down,		PCURDOWN,	KEY_CURDOWN },
};

#define CODEMAP_SIZE	(sizeof (codemap) / sizeof (codemap[0]))

static int	get_func_type(unsigned int keysym)
{
	int	i;

	for (i = 0; i < CODEMAP_SIZE; i++) {
		if (codemap[i].keysym == keysym)
			return codemap[i].type;
	}
	return KEY_OTHERS;
}

static char	get_pcode(unsigned int keysym)
{
	int	i;

	for (i = 0; i < CODEMAP_SIZE; i++) {
		if (codemap[i].keysym == keysym)
			return codemap[i].code;
	}
	return 0;
}

/*
 *	maptobase()
 *	Lookup imkeymap
 */
static unsigned int	maptobase(
	FEPCB *fepcb,
	IMKeymap *immap,
	unsigned int keysym,
	unsigned int state)
{
	if((state & LockMask) &&
		(fepcb->shift[0] == KATAKANA || fepcb->shift[0] == HIRAGANA) &&
				((keysym == XK_e) || (keysym == XK_z) ||
				(keysym == XK_E) || (keysym == XK_Z))){
			state &= ~LockMask;
	}

	_IMMapKeysym(immap, &keysym, &state);
	if (state)
		keysym = XK_VoidSymbol;
	return keysym;
}

/*-----------------------------------------------------------------------*
*  set_highlight()
*      This routine inspects conversion mode flag in FEPCB and sets
*      attribute data to aux attribute buff according to it.
*-----------------------------------------------------------------------*/

static void	set_highlight(FEPCB *fepcb)
{
	int	pos, len;
	char	**toindex;
	int	i;

	/* attribute table definition for one line format */
	/* the format of this table must correspond to string */
	/* definition made somewhere in this file */
	static short att_tbl[4][2] = {
		{ 14, 10 },		/* look ahead    */
		{ 26, 8 },		/* multi phrase  */
		{ 36, 10 },		/* single phrase */
		{ 48, 10 }		/* word          */
	};

        /* initialize all attribute first once */
	toindex = fepcb->auxbufa;
	for (i = 0; i < fepcb->auxacsz.itemnum; i++)
		(void)memset(*toindex++, 0, fepcb->auxacsz.itemsize);

        /* set new attribute values */
	if (fepcb->auxformat == KP_SHORTAUX) {
		pos = att_tbl[fepcb->tconvmode][0];
		len = att_tbl[fepcb->tconvmode][1];
		switch (fepcb->tconvmode) {
		case LOOKAHEAD:
			(void)memset(*fepcb->auxbufa + pos, KP_HL_REVERSE, len);
			break;
		case MPHRASE:
			(void)memset(*fepcb->auxbufa + pos, KP_HL_REVERSE, len);
			break;
		case SPHRASE:
			(void)memset(*fepcb->auxbufa + pos, KP_HL_REVERSE, len);
			break;
		case WORD:
			(void)memset(*fepcb->auxbufa + pos, KP_HL_REVERSE, len);
			break;
		}
        } 
	else {	/* long format aux area */
		(void)memset(*(fepcb->auxbufa + 1 + fepcb->tconvmode), 
			KP_HL_REVERSE, MCONVAUXLEN);
	}
	return;
}

/*-----------------------------------------------------------------------*
*  	set_convsw_state()
*       This routine sets convsw state to Aux buffer.
*-----------------------------------------------------------------------*/
static void	set_convsw_state(FEPCB *fepcb)
{
	char	**toindex;
	char	*from;
	int	i;

	/*
         * menu definition for one line format
	 * whenever following table is changed, also change
	 * corresponding attribute table defined within this file and
	 * size inforamtion defined in header file
	 */
	static  char    convswlist[] = {
		0x81, 0x40,
		/*conv.mode */
		0x95, 0xcf, 0x8a, 0xb7, 0x83, 0x82, 0x81, 0x5b, 0x83, 0x68,
		0x81, 0x40,
		/* 1. */
		0x82, 0x50, 0x81, 0x44,
		/* look-ahead multiple phrase */
		0x90, 0xe6, 0x93, 0xc7, 0x82, 0xdd,
		0x81, 0x40,
		/* 2. */
		0x82, 0x51, 0x81, 0x44,
		/* multiple phrase */
		0x88, 0xea, 0x8a, 0x87, 
		0x81, 0x40,
		/* 3. */
		0x82, 0x52, 0x81, 0x44,
		/* single phrase  */
		0x92, 0x50, 0x95, 0xb6, 0x90, 0xdf,
		0x81, 0x40,
		/* 4. */
		0x82, 0x53, 0x81, 0x44,
		/* word */
		0x95, 0xa1, 0x8d, 0x87, 0x8c, 0xea,
		0x81, 0x40
	};

	/*
	 * menu definition for multi line format
	 * whenever following table is changed, also change
	 * corresponding attribute table defined within this file and
	 * size inforamtion defined in header file
	 */
	static char mconvlist[] = {
		/* conversion mode */
		0x81, 0x40, 0x95, 0xcf, 0x8a, 0xb7, 0x83, 0x82, 0x81, 0x5b, 
		0x83, 0x68, 0x81, 0x40, 0x81, 0x40, 0x81, 0x40, 0x81, 0x40,
		/* look ahead */
		0x81, 0x40, 0x82, 0x50, 0x81, 0x44, 0x90, 0xe6, 0x93, 0xc7,   
		0x82, 0xdd, 0x98, 0x41, 0x95, 0xb6, 0x90, 0xdf, 0x81, 0x40,
		/* multiple phrase */
		0x81, 0x40, 0x82, 0x51, 0x81, 0x44, 0x88, 0xea, 0x8a, 0x87, 
		0x98, 0x41, 0x95, 0xb6, 0x90, 0xdf, 0x81, 0x40, 0x81, 0x40,
		/* single phrase */
		0x81, 0x40, 0x82, 0x52, 0x81, 0x44, 0x92, 0x50, 0x95, 0xb6, 
		0x90, 0xdf, 0x81, 0x40, 0x81, 0x40, 0x81, 0x40, 0x81, 0x40,
		/* word */
		0x81, 0x40, 0x82, 0x53, 0x81, 0x44, 0x95, 0xa1, 0x8d, 0x87, 
		0x8c, 0xea, 0x81, 0x40, 0x81, 0x40, 0x81, 0x40, 0x81, 0x40,
        };

	/*
	 * there are two formats of this menu while I know
	 * the first one is NOT use by JIMFEP Rel 1.0
	 */
	if (fepcb->auxformat == KP_SHORTAUX) {
		(void)memcpy(*fepcb->auxbufs, convswlist, MINAUX);
		fepcb->auxacsz.itemsize = MINAUX; 
		fepcb->auxacsz.itemnum = 1; 
	} 
	else {	/* aux is of long format */
		toindex = fepcb->auxbufs;
		from = mconvlist;
		for (i = 0; i < MCONVAUXNUM; i++)
			(void)memcpy(*toindex++, from + (MCONVAUXLEN * i),
				MCONVAUXLEN);
		fepcb->auxacsz.itemsize = MCONVAUXLEN; 
		fepcb->auxacsz.itemnum = MCONVAUXNUM; 
	} 
	/* no cursor in conversion switch at all */
	fepcb->auxcrps.colpos = -1;
	fepcb->auxcrps.rowpos = -1;  /* count from zero */
	fepcb->axcrpsch = ON;
	return;
}

/*-----------------------------------------------------------------------*
*	update_echobuf
* updates echo buffer if it has changed
*-----------------------------------------------------------------------*/
static void	update_echobuf(FEPCB *fepcb)
{
	KCB	*kcb = fepcb->kcb;
	int	pos;
	unsigned char	attchar;

	if (kcb->chlen > 0) {
		(void)memcpy(fepcb->echobufs, kcb->string, (int)kcb->lastch);
		for (pos = 0; pos < kcb->lastch; pos++) {
			attchar = REVERSE & kcb->hlatst[pos];
			if (attchar == REVERSE)
				attchar = KP_HL_REVERSE;
			fepcb->echobufa[pos] = attchar | KP_HL_UNDER;
		}
		fepcb->echochfg.flag = ON;
		fepcb->echochfg.chtoppos = kcb->chpos;
		fepcb->echochfg.chlenbytes = kcb->chlen;
		fepcb->echoacsz = kcb->lastch;
	}
	return;
}

/*-----------------------------------------------------------------------*
*	update cursor position
* updates cursor position in Echo buffer if it has changed
*-----------------------------------------------------------------------*/
static void	update_curpos(FEPCB *fepcb)
{
	KCB	*kcb = fepcb->kcb;

	if (kcb->curcol != fepcb->echocrps) {
		fepcb->echocrps = kcb->curcol;
		fepcb->eccrpsch = ON;
	}
	return;
}

/*-----------------------------------------------------------------------*
*	update auxiliary buffer 
* this routine does not handle aux area of mode switch which has 
* actually attribute values other than zero. so, this routine
* simply clear attribute buffer with zero.
* updates Aux buffer if it has changed
*-----------------------------------------------------------------------*/
static void	update_auxbuf(FEPCB *fepcb)
{
	KCB	*kcb = fepcb->kcb;
	int	i;
	char	**toindex, *from;

	if (kcb->axuse1 == USE && fepcb->auxuse == USE) {
		if (kcb->chlna1 > 0) {
			/* string data */
			toindex = fepcb->auxbufs;
			from = kcb->aux1;
			for (i = 0; i < kcb->ax1row; i++) 
				(void)memcpy(*toindex++,
					from + (kcb->ax1col * i),
					(int)kcb->ax1col);
			/* attribute data */
			toindex = fepcb->auxbufa;
			for (i = 0; i < kcb->ax1row; i++) 
				(void)memset(*toindex++, 0, (int)kcb->ax1col);
			fepcb->auxchfg = ON;
			fepcb->auxacsz.itemsize = kcb->ax1col;
			fepcb->auxacsz.itemnum = kcb->ax1row;
		}
		/* only Kanji Number input aux area has cursor */
		if (kjauxkind(kcb) == AX_KJCODE) {
			if (kcb->cura1c != fepcb->auxcrps.colpos) {
				fepcb->auxcrps.colpos = kcb->cura1c;
				fepcb->axcrpsch = ON;
			}
			if (kcb->cura1r != fepcb->auxcrps.rowpos) {
				fepcb->auxcrps.rowpos = kcb->cura1r;
				fepcb->axcrpsch = ON;
			}
		}
	}
	else if (kcb->axuse1 == USE && fepcb->auxuse == NOTUSE) {
		/* string data */
		toindex = fepcb->auxbufs;
		from = kcb->aux1;
		for (i = 0; i < kcb->ax1row; i++) 
			(void)memcpy(*toindex++, from + (kcb->ax1col * i),
				(int)kcb->ax1col);
		/* attribute data */
		toindex = fepcb->auxbufa;
		for (i = 0; i < kcb->ax1row; i++) 
			(void)memset(*toindex++, 0, (int)kcb->ax1col);
		fepcb->auxuse = USE;
		fepcb->auxchfg = ON;
		fepcb->auxacsz.itemsize = kcb->ax1col;
		fepcb->auxacsz.itemnum = kcb->ax1row;
		/* only Kanji Numer aux area has cursor */
		if (kjauxkind(kcb) == AX_KJCODE) {
			fepcb->auxcrps.colpos = kcb->cura1c;
			fepcb->auxcrps.rowpos = 0;  /* count from zero */
			fepcb->axcrpsch = ON;
		}
		else {
			fepcb->auxcrps.colpos = -1;
			fepcb->auxcrps.rowpos = -1;  /* count from zero */
			fepcb->axcrpsch = ON;
		}
	}
	else if (kcb->axuse1 == NOTUSE && fepcb->auxuse == USE) {
		fepcb->auxuse = NOTUSE;
		fepcb->auxchfg = ON;
		fepcb->axcrpsch = ON;
		fepcb->auxacsz.itemsize = 0;
		fepcb->auxacsz.itemnum = 0;
		fepcb->auxcrps.colpos = -1;
		fepcb->auxcrps.rowpos = -1;  /* count from zero */
	}
	return;
}

/*-----------------------------------------------------------------------*
*	update indicator
*  updates indicator buffer if it has changed
*-----------------------------------------------------------------------*/
static void	update_indicator(FEPCB *fepcb)
{
	extern void	set_indicator();

	KCB	*kcb = fepcb->kcb;

	switch (kcb->shift) {
	case SHIFT1:	/* AN / K / H */
		fepcb->shift[0] = kcb->shift1;
		set_indicator(fepcb);
		fepcb->indchfg = ON;
		break;

	case SHIFT2:	/* RKC On / Off */
		fepcb->shift[1] = kcb->shift2;
		set_indicator(fepcb);
		fepcb->indchfg = ON;
		break;

	case SHIFT3:	/* Single / Double */
		fepcb->shift[2] = kcb->shift3;
		set_indicator(fepcb);
		fepcb->indchfg = ON;
		break;
	}
	return;
}

/*
 *	update_all()
 *	update echobuf, cursor position and auxbuf.
 */

static void	update_all(FEPCB *fepcb)
{
	update_echobuf(fepcb);
	update_curpos(fepcb);
	update_auxbuf(fepcb);
}

/*
 * JAO detection (Just After Overflow)
 *
 * First, see if monitor itself in overflow state.
 * If so, send RESET
 * Second, see if monitor string length exceeds overflow limit of
 * jed, in that case, make it trucated.
 */
static void	jao(FEPCB *fepcb)
{
	KCB	*kcb = fepcb->kcb;
	unsigned char	c;
	int	i;
	int	savedcursor;
	int	savedchpos;
	int	savedchlen;

	if (kcb->cnvsts == OVERFLOW) {
		kcb->type = TYPE2;
		kcb->code = PRESET;
		exkjinpr(kcb);
		savedchpos = kcb->chpos;
		savedchlen = kcb->chlen;
		kcb->type = TYPE2;
		kcb->code = PRESET;
		exkjinpr(kcb);
		fepcb->isbeep = ON;
		kcb->chpos = savedchpos;
		kcb->chlen = savedchlen;
	}
	if (kcb->lastch > fepcb->echoover) {
		i = 0;
		while (1) {
			if (0x81 <= (c = kcb->string[i]) && c <= 0x9f ||
				0xe0 <= c && c <= 0xfc) {
				if (i + 2 > fepcb->echoover)
					break;
				i += 2;
			}
			else { 
				if (i + 1 > fepcb->echoover)
					break;
				i++;
			}
		}
		/* place cursor at appropriate position */
		savedcursor = kcb->curcol;
		savedchpos = kcb->chpos;
		kcb->setcsc = i;
		kcb->setcsr = 1;
		exkjcrst(kcb);
		kcb->type = TYPE2;
		kcb->code = PEREOF;
		exkjinpr(kcb);
		if (savedcursor < i) {
			kcb->setcsc = savedcursor;
			kcb->setcsr = 1;
			exkjcrst(kcb);
		}
		fepcb->isbeep = ON;
		(void)rkcbufclear(kcb);
		if (fepcb->echoover - savedchpos > 0)
			kcb->chlen = fepcb->echoover - savedchpos;
		else
			kcb->chlen = 0;
		kcb->chpos = savedchpos;
	}
	return;
}

/*
 *	charinpr()
 *	merely a convinence function.
 *	call kanji monitor with a character input.
 */
static void	charinpr(FEPCB *fepcb, unsigned char c)
{
	fepcb->kcb->type = TYPE1;
	fepcb->kcb->code = c;
	(void)exkjinpr(fepcb->kcb);
        if (fepcb->kcb->beep)
		fepcb->isbeep = ON;
}

/*
 *	rkcmap()
 *	convert alphabet to katakana which is corresponding to the
 *	alphabet on the IBM106 Japanese keyboard.
 *	Only [A-Za-z] are valid input to this function.
 */
static char	rkcmap(unsigned int keysym)
{
	static char	RkcMap[] = {
		0xde, 0xc1, 0xba, 0xbf, 0xbc, 0xb2, 0xca, 0xb7,	/*  a -  g */
		0xb8, 0xc6, 0xcf, 0xc9, 0xd8, 0xd3, 0xd0, 0xd7,	/* h  -  o */
		0xbe, 0xc0, 0xbd, 0xc4, 0xb6, 0xc5, 0xcb, 0xc3,	/* p  -  w */
		0xbb, 0xdd, 0xc2, 0xdf, 0xb0, 0xa3, 0xcd, 0xdb	/* x - z   */
	};
	return RkcMap[keysym & 0x1f];
}

/*
 *	net_char_filter()
 *	- JBO (Just Before Overflow) : detect overflow condition
 *	  before calling the kanji monitor.
 *	- JAO (Just After Overflow) : if the overflow occured as the
 *	  result of calling the kanji monitor, reset it and truncate
 *	  the string.  There are 2 cases, overflow of fep's echo
 *	  buffer and overflow of the kanji monitor itself.
 *	- Direct Input : what should happen if alphabet comes in when
 *	  the input mode is kana ?
 *	  The Direct Input is a new feature to escape the current
 *	  input mode and input whatever comes to kanji monitor properly.
 */
static void	net_char_filter(FEPCB *fepcb, unsigned int keysym)
{
	KCB	*kcb = fepcb->kcb;
	char	shift1, shift2, shift3;		/* save area */
	char	c;
	short	chlen, chpos, chpsa1, chlna1;

	kcb->chlen = 0; /* pretend Monitor makes no change */
	kcb->chlna1 = 0;

	/*
	 *	JBO detection (Just Before Overflow)
	 *	and character input.
	 */
	shift1 = kcb->shift1; shift2 = kcb->shift2; shift3 = kcb->shift3;
	c = keysym & 0xff;
	if (shift1 == ALPHANUM) {
		if (shift3 == SINGLE) {
			if (kcb->lastch > fepcb->echoover - 1) {
				fepcb->isbeep = ON;
				return;
			}
		}
		else {
			if (kcb->lastch > fepcb->echoover - 2) {
				fepcb->isbeep = ON;
				return;
			}
		}
		if (iskanakeysym(keysym))
			chimode(kcb, KATAKANA, shift2, shift3);
	}
	else if (shift1 == KATAKANA) {
		if (shift2 == RKC_ON) {
			if (shift3 == SINGLE) {
				if (iskanakeysym(keysym)) {
					if (kcb->lastch > fepcb->echoover - 1) {
						fepcb->isbeep = ON;
						return;
					}
					chimode(kcb, shift1, RKC_OFF, shift3);
				}
				else if (ispunctkeysym(keysym) ||
					isspacekeysym(keysym)) {
					if (kcb->lastch > fepcb->echoover - 1) {
						fepcb->isbeep = ON;
						return;
					}
					chimode(kcb, ALPHANUM, RKC_OFF, shift3);
				}
				else {
					if (kcb->lastch > fepcb->echoover - 3) {
						fepcb->isbeep = ON;
						return;
					}
					c = rkcmap(keysym);
				}
			}
			else {
				if (iskanakeysym(keysym)) {
					if (kcb->lastch > fepcb->echoover - 2) {
						fepcb->isbeep = ON;
						return;
					}
					chimode(kcb, shift1, RKC_OFF, shift3);
				}
				else if (ispunctkeysym(keysym) ||
					isspacekeysym(keysym)) {
					if (kcb->lastch > fepcb->echoover - 2) {
						fepcb->isbeep = ON;
						return;
					}
					chimode(kcb, ALPHANUM, RKC_OFF, shift3);
				}
				else {
					if (kcb->lastch > fepcb->echoover - 4) {
						fepcb->isbeep = ON;
						return;
					}
					c = rkcmap(keysym);
				}
			}
		}
		else {
			if (shift3 == SINGLE) {
				if (kcb->lastch > fepcb->echoover - 1) {
					fepcb->isbeep = ON;
					return;
				}
			}
			else {
				if (kcb->lastch > fepcb->echoover - 2) {
					fepcb->isbeep = ON;
					return;
				}
			}
			if (isasciikeysym(keysym) || isspacekeysym(keysym))
				chimode(kcb, ALPHANUM, shift2, shift3);
		}
	}
	else /* shift1 == HIRAGANA */ {
		if (shift2 == RKC_ON) {
			if (iskanakeysym(keysym)) {
				if (kcb->lastch > fepcb->echoover - 2) {
					fepcb->isbeep = ON;
					return;
				}
				chimode(kcb, shift1, RKC_OFF, DOUBLE);
			}

			else if ((XK_exclam <= keysym &&
						keysym <= XK_semicolon) ||
					keysym == XK_equal ||
					keysym == XK_at ||
					(XK_bracketleft <= keysym &&
						keysym <= XK_grave) ||
					keysym == XK_bar ||
					keysym == XK_asciitilde) {
				if (kcb->lastch > fepcb->echoover - 2) {
					fepcb->isbeep = ON;
					return;
				}
				chimode(kcb, ALPHANUM, RKC_OFF, shift3);
			}

			else if (ispunctkeysym(keysym) ||
				isspacekeysym(keysym)) {
				if (kcb->lastch > fepcb->echoover - 2) {
					fepcb->isbeep = ON;
					return;
				}
				chimode(kcb, ALPHANUM, RKC_OFF, DOUBLE);
			}
			else {
				if (kcb->lastch > fepcb->echoover - 4) {
					fepcb->isbeep = ON;
					return;
				}
				c = rkcmap(keysym);
			}
		}
		else {
			if (kcb->lastch > fepcb->echoover - 2) {
				fepcb->isbeep = ON;
				return;
			}
			if (isasciikeysym(keysym) || isspacekeysym(keysym))
				chimode(kcb, ALPHANUM, shift2, DOUBLE);
		}
	}
	charinpr(fepcb, c);
	chlen = kcb->chlen;
	chpos = kcb->chpos;
	chpsa1 = kcb->chpsa1;
	chlna1 = kcb->chlna1;
	chimode(kcb, shift1, shift2, shift3);	/* restore original imode */
	kcb->chlen = chlen;
	kcb->chpos = chpos;
	kcb->chpsa1 = chpsa1;
	kcb->chlna1 = chlna1;

	/*
	 *	JAO (Just After Overflow)
	 *	check overflow condition
	 */
	jao(fepcb);
}

/*
 *	char_filter()
 */
static int	char_filter(FEPCB *fepcb, IMKeymap *immap,
	unsigned int keysym, IMBuffer *imb)
{
	KCB	*kcb = fepcb->kcb;
	int	intstate;

	intstate = get_state(fepcb);

	switch (intstate) {
	case INACTKNJ:
	case ACTKNJ:
	case ALLCANDS:
		keysym = mapspecial(keysym);
		if (!isnumkeysym(keysym)) {
			if (!IsNoBeep(keysym))
				fepcb->isbeep = ON;
			return KP_USED;
		}
		net_char_filter(fepcb, keysym);
		update_all(fepcb);
		return KP_USED;
	case CONVSW:
		keysym = mapspecial(keysym);
		switch (keysym) {
		case XK_1:
			if (fepcb->tconvmode != LOOKAHEAD) {
				fepcb->tconvmode = LOOKAHEAD;
				set_highlight(fepcb);
				fepcb->auxchfg = ON;
			}
			break;
		case XK_2:
			if (fepcb->tconvmode != MPHRASE) {
				fepcb->tconvmode = MPHRASE;
				set_highlight(fepcb);
				fepcb->auxchfg = ON;
			}
			break;
		case XK_3:
			if (fepcb->tconvmode != SPHRASE) {
				fepcb->tconvmode = SPHRASE;
				set_highlight(fepcb);
				fepcb->auxchfg = ON;
			}
			break;
		case XK_4:
			if (fepcb->tconvmode != WORD) {
				fepcb->tconvmode = WORD;
				set_highlight(fepcb);
				fepcb->auxchfg = ON;
			}
			break;
		default:
			if (!IsNoBeep(keysym))
				fepcb->isbeep = ON;
			break;
		}
		return KP_USED;
	}

	/*
	 *	PROCESSOF (aka SuppressedMode)
	 *	Half Width Katakana characters may be generated (but no RKC)
	 */
	if (intstate == PROCESSOFF) {
		if (fepcb->alpha || !ischarkeysym(keysym))
			return KP_NOTUSED;
		if (isasciikeysym(keysym) || isspcharkeysym(keysym))
			keysym = maptobase(fepcb, immap, keysym, ES_KANA);
		keysym = mapspecial(keysym);
		if (ischarkeysym(keysym))
			_IMSimpleMapping(immap, keysym, 0, imb);
		return KP_USED;
	}

	/*
	 *	normal character input (INACTIVE, ACTIVATING or ACTIVE)
	 */

	/*
	 *	input is not used in the following conditions.
	 */
	if (intstate == INACTIVE || intstate == ACTIVATING) {
		if (fepcb->shift[2] == SINGLE && fepcb->shift[0] == ALPHANUM ||
			!ischarkeysym(keysym)) {
			if (intstate == ACTIVATING)
				(void)rkcbufclear(kcb);
			return KP_NOTUSED;
		}
	}

	if (fepcb->shift[0] == KATAKANA || fepcb->shift[0] == HIRAGANA) {
		if (fepcb->shift[1] == RKC_ON) {
			if (ispunctkeysym(keysym) || isspcharkeysym(keysym))
				keysym = maptobase(fepcb, immap, keysym, ES_RKC);
		}
		else {
			if (isasciikeysym(keysym) || isspcharkeysym(keysym))
				keysym = maptobase(fepcb, immap, keysym, ES_KANA);
		}
	}

	keysym = mapspecial(keysym);	/* get equivalent.  e.g., KP_0 -> 0 */

	if (!ischarkeysym(keysym)) {
		if (intstate == ACTIVE && !IsNoBeep(keysym))
			fepcb->isbeep = ON;
		return KP_USED;
	}

	net_char_filter(fepcb, keysym);
	update_all(fepcb);
	if (intstate != ACTIVE && kcb->lastch > 0 &&
		fepcb->shift[2] == SINGLE && fepcb->shift[0] != HIRAGANA) {
		placestr(imb, kcb->string, kcb->lastch);
		fepcb->echochfg.chlenbytes = kcb->lastch;
		fepcb->echochfg.chtoppos = 0;
		(void)exkjclr2(kcb); /* clear but rkc */
		(void)memset(fepcb->echobufs, 0, fepcb->echosize);
		fepcb->echoacsz = 0;
		fepcb->echochfg.flag = ON;
		fepcb->echocrps = 0;
		fepcb->eccrpsch = ON;
		(void)set_imode(fepcb);
	}
	return KP_USED;
}

/*
 *	net_func_filter()
 *	convert a given keysym to a pseudo code and call the kanji
 *	monitor.
 *	does the JAO (see the above net_char_filter())
 */
static void	net_func_filter(FEPCB *fepcb, unsigned int keysym)
{
	KCB	*kcb = fepcb->kcb;

	kcb->type = TYPE2;
	kcb->code = get_pcode(keysym);
	exkjinpr(kcb);
        if (kcb->beep)
		fepcb->isbeep = ON;
	jao(fepcb);
}

/*
 *	func_filter()
 */
static int	func_filter(FEPCB *fepcb, IMKeymap *immap,
	unsigned int keysym, IMBuffer *imb)
{
	KCB	*kcb = fepcb->kcb;
	int	intstate;
	int	type;

	intstate = get_state(fepcb);

	/*
	 *	support Zenkaku/Hankaku (Full/Half width character) Lock key.
	 *	map them to Zenkaku_Hankaku_toggle.
	 */
	switch (keysym) {
	case XK_Zenkaku:
		if (fepcb->shift[2] == DOUBLE)
			return KP_USED;
		keysym = XK_Zenkaku_Hankaku;
		break;
	case XK_Hankaku:
		if (fepcb->shift[2] == SINGLE)
			return KP_USED;
		keysym = XK_Zenkaku_Hankaku;
		break;
	}

	type = get_func_type(keysym);

	if (type == KEY_SHIFT) {
		/*
		 *	Shift keys are all trapped here.
		 */
		if (intstate != PROCESSOFF) {
			/*
			 *	If @im=DoubleByte, don't switch to
			 *	Half Width mode.
			 */
			if (fepcb->dbcsormix != KP_ONLYDBCS ||
				keysym != XK_Zenkaku_Hankaku) {
				net_func_filter(fepcb, keysym);
				update_indicator(fepcb);
			}
		}
		else {
			/*
			 *	PROCESSOFF (Suppressed Mode)
			 *	No preediting but JISX0201 Katakana
			 *	characters are generated.
			 *	So here we track only XK_Katakana and
			 *	XK_Eisu_toggle.
			 *	The indicators are not updated (no callback
			 *	is called in Suppressed Mode).
			 */
			if (keysym == XK_Katakana)
				fepcb->alpha = FALSE;
			else if (keysym == XK_Eisu_toggle)
				fepcb->alpha = TRUE;
		}
		return KP_USED;
	}

	if (intstate == PROCESSOFF)
		return KP_NOTUSED;

	switch (intstate) {
	case INACTIVE:
		switch (type) {
		case KEY_KJNUM:
			net_func_filter(fepcb, keysym);
			update_all(fepcb);
			return KP_USED;
		case KEY_CONVSW:
			fepcb->tconvmode = fepcb->convmode;
			set_convsw_state(fepcb);
			fepcb->axconvsw = ON;
			fepcb->auxuse = USE;
			fepcb->auxchfg = ON;
			set_highlight(fepcb);
			return KP_USED;
		case KEY_CONV:
		case KEY_NOCONV:
			if(kcb->shift1 == ALPHANUM && kcb->shift3 == SINGLE){
				return KP_NOTUSED;
			}
		case KEY_PRECAND:
		case KEY_ALLCAND:
		case KEY_REGST:
			/*
			 *	These are eaten.  Especially CONV and
			 *	NOCONV are eaten here otherwise they
			 *	generate a space character on return
			 *	from IMLookupString().   This is bad
			 *	if the state is Full Width state.
			 */
			return KP_USED;
		}
		return KP_NOTUSED;
	case ACTIVATING:
		/*
		 *	This is a subtle state.  RKC consonant is stacked.
		 *	BackSpace needs a special treatment.
		 */
		switch (type) {
		case KEY_ENTER:
			net_func_filter(fepcb, keysym);
			update_all(fepcb);
			if (kcb->lastch == 0)
				return KP_NOTUSED;
			placestr(imb, kcb->string, kcb->lastch);
			fepcb->echochfg.chlenbytes = kcb->lastch;
			fepcb->echochfg.chtoppos = 0;
			(void)exkjclr(kcb);
			(void)memset(fepcb->echobufs, 0, fepcb->echosize);
			fepcb->echoacsz = 0;
			fepcb->echochfg.flag = ON;
			fepcb->echocrps = 0;
			fepcb->eccrpsch = ON;
			(void)set_imode(fepcb);
			return KP_USED;
		case KEY_KJNUM:
			rkcbufclear(kcb);
			net_func_filter(fepcb, keysym);
			update_all(fepcb);
			return KP_USED;
		case KEY_CONVSW:
			rkcbufclear(kcb);
			fepcb->tconvmode = fepcb->convmode;
			set_convsw_state(fepcb);
			fepcb->axconvsw = ON;
			fepcb->auxuse = USE;
			fepcb->auxchfg = ON;
			set_highlight(fepcb);
			return KP_USED;
		case KEY_CONV:
		case KEY_NOCONV:
		case KEY_PRECAND:
		case KEY_ALLCAND:
		case KEY_BACKSP:
			rkcbufclear(kcb);
			return KP_USED;
		}
		rkcbufclear(kcb);
		return KP_NOTUSED;
	case ACTIVE:
		switch (type) {
		case KEY_CONVSW:
		case KEY_REGST:
		case KEY_RESET:
		case KEY_OTHERS:
			fepcb->isbeep = ON;
			return KP_USED;
		case KEY_ENTER:
			net_func_filter(fepcb, keysym);
			update_all(fepcb);
			if (kcb->lastch > 0) {
				placestr(imb, kcb->string, kcb->lastch);
				fepcb->echochfg.chlenbytes = kcb->lastch;
				fepcb->echochfg.chtoppos = 0;
				(void)exkjclr(kcb);
				(void)memset(fepcb->echobufs, 0,
					fepcb->echosize);
				fepcb->echoacsz = 0;
				fepcb->echochfg.flag = ON;
				fepcb->echocrps = 0;
				fepcb->eccrpsch = ON;
				(void)set_imode(fepcb);
			}
			return KP_USED;
		case KEY_CURUP:
			return KP_UP;
		case KEY_CURDOWN:
			return KP_DOWN;
		case KEY_ALLCAND:
			if(fepcb->selection){
				jedMakeListbox(fepcb, PALLCAND);
				if (kcb->beep){
					fepcb->isbeep = ON;
					return KP_USED;
				}
				fepcb->auxuse = USE;
				fepcb->auxchfg = ON;
				fepcb->auxacsz.itemsize = kcb->ax1col;
				{
					IMPanel *jedGetPanel();
					IMPanel *panelp = jedGetPanel(fepcb);
					fepcb->auxacsz.itemnum = panelp->item_row;
				}
				return KP_USED;
			}
		}
		net_func_filter(fepcb, keysym);
		update_all(fepcb);
		return KP_USED;
	case INACTKNJ:
	case ACTKNJ:
		switch (type) {
		case KEY_ENTER:
			net_func_filter(fepcb, keysym);
			update_all(fepcb);
			if (intstate == ACTKNJ)
				return KP_USED;
			if (kcb->lastch == 0)
				return KP_USED;
			placestr(imb, kcb->string, kcb->lastch);
			(void)exkjclr(kcb);
			kcb->code = PKJNUM;
			kcb->type = TYPE2;
			(void)exkjinpr(kcb);
			(void)memset(fepcb->echobufs, 0, fepcb->echosize);
			fepcb->echoacsz = 0;
			fepcb->echochfg.flag = OFF;
			fepcb->echocrps = 0;
			fepcb->eccrpsch = ON;
			(void)set_imode(fepcb);
			return KP_USED;
		case KEY_BACKSP:
		case KEY_CURSOR:
		case KEY_ERASE:
		case KEY_KJNUM:
		case KEY_RESET:
			net_func_filter(fepcb, keysym);
			update_all(fepcb);
			return KP_USED;
		}
		fepcb->isbeep = ON;
		return KP_USED;
	case ALLCANDS:
		switch (type) {
		case KEY_CONV:
		case KEY_PRECAND:
		case KEY_ENTER:
		case KEY_RESET:
			net_func_filter(fepcb, keysym);
			update_all(fepcb);
			return KP_USED;
		}
		fepcb->isbeep = ON;
		return KP_USED;
	case CONVSW:
		switch (type) {
		case KEY_ENTER:
			if (fepcb->tconvmode != fepcb->convmode) {
				fepcb->convmode = fepcb->tconvmode;
				exsetcmode(kcb, fepcb->convmode);
				set_imode(fepcb);
			}
			/* drop thru */
		case KEY_RESET:
			fepcb->auxuse = NOTUSE;
			fepcb->auxacsz.itemsize = 0; 
			fepcb->auxacsz.itemnum = 0; 
			fepcb->axconvsw = OFF;
			fepcb->auxchfg = ON;
			return KP_USED;
		}
		fepcb->isbeep = ON;
		return KP_USED;
	}
	fepcb->isbeep = ON;	/* never happen */
	return KP_USED;
}

/*
 *	jedFilter
 */
int	jedFilter(FEPCB *fepcb, IMKeymap *immap,
		unsigned int keysym, unsigned int state, IMBuffer *imb)
{
	/*
	 *  sets all trigger flags 'off'
	 */
	fepcb->echochfg.flag = OFF;
	fepcb->auxchfg  = OFF;
	fepcb->indchfg  = OFF;
	fepcb->eccrpsch = OFF;
	fepcb->axcrpsch = OFF;
	fepcb->isbeep = OFF;

	keysym = maptobase(fepcb, immap, keysym, state);

	if (keysym == XK_Touroku && fepcb->echoacsz == 0)
		return KP_REGISTRATION;

	if (isfunckeysym(keysym))
		return func_filter(fepcb, immap, keysym, imb);
	return char_filter(fepcb, immap, keysym, imb);
}

/*
 *	jedLookup
 */
void	jedLookup(FEPCB *fepcb, IMKeymap *immap,
		unsigned int keysym, unsigned int state, IMBuffer *imb)
{
	_IMMapKeysym(immap, &keysym, &state);
	_IMSimpleMapping(immap, keysym, state, imb);
	return;
}


/*
 *	For Listbox Support
 */

#define	SPZERO	{0x81, 0x40, 0x81, 0x40, 0x82, 0x4f}		/* "  0" */
#define	BLANK	{0x81, 0x40, 0x81, 0x40, 0x81, 0x40, 0x81, 0x40, 0x81, 0x40}
								/* "     " */
#define	ZENKOHO	{0x91, 0x53, 0x8c, 0xf3, 0x95, 0xe2}		/* "zenkoho" */
#define	TANKAN	{0x81, 0x40, 0x92, 0x50, 0x8a, 0xbf}		/* "tankan" */

IMPanel	*jedGetPanel(FEPCB *fepcb)	{ return(&(fepcb->impanel)); }
char	*jedGetTitle(FEPCB *fepcb)	{ return(fepcb->title); }
int	jedSetSelectionFlag(FEPCB *fepcb, int flag)
					{ return(fepcb->selection = flag); }
int	jedHasSelection(FEPCB *fepcb)	{ return(fepcb->impanel.item != NULL); }
int	IsTankanList(FEPCB *fepcb)
{
	char	tankan[] = TANKAN;
	return(!strncmp(tankan, jedGetTitle(fepcb), sizeof(tankan)));
}

/*
 *	Make Listbox Data (SJIS) by Simulating Key Input
 */
jedMakeListbox(
FEPCB	*fepcb,		/* FEPCB pointer */
int	first		/* specify the first key (PALLCAND or PCONVERT) */
)
{
	char	zero[] = SPZERO;	/* "  0" */
	char	blank[] = BLANK;	/* "     " */
	char	zenkoho[] = ZENKOHO;	/* "zenkoho" */
	char	tankan[] = TANKAN;	/* "tankan" */
	KCB	*kcb = fepcb->kcb;
	char	*str;
	IMItem	*item;
	int	itemmax, itemnum, stlen, i, j, len;

	/*
	 * Simulate the First Key Input
	 */
	kcb->type = TYPE2;
	kcb->code = first;
	exkjinpr(kcb);
	if (kcb->beep){
		fepcb->isbeep = ON;
		return;
	}

	/*
	 * Allocate IMItem[] and String Area for 50 items (at first)
	 */
	itemmax = 50;
	item = (IMItem *)malloc(itemmax * sizeof(IMItem));
	if(item == NULL){
		fepcb->isbeep = ON;
		return;
	}
	stlen = kcb->ax1col;
	str = (char *)malloc(itemmax * stlen * 2);
	if(str == NULL){
		free(item);
		fepcb->isbeep = ON;
		return;
	}

	/*
	 * Set the Title of the Listbox
	 */ 
	if(strncmp(kcb->aux1, tankan, sizeof(tankan)) == 0){
		strncpy(fepcb->title, tankan, sizeof(tankan));
		fepcb->title[sizeof(tankan)] = '\0';
	}else{
		strncpy(fepcb->title, zenkoho, sizeof(zenkoho));
		fepcb->title[sizeof(zenkoho)] = '\0';
	}

	/*
	 * Set IMItem Data of the 1st Page (kcb->aix1row <= 10)
	 */
	itemnum = 0;
	for(i = 2; i < kcb->ax1row; i++){	/* The 1st two are titles */
		memcpy(str, kcb->aux1 + stlen * i + 4, stlen - 4);
		memcpy(str + stlen - 4, blank, 4);
		if(!(strncmp(str, blank, 6) == 0)){
			item[itemnum].selectable = itemnum+1;
			item[itemnum].text.str = str;
			item[itemnum++].text.len = stlen;
		}
		str += stlen;
	}

	/*
	 * Set IMItem Data until the # of Rest Candidates is 0
	 */
	while(strncmp(&(kcb->aux1[12]), zero, sizeof(zero)) != 0){
		/*
		 * Simulate Key Input
		 */
		kcb->type = TYPE2;
		kcb->code = PCONVERT;
		exkjinpr(kcb);
		if (kcb->beep){
			fepcb->isbeep = ON;
			break;
		}
		/*
		 * Set IMItem Data
		 */
		for(i = 2; i < kcb->ax1row; i++){ /* The 1st two are titles */
			memcpy(str, kcb->aux1 + stlen * i + 4, stlen - 4);
			memcpy(str + stlen - 4, blank, 4);
			if(!(strncmp(str, blank, 6) == 0)){
				item[itemnum].selectable = itemnum+1;
				item[itemnum].text.str = str;
				item[itemnum++].text.len = stlen;
			}
			str += stlen;
		}
		/*
		 * Reallocate IMItem[] if itemnum >= itemmax
		 */
		if(itemmax <= itemnum){
			itemmax += 50;
			item = (IMItem*)realloc(item, itemmax * sizeof(IMItem));
			if(item == NULL){
				fepcb->isbeep = ON;
				break;
			}
			/*
			 * Allocate New Area for New Strings
			 */
			str = (char *)malloc(itemmax * stlen * 2);
			if(str == NULL){
				free(item);
				fepcb->isbeep = ON;
				break;
			}
		}
	}

	/*
	 * Remove Trailing Spaces (except for ONE space)
	 */
	len = 2;
	for(i = 0; i < itemnum; i ++){
		for(j = stlen - 2; j > len; j -= 2){
			if(item[i].text.str[j] != 0x81 ||
					item[i].text.str[j+1] != 0x40) {
				if(j > len) {
					len = j;
					break;
				}
			}
		}
	}
	len += 4;
	if(len < stlen){
		stlen = len;
		for(i = 0; i < itemnum; i ++){
			item[i].text.len = len;
		}
	}

	/*
	 * Set IMPanel Data
	 */
	fepcb->impanel.maxwidth = stlen;
	fepcb->impanel.item_row = itemnum;
	fepcb->impanel.item_col = 1;
	fepcb->impanel.item = item;

	jao(fepcb);
	return;
}

/*
 *	Free Listbox Data Area
 */
jedFreeListbox(FEPCB *fepcb)
{
	int	i;

	for(i = 0; i < fepcb->impanel.item_row; i += 50){
		if(fepcb->impanel.item[i].text.str != NULL){
			free(fepcb->impanel.item[i].text.str);
		}
	}
	if(fepcb->impanel.item != NULL){
		free(fepcb->impanel.item);
		fepcb->impanel.item = NULL;
	}
	fepcb->impanel.item_row = 0;
	fepcb->impanel.item_col = 0;
}

/*
 *	Set the Position at the 1st Page of the List
 */
static	jedFirstPage(FEPCB *fepcb)
{
	char	zero[] = SPZERO;	/* "  0" */
	KCB	*kcb = fepcb->kcb;
	char	str[16];

	kcb->type = TYPE2;
	kcb->code = PCONVERT;
	memcpy(str, &(kcb->aux1[12]), sizeof(zero));
	while((kcb->ax1row > 0) &&
			(strncmp(&(kcb->aux1[12]), zero, sizeof(zero)) != 0)){
		exkjinpr(kcb);
		if (kcb->beep){
			fepcb->isbeep = ON;
			return;
		}
		if (strncmp(&(kcb->aux1[12]), str, sizeof(zero)) == 0){
			fepcb->isbeep = ON;
			return;
		}
	}

	exkjinpr(kcb);
	if (kcb->beep){
		fepcb->isbeep = ON;
	}
}

/*
 *	Set the Position at the specified Page of the List
 */
jedPositionPage(FEPCB *fepcb, int page)
{
	KCB	*kcb = fepcb->kcb;

	jedFirstPage(fepcb);	/* Sometimes SINGLE KANJI data follows	*/
	jedFirstPage(fepcb);	/*		the all candidates	*/

	if(page == 0){
		return;
	}
	kcb->type = TYPE2;
	kcb->code = PCONVERT;
	for(; page > 0; page--){
		exkjinpr(kcb);
		if (kcb->beep){
			fepcb->isbeep = ON;
			return;
		}
	}
}


/*
 *	return KCB pointer
 */
KCB	*jedGetKCB(FEPCB *fp)
{
	return fp->kcb;
}
