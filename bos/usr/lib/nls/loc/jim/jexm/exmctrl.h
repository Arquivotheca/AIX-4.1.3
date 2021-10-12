/* @(#)83	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jexm/exmctrl.h, libKJI, bos411, 9428A410j 7/23/92 01:50:14 */
/*
 * COMPONENT_NAME :	Japanese Input Method - Ex Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  ExMon version 6.0       06/28/88
 */

#include <stdio.h>
#include <sys/types.h>
#include "exmkmisa.h"

#define SDICT_NUM 16  		/* Max counts of system dictionaries    */
#define SDICT_LEN 80  		/* Max length of system dictionary name */

/* dict name structure added 5/27/92 */
typedef struct {
    char *sys[SDICT_NUM+1];     /* system dict. file name 		*/
    char *user;         	/* user dict. file name   		*/
    char *adj;          	/* adjunct dict. file name		*/
    } DICTS;

	/********************************************************************/
	/*	EXTINF/MEXTINF	Extended information				*/
	/********************************************************************/

typedef	struct	_EXTINF	{
	KMPF	*prokmpf;	/* ptr to KMPF structure	*/
	short	maxstc;				/* max number of columns                    */
	short	maxstr;				/* max number of rows                       */
	short	maxa1c;				/* max number of columns of aux area no.1   */
	short	maxa1r;				/* max number of rows of aux area no.1      */
	short	maxa2c;				/* max number of columns of aux area no.2   */
	short	maxa2r;				/* max number of rows of aux area no.2      */
	short	maxa3c;				/* max number of columns of aux area no.3   */
	short	maxa3r;				/* max number of rows of aux area no.3      */
	short	maxa4c;				/* max number of columns of aux area no.4   */
	short	maxa4r;				/* max number of rows of aux area no.4      */
	DICTS   *dicts;                        /* dict name str address 6/28/89 */
	long	rsv2;				/*** reserved for future use              ***/
	long	rsv3;				/*** reserved for future use              ***/
	long	rsv4;				/*** reserved for future use              ***/
}	EXTINF;

	/********************************************************************/
	/*	KJSVPT	internal save area										*/
	/********************************************************************/

typedef	struct	_PHASE1	{
		int		diaflg;					/* diagnosis mode flag				*/
		int		ncodes;					/* number of input codes			*/
		char	type[P1BUFSIZE];		/* array of input types				*/
		char	code[P1BUFSIZE];		/* array of input codes				*/
		int		rlen;					/* length of romaji input			*/
		int		klen;					/* length of kana output			*/
		int		rslen;					/* length of rest					*/
		char	romaji[P1BUFSIZE];		/* romaji input area                */
		char	kana[P1BUFSIZE];		/* kana output area                 */
		char	rest[P1BUFSIZE];		/* rest area                        */
}	PHASE1;

typedef	struct	_PHASE2	{
		int		ncodes;					/* number of input codes			*/
		char	type[P2BUFSIZE];		/* array of input types				*/
		char	code[P2BUFSIZE];		/* array of input codes				*/
		int		state;					/* EXMON internal state             */
		int		prevstate;				/* state before ACI                 */
		int		nextstate;				/* state after ACI                  */
		int		curst;					/* cursor position in STRING		*/
		int		lastst;					/* last position in STRING			*/
		int		curis;					/* cursor position in ISTRING		*/
		int		lastis;					/* last position in ISTRING			*/
		int		topim;					/* MSIMAGE start position in STRING */
		int		curim;					/* cursor poition in MSIMAGE		*/
		int		lastim;					/* last position in MSIMAGE			*/
		int		lastsd;					/* last position in SDTTBL			*/
		int		moncall;				/* whether called the Monitor or not*/
		short	setcsc;					/* set cursor position				*/
		char	shift3;					/* Single/Double byte char shift    */
		char	repins;					/* replace/insert flag              */
		char	axuse1;					/* aux area 1 use flag              */
		char	beep;					/* beep flag                        */
		char	discrd;					/* discard flag                     */
}	PHASE2;

typedef	struct	_BUFFER	{
	char	*string;					/* ptr to STRING					*/
	char	*hlatst;					/* ptr to HLATST					*/
	int		actcol;						/* active input field length		*/
	char	*istring;					/* ptr to ISTRING                   */
	char	*ihlatst;					/* ptr to IHLATST                   */
	int		maxis;						/* max length of ISTRING			*/
	char	*msimage;					/* ptr to MSIMAGE                   */
	char	*mhimage;					/* ptr to MHIMAGE                   */
	int		maxim;						/* max length of MSIMAGE			*/
	char	*sdttbl;					/* ptr to SDTTBL                    */
	char	*sdtwork;					/* ptr to SDTTBL work area          */
	int		maxsd;						/* max length of SDTTBL				*/
}	BUFFER;

typedef	struct	_KJSVPT	{
	struct	_KJCBLK	*mkjcblk;			/* ptr to MKJCBLK					*/
	int		initflg;					/* whether initialized or not		*/
	int		beep;						/* beep available flag				*/
	int		normalbs;					/* back spacing						*/
	int		dollar;						/* '$' input supress flag			*/
	BUFFER	buffer;						/* buffer structure					*/
	EXTINF	extinf;						/* extended information EXTINF		*/
	EXTINF	mextinf;					/* extended information	MEXTINF		*/
	PHASE1	phase1;						/* phase1 structure					*/
	PHASE2	phase2;						/* phase2 structure					*/
}	KJSVPT;

	/********************************************************************/
	/*	KJCBLK/MKJCBLK	a suructure of the Kanjk Control Block			*/
	/********************************************************************/

