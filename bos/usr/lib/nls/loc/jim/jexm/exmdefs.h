/* @(#)84	1.2  src/bos/usr/lib/nls/loc/jim/jexm/exmdefs.h, libKJI, bos411, 9428A410j 6/6/91 11:19:07 */

/*
 * COMPONENT_NAME :	Japanese Input Method - Ex Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  ExMon version 6.0       06/28/88
 */

/*
 *	ExMon version 4.   11/20/87
 */
	/********************************************************************/
	/*	definition of constants and macros								*/
	/********************************************************************/

/*	useful constant															*/
#define	TRUE	1
#define	FALSE	0
#define	YES		1
#define	NO		0
#define	NULL	0
#define	SPACE	0x20
#define	KATAKANA_N		0xdd
#define	KATAKANA_MI		0xd0
#define	HIRAGANA_WI		0x01
#define	HIRAGANA_WE		0x02
#define	HIRAGANA_XWA	0x03
#define	KATAKANA_VU		0x04
#define	KATAKANA_WI		0x05
#define	KATAKANA_WE		0x06
#define	KATAKANA_XWA	0x07
#define	KATAKANA_XKA	0x08
#define	KATAKANA_XKE	0x09
#define	KATAKANA_A		0xb1
#define	KATAKANA_I		0xb2
#define	KATAKANA_U		0xb3
#define	KATAKANA_SA		0xbb
#define	KATAKANA_CHI	0xc1
#define	KATAKANA_TE		0xc3
#define	KATAKANA_NA		0xc5
#define	KATAKANA_NI		0xc6
#define	KATAKANA_NU		0xc7
#define	KATAKANA_NO		0xc9
#define	KATAKANA_HI		0xcb
#define	KATAKANA_HU		0xcc

/*	useful macros															*/
#define	MAX(x,y)	((x) > (y) ? (x) : (y))
#define	MIN(x,y)	((x) > (y) ? (y) : (x))
#define	kj1st(c)  ((c) & 0x80 && ((c) - 0x20) & 0x40 && ~(((c) - 0x01) | 0x83))
#define	kj2nd(c)  ((c) >= 0x40)
#define	jiscii(c) (0x1f < (c) && (c) < 0x7f || 0xa0 < (c) && (c) < 0xe0)
#define	or	: case 
#define inpr2(p) {mkjcblk->type = TYPE2; mkjcblk->code = (p); _Jinpr(mkjcblk);}
#define inpr1(c) { \
		if (0x01 <= (c) && (c) <= 0x09) { \
			inpr2(PRKC) \
			switch (c) { \
			case HIRAGANA_WI : \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_TE; \
				_Jinpr(mkjcblk); \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_NI; \
				_Jinpr(mkjcblk); \
				break; \
			case HIRAGANA_WE : \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_TE; \
				_Jinpr(mkjcblk); \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_I; \
				_Jinpr(mkjcblk); \
				break; \
			case HIRAGANA_XWA : \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_SA; \
				_Jinpr(mkjcblk); \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_TE; \
				_Jinpr(mkjcblk); \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_CHI; \
				_Jinpr(mkjcblk); \
				break; \
			case KATAKANA_VU : \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_HI; \
				_Jinpr(mkjcblk); \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_NA; \
				_Jinpr(mkjcblk); \
				break; \
			case KATAKANA_WI : \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_TE; \
				_Jinpr(mkjcblk); \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_NI; \
				_Jinpr(mkjcblk); \
				break; \
			case KATAKANA_WE : \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_TE; \
				_Jinpr(mkjcblk); \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_I; \
				_Jinpr(mkjcblk); \
				break; \
			case KATAKANA_XWA : \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_SA; \
				_Jinpr(mkjcblk); \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_TE; \
				_Jinpr(mkjcblk); \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_CHI; \
				_Jinpr(mkjcblk); \
				break; \
			case KATAKANA_XKA : \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_SA; \
				_Jinpr(mkjcblk); \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_NO; \
				_Jinpr(mkjcblk); \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_CHI; \
				_Jinpr(mkjcblk); \
				break; \
			case KATAKANA_XKE : \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_SA; \
				_Jinpr(mkjcblk); \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_NO; \
				_Jinpr(mkjcblk); \
				mkjcblk->type = TYPE1; mkjcblk->code = KATAKANA_I; \
				_Jinpr(mkjcblk); \
				break; \
			} \
			inpr2(PRKC) \
		} \
		else {\
			mkjcblk->type = TYPE1; mkjcblk->code = (c); _Jinpr(mkjcblk); \
		} \
	}

	/********************************************************************/
	/*	constants definition for parameters in Kanji Control Block		*/
	/********************************************************************/

