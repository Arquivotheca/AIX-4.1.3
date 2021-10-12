/* @(#)78	1.4.1.1  src/bos/usr/lib/nls/loc/jim/jed/jedexm.h, libKJI, bos411, 9428A410j 7/23/92 01:48:27	*/
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _h_jedexm_
#define _h_jedexm_

#include "kmpf.h"
/*
 *	constants definition for parameters in Kanji Control Block
 */

/*	type		*/
#define	TYPE1		1
#define	TYPE2		2

/*	flatsd		*/
#define	ONLYDOUBLE	1
#define	MIXEDMODE	2

/*	axuse1		*/
#define USE		1
#define NOTUSE		0

/*	shift		*/
#define	SHIFT1		0x01
#define	SHIFT2		0x02
#define	SHIFT3		0x04

/*	shift1		*/
#define	ALPHANUM	1
#define	KATAKANA	2
#define	HIRAGANA	3

/*	shift2		*/
#define	RKC_ON		1
#define	RKC_OFF		2

/*	shift3		*/
#define	SINGLE		1
#define	DOUBLE		2

/*	curlen		*/
#define	CURATS		1
#define	CURATD		2

/*	cnvsts		*/
#define	FINISHED	0
#define	GOINGON		1
#define	OVERFLOW	2

/*	repins		*/
#define INSERT		1
#define	REPLACE		0

/*	beep		*/
#define	OFF		0
#define	ON		1

/*	discrd		*/
#define	NOTDISCARD	0
#define	DISCARD		1

/*	for IHLATST, MHIMAGE, MHLATST and HLATST	*/
#define	HLMASK		0x0f
#define	NOHIGHLIGHT	0xf0
#define	REVERSE		0x01
#define	UNDERSCORE	0x02
#define	KJMASK		0xf0
#define	KJ1st		0x10
#define	KJ2nd		0x20
#define	JISCII		0x30

/*	for Conversion Mode.	*/

#define	LOOKAHEAD	0
#define	MPHRASE		1
#define	SPHRASE		2
#define	WORD		3

/*	for IHLATST, MHIMAGE, MHLATST and HLATST	*/

#define	HLMASK		0x0f

/*
 *	definition of pseudo code
 */

#define PKATAKANA	0x01	/* Katakana shift			*/
#define PALPHANUM	0x02	/* Alpha_Numeric shift			*/
#define	PHIRAGANA	0x03	/* Hiragana shift			*/
#define	PRKC		0x04	/* RKC shift				*/
#define	PCONVERT	0x05	/* Convert key				*/
#define	PNOCONV		0x06	/* No_Convert key			*/
#define	PALLCAND	0x07	/* All_Candidates			*/
#define	PREGST      	0x08	/* reserved for word registration	*/
#define	PKJNUM		0x09	/* Kanji Number Input			*/
#define	PCONVSW		0x0a	/* Convert Mode Switch			*/
#define	PDIAGNOS	0x0b	/* Diagnosis				*/
#define	PPRECAND	0x0c	/* Previous Candidates			*/
#define	PSGLDBL		0x0d	/* single / double toggle		*/
#define	PYOMI		0x12	/* Bunsetsu-Yomi			*/
#define	PENTER		0x20	/* Enter				*/
#define	PACTION		0x21	/* Action				*/
#define	PCR		0x22	/* Carrige Return			*/
#define	PRESET		0x23	/* Reset				*/
#define	PCURRIGHT	0x24	/* Cursor Right				*/
#define	PCURLEFT	0x25	/* Cursor Left				*/
#define	PCURUP		0x26	/* Cursor Up				*/
#define	PCURDOWN	0x27	/* Cursor Down				*/
#define	PCURDRIGHT	0x28	/* Cursor Double Right			*/
#define	PCURDLEFT	0x29	/* Cursor Double Left			*/
#define	PEREOF		0x2a	/* ErEOF				*/
#define	PERINPUT	0x2b	/* ErInput				*/
#define	PINSERT		0x2c	/* Insert toggle			*/
#define	PDELETE		0x2d	/* Delete Character			*/
#define	PBACKSPACE	0x2e	/* Back Space				*/
/* we will add control/cursor left/right */
#define PCTRCURRIGHT    0x42    /* control cursor right                 */
#define PCTRCURLEFT     0x43    /* control cursor left                  */

/*
 *	definition of ESC sequences for pseudo code
 */