typedef	int		TRB;

typedef	struct	_KJCBLK	{
	long	length;				/* length of Kanji Control Block (byte).    */
	long	id;					/* Kanji Monitor ID                         */
	KJSVPT	*kjsvpt;			/* ptr to kjmonitor internal save area      */
	TRB		*tracep;			/* Ptr to Trace Area.(TRB)					*/
	char	*string;			/* ptr to Input Field                       */
	char	*hlatst;			/* ptr to highlight attribute area          */
	char	*aux1;				/* ptr to auxiliary area no.1               */
	char	*hlata1;			/* ptr to aux area no.1 highlight attribute */
	char	*aux2;				/* ptr to auxiliary area no.2               */
	char	*hlata2;			/* ptr to aux area no.2 highlight attribute */
	char	*aux3;				/* ptr to auxiliary area no.3               */
	char	*hlata3;			/* ptr to aux area no.3 highlight attribute */
	char	*aux4;				/* ptr to auxiliary area no.4               */
	char	*hlata4;			/* ptr to aux area no.4 highlight attribute */
	char	*auxdir;			/* Pointer to auiliary area.				*/
	short	csid;				/* character set id                         */
	short	actcol;				/* byte length of active input field        */
	short	actrow;				/* num. of rows in active input field       */
	short	ax1col;				/* a num of columns in the aux area 1       */
	short	ax1row;				/* a num of rows in the aux area 1          */
	short	ax2col;				/* a num of columns in the aux area 2       */
	short	ax2row;				/* a num of rows in the aux area 2          */
	short	ax3col;				/* a num of columns in the aux area 3       */
	short	ax3row;				/* a num of rows in the aux area 3          */
	short	ax4col;				/* a num of columns in the aux area 4       */
	short	ax4row;				/* a num of rows in the aux area 4          */
	short	maxa1c;				/* max num of columns in the aux area 1		*/
	short	maxa1r;				/* max num of rows in the aux area 1		*/
	short	maxa2c;				/* max num of columns in the aux area 2		*/
	short	maxa2r;				/* max num of rows in the aux area 2		*/
	short	maxa3c;				/* max num of columns in the aux area 3		*/
	short	maxa3r;				/* max num of rows in the aux area 3		*/
	short	maxa4c;				/* max num of columns in the aux area 4		*/
	short	maxa4r;				/* max num of rows in the aux area 4		*/
	short	curcol;				/* cursor position (offset)                 */
	short	currow;				/* cursor position in row                   */
	short	setcsc;				/* cursor position                          */
	short	setcsr;				/* cursor position in row                   */
	short	cura1c;				/* pseudo cursor position in aux area 1     */
	short	cura1r;				/* pseudo cursor position in aux area 1     */
	short	cura2c;				/* pseudo cursor position in aux area 2     */
	short	cura2r;				/* pseudo cursor position in aux area 2     */
	short	cura3c;				/* pseudo cursor position in aux area 3     */
	short	cura3r;				/* pseudo cursor position in aux area 3     */
	short	cura4c;				/* pseudo cursor position in aux area 4     */
	short	cura4r;				/* pseudo cursor position in aux area 4     */
	short	chpos;				/* changed character position               */
	short	chlen;				/* changed character length                 */
	short	chpsa1;				/* changed char position in aux area 1      */
	short	chlna1;				/* changed char length in aux area 1        */
	short	chpsa2;				/* changed char position in aux area 2      */
	short	chlna2;				/* changed char length in aux area 2        */
	short	chpsa3;				/* changed char position in aux area 3      */
	short	chlna3;				/* changed char length in aux area 3        */
	short	chpsa4;				/* changed char position in aux area 4      */
	short	chlna4;				/* changed char length in aux area 4        */
	short	lastch;				/* the last character position              */
	char	type;				/* input code type                          */
	char	code;				/* input code                               */
	char	flatsd;				/* input field attribute                    */
	char	axuse1;				/* aux area 1 use flag                      */
	char	axuse2;				/* aux area 2 use flag                      */
	char	axuse3;				/* aux area 3 use flag                      */
	char	axuse4;				/* aux area 4 use flag                      */
	char	ax1loc;				/* aux area 1 default location              */
	char	ax2loc;				/* aux area 2 default location              */
	char	ax3loc;				/* aux area 3 default location              */
	char	ax4loc;				/* aux area 4 default location              */
	char	indlen;				/* byte length of shift indicators          */
	char	shift;				/* shift changejflag                        */
	char	shift1;				/* AN/Kana shift                            */
	char	shift2;				/* RKC shift                                */
	char	shift3;				/* Single/Double byte char shift            */
	char	shift4;				/* reserved for future use                  */
	char	curlen;				/* cursor length                            */
	char	cnvsts;				/* conversion status                        */
	char	repins;				/* replace/insert flag                      */
	char	beep;				/* beep flag                                */
	char	discrd;				/* discard flag                             */
	char	trace;				/* Trace flag.								*/
	char	conv;				/* Conversion Flag.							*/
	long	rsv1;				/* reserved for future use                  */
	long	rsv2;				/* reserved for future use                  */
	long	rsv3;				/* reserved for future use                  */
	long	rsv4;				/* reserved for future use                  */
	long	rsv5;				/* reserved for future use                  */
	long	rsv6;				/* reserved for future use                  */
	long	rsv7;				/* reserved for future use                  */
	long	rsv8;				/* reserved for future use                  */
}	KJCBLK;