/*	for kjcblk->type														*/
#define	TYPE1	1
#define	TYPE2	2
#define	TYPE3	3

/*	for kjcblk->flatsd														*/
#define	ONLYDOUBLE	1
#define	MIXEDMODE	2

/*	for kjcblk->axuse1														*/
#define USE		1
#define NOTUSE	0

/*	for kjcblk->shift														*/
#define	SHIFT1	0x01
#define	SHIFT2	0x02
#define	SHIFT3	0x04

/*	for kjcblk->shift1														*/
#define	ALPHANUM 1
#define	KATAKANA 2
#define	HIRAGANA 3

/*	for kjcblk->shift2														*/
#define	RKC_ON	1
#define	RKC_OFF	2

/*	for kjcblk->shift3														*/
#define	SINGLE	1
#define	DOUBLE	2

/*	for kjcblk->curlen														*/
#define	CURATS	1
#define	CURATD	2

/*	for kjcblk->cnvsts														*/
#define	FINISHED	0
#define	GOINGON		1
#define	OVERFLOW	2

/*	for kjcblk->repins														*/
#define INSERT	1
#define	REPLACE	0

/*	for kjcblk->beep														*/
#define	OFF		0
#define	ON		1

/*	for	kjcblk->discrd														*/
#define	NOTDISCARD	0
#define	DISCARD		1

	/********************************************************************/
	/*	constants definition for Ex. Mon internal parameters			*/
	/********************************************************************/

/*	max length of SEISHO in dictionary										*/
#define	MAXLENOFKJ		40

/* for rkc. return character type											*/
#define SGLKATA	1
#define DBLKATA	2
#define DBLHIRA	3

/*	return code for characcept()											*/
#define	UNACCEPTABLE	0
#define	ACCEPTABLE		1

/*	for kjsvpt->initflg														*/
#define	INITIALIZED		1
#define	NOTTERMINATED	1
#define	NOTINITIALIZED	0
#define	TERMINATED		0

/*	for internal buffer size												*/
#define	P1BUFSIZE	8
#define	P2BUFSIZE	16

/*	for SDTTBL																*/
#define	SINGLEF		1
#define	DOUBLEF		2
#define	SPECIALF	3

/* special character code (Single and Double)								*/
#define SCHARS	0x24
#define SCHARD	0x8190
#define SCHARH	0x81
#define SCHARL	0x90

/*	for	IHLATST and MHIMAGE													*/
#define	KJMASK	0xf0
#define	KJ1st	0x10
#define	KJ2nd	0x20
#define	JISCII	0x30

/*	for	IHLATST, MHIMAGE, MHLATST and HLATST								*/
#define	HLMASK		0x0f
#define	NOHIGHLIGHT	0xf0
#define	REVERSE		0x01
#define	UNDERSCORE	0x02

/*	for conversion mode.													*/
#define	LOOKAHEAD	0
#define	MPHRASE		1
#define	SPHRASE		2
#define	WORD		3