#define ESCKATAKANA	"\033[001z"	/* 0x01 katakana shift		*/
#define ESCALPHANUM	"\033[002z"	/* 0x02 Alpha_Numeric shift	*/
#define	ESCHIRAGANA	"\033[003z"	/* 0x03 Hiragana shift		*/
#define	ESCRKC		"\033[004z"	/* 0x04 RKC shift		*/
#define	ESCCONVERT	"\033[005z"	/* 0x05 Convert key		*/
#define	ESCNOCONV	"\033[006z"	/* 0x06 No_Convert key		*/
#define	ESCALLCAND	"\033[007z"	/* 0x07 All_Candidates		*/
#define	ESCREGST	"\033[008z"	/* 0x08 reserverd for word reg  */
#define	ESCKJNUM	"\033[009z"	/* 0x09 kanji number            */
#define	ESCCONVSW	"\033[010z"	/* 0x0a Convert Mode Switch	*/
#define	ESCPRECAND	"\033[012z"	/* 0x0c Previous Candidates	*/
#define	ESCSGLDBL	"\033[013z"	/* 0x0d single / double toggle	*/
#define	ESCYOMI		"\033[018z"	/* 0x12 Bunsetsu-Yomi		*/
#define	ESCACTION	"\033[114q"	/* 0x21 Action			*/
#define	ESCRESET	"\033[121q"	/* 0x23 Reset (Control/ESC)	*/
#define	ESCCURRIGHT	"\033[C"	/* 0x24 Cursor Right		*/
#define	ESCCURLEFT	"\033[D"	/* 0x25 Cursor Left		*/
#define	ESCCURUP	"\033[A"	/* 0x26 Cursor Up		*/
#define	ESCCURDOWN	"\033[B"	/* 0x27 Cursor Down		*/
#define	ESCCURDRIGHT	"\033[169q"	/* 0x28 Cursor Double Right	*/
#define	ESCCURDLEFT	"\033[160q"	/* 0x29 Cursor Double Left	*/
#define	ESCEREOF	"\033[146q"	/* 0x2a ErEOF			*/
#define	ESCERINPUT	"\033[149q"	/* 0x2b ErInput			*/
#define	ESCDELETE	"\033[P"	/* 0x2d Delete Character	*/
#define	BS	0x08
#define	CR	0x0d
/* we will add control cursor right and left */
#define ESCCTRCURRIGHT  "\033[168q"     /* control cursor right         */
#define ESCCTRCURLEFT   "\033[159q"     /* control cursor left          */

/*
 *	constants definition for Ex. Mon return code
 */

#define	EXM_NOERR	 0
#define	EXM_ALLOCERR	-1
#define	EXM_PARMERR	-2
#define	EXM_NOTTERM	-3
#define	EXM_NOTOPEN	-4
#define	EXM_NOTINIT	-5

/* we will add a pointer to structure which contains pointers to*/

#define SDICT_NUM 16  		/* Max counts of system dictionaries    */
#define SDICT_LEN 80  		/* Max length of system dictionary name */

typedef struct {
    char *sys[SDICT_NUM+1];     /* system dict. file name       	*/
    char *user;         	/* user dict. file name   		*/
    char *adj;          	/* adjunct dict. file name		*/
    } DICTS;

/*
 *	EXT			Extended information
 */

typedef	struct	{
	KMPF	*prokmpf;	/* ptr to KMPF structure		    */
	short	maxstc;		/* max number of columns                    */
	short	maxstr;		/* max number of rows                       */
	short	maxa1c;		/* max number of columns of aux area no.1   */
	short	maxa1r;		/* max number of rows of aux area no.1      */
	short	rsr1[6];	/* reserved				    */
	DICTS   *dicts;         /* dict name structure addresses            */
	long	rsr2[3];	/* reserved				    */
}	EXT;

/*
 *	KCB		a suructure of the Kanjk Control Block
 */

typedef	struct	{
	long	length;		/* length of Kanji Control Block (byte).    */
	long	id;		/* Kanji Monitor ID                         */
	int	rsv1[2];	/* reserved				    */
	char	*string;	/* ptr to Input Field                       */
	char	*hlatst;	/* ptr to highlight attribute area          */
	char	*aux1;		/* ptr to auxiliary area no.1               */
	char	*hlata1;	/* ptr to aux area no.1 highlight attribute */
	int	rsv2[7];	/* reserved				    */
	short	csid;		/* character set id                         */
	short	actcol;		/* byte length of active input field        */
	short	actrow;		/* num. of rows in active input field       */
	short	ax1col;		/* a num of columns in the aux area 1       */
	short	ax1row;		/* a num of rows in the aux area 1          */
	short	rsv3[6];	/* reserved				    */
	short	maxa1c;		/* max num of columns in the aux area 1	    */
	short	maxa1r;		/* max num of rows in the aux area 1	    */
	short	rsv4[6];	/* reserved				    */
	short	curcol;		/* cursor position (offset)                 */
	short	currow;		/* cursor position in row                   */
	short	setcsc;		/* cursor position to be set		    */
	short	setcsr;		/* cursor position to be set		    */
	short	cura1c;		/* pseudo cursor position in aux area 1     */
	short	cura1r;		/* pseudo cursor position in aux area 1     */
	short	rsv6[6];	/* reserved				    */
	short	chpos;		/* changed character position               */
	short	chlen;		/* changed character length                 */
	short	chpsa1;		/* changed char position in aux area 1      */
	short	chlna1;		/* changed char length in aux area 1        */
	short	rsv7[6];	/* reserved				    */
	short	lastch;		/* the last character position              */
	char	type;		/* input code type                          */
	char	code;		/* input code                               */
	char	flatsd;		/* input field attribute                    */
	char	axuse1;		/* aux area 1 use flag                      */
	char	rsv8[8];	/* reserved				    */
	char	shift;		/* shift change flag                        */
	char	shift1;		/* AN/Kana shift                            */
	char	shift2;		/* RKC shift                                */
	char	shift3;		/* Single/Double byte char shift            */
	char	rsv9;		/* reserved				    */
	char	curlen;		/* cursor length                            */
	char	cnvsts;		/* conversion status                        */
	char	repins;		/* replace/insert flag                      */
	char	beep;		/* beep flag                                */
	char	discrd;		/* discard flag                             */
	char	rsv10;		/* reserved				    */
	char	conv;		/* Conversion Flag.			    */
	long	rsv11[8];	/* reserved				    */
}	KCB;

#endif /* _h_jedexm_ */