/*	constant definition of Monitor internal state							*/
#define	NCS		0					/* No Convertable String State			*/
#define	CYI		(NCS+1)				/* Continuous Yomi Input State			*/
#define	CNV		(CYI+1)				/* CNVersion State						*/
#define	EDE		(CNV+1)				/* Edit State							*/
#define	EDM		(EDE+1)				/* Edit State							*/
#define	EDC		(EDM+1)				/* Edit State							*/
#define	HK1		(EDC+1)				/* Edit State							*/
#define	HK2		(HK1+1)				/* Hiragana/Katakana Conversion State 2	*/
#define	ACI		(HK2+1)				/* All Candidates Input State			*/
#define	KNI		(ACI+1)				/* Kanji Number Input State				*/
#define	CMS		(KNI+1)				/* Conversion Mode Switch State			*/
#define	DIA		(CMS+1)				/* Diagnosis State						*/
#define	OVF		(DIA+1)				/* Overflow State						*/

	/********************************************************************/
	/*	definition of pseudo code										*/
	/********************************************************************/

#define PINVALID	0x00			/* Invalid pseudo code					*/
#define PKATAKANA	0x01			/* Katakana shift						*/
#define PALPHANUM	0x02			/* Alpha_Numeric shift					*/
#define	PHIRAGANA	0x03			/* Hiragana shift						*/
#define	PRKC		0x04			/* RKC shift							*/
#define	PCONVERT	0x05			/* Convert key							*/
#define	PNOCONV		0x06			/* No_Convert key						*/
#define	PALLCAND	0x07			/* All_Candidates						*/
#define	PREGIST		0x08			/* Registration							*/
#define	PKJNUM		0x09			/* Kanji Number Input					*/
#define	PCONVSW		0x0a			/* Convert Mode Switch					*/
#define	PDIAGNOS	0x0b			/* Diagnosis							*/
#define	PPRECAND	0x0c			/* Previous Candidates					*/
#define	PSGLDBL		0x0d			/* single / double toggle				*/
#define	PYOMI		0x12			/* Bunsetsu-Yomi						*/
#define	PENTER		0x20			/* Enter								*/
#define	PACTION		0x21			/* Action								*/
#define	PCARRIGE	0x22			/* Action								*/
#define	PRESET		0x23			/* Reset								*/
#define	PCURRIGHT	0x24			/* Cursor Right							*/
#define	PCURLEFT	0x25			/* Cursor Left							*/
#define	PCURUP		0x26			/* Cursor Up							*/
#define	PCURDOWN	0x27			/* Cursor Down							*/
#define	PCURDRIGHT	0x28			/* Cursor Double Right					*/
#define	PCURDLEFT	0x29			/* Cursor Double Left					*/
#define	PEREOF		0x2a			/* ErEOF								*/
#define	PERINPUT	0x2b			/* ErInput								*/
#define	PINSERT		0x2c			/* Insert toggle						*/
#define	PDELETE		0x2d			/* Delete Character						*/
#define	PBACKSPACE	0x2e			/* Back Space							*/
#define	PTAB		0x2f			/* Tab									*/
#define	PBACKTAB	0x30			/* Back Tab								*/
#define	PHOME		0x31			/* Home									*/
#define	PESCAPE		0x32			/* Escape								*/
#define	PCAPS		0x33			/* Caps Lock							*/
#define	PCLEAR		0x40			/* Clear								*/
#define PDIAGON		0x41			/* make diag mode on					*/
#define PDIAGOFF	0x42			/* make diag mode off					*/
#define	PCALL		0x44			/* Confirmation							*/
#define	PCBS		0x45			/* Confirmation							*/
#define	PCCHAR		0x46			/* Confirmation							*/
#define	PCDEL		0x47			/* Confirmation							*/
#define	PCHKC		0x48			/* Confirmation							*/
#define	PSETCUR		0x4c			/* set cursor position					*/
#define	MAXPSEUDO	0x50

	/********************************************************************/
	/*	constants definition for Ex. Mon return code					*/
	/********************************************************************/

#define	EXMONNOERR		 0
#define	EXMONALLOCERR	-1
#define	EXMONPARMERR	-2
#define	EXMONNOTTERM	-3
#define	EXMONNOTOPEN	-4
#define	EXMONNOTINIT	-5
