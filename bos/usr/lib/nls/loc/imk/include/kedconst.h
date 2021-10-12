/* @(#)44        1.2  src/bos/usr/lib/nls/loc/imk/include/kedconst.h, libkr, bos411, 9428A410j 3/23/93 20:31:08 */
/*
 * COMPONENT_NAME :	(KRIM) - AIX Input Method
 *
 * FUNCTIONS :		kedconst.h
 *
 * ORIGINS :		27
 *
 * (C) COPYRIGHT International Business Machines Corp.  1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _h_kedconst_
#define _h_kedconst_

/*********************/
/* generic constants */
/*********************/
#define ON      1
#define OFF     0
#define TRUE    1
#define FALSE   0

/*****************************/
/* return code to the caller */
/*****************************/
#define KP_OK   0       /* Return Code (successful) */
#define KP_ERR  -1      /* Return Code (error)      */

/******************/
/* aux buf format */
/******************/
#define KP_LONGAUX 0
#define KP_SHORTAUX 1

/********************************/
/* return value of kedProcess() */
/********************************/
#define KP_USED         0       /* passed keysym has been used        */
#define KP_NOTUSED      1       /* passed keysym caused output string */
#define KP_UP           2       /* cursor up comes                    */
#define KP_DOWN         3       /* cursor down comes                  */

/***********************************/
/* return values of kedGetAuxType() */
/***********************************/
#define AUXBUF_NOTUSED  0	/* there is no aux area used */
#define MULTICAND_USED  1 	/* all candidate list        */
#define CODEINPUT_USED	2	/* code input 	             */

/****************************************************/
/* definitions for the operation of the kpControl() */
/****************************************************/
#define KP_CHANGELEN    1       /* change overflow limit  */
#define KP_RESETAUX     2       /* reset (clear) aux area */
#define KP_SETLANG      3       /* specify DBCS or mix    */
#define KP_SETCURSOR    4       /* set cursor to arbitrary position */

#define     KP_ONLYHANGUL  0       /* only hangul code points allowed */

/***************************************/
/* definitions for the inputmode value */
/***************************************/
#define MD_ENG		0
#define MD_HAN		1
#define MD_JAMO		2

#define MD_BANJA	0
#define MD_JEONJA	1

#define MD_HJOFF		0
#define MD_HJON			1

#define MD_SUPP		0
#define MD_NORM		1

#define MD_REPLACE	0	/* replace mode NEVER USED by KIM Ver 1.0 */
#define MD_INSERT	1	/* KIM is always in this mode             */

/************************/
/* constant defintions  */
/************************/

/* beep */
#define KP_BEEPON 1
#define KP_BEEPOFF 0

/* learn */
#define KP_LEARNON 1
#define KP_LEARNOFF 0

/* acm */
#define KP_ACMON 1
#define KP_ACMOFF 0

/* alhpanum */
#define KP_ALPHAON 1
#define KP_ALPHAOFF 0

/* shift */
#define shift_ALT      0x8
#define shift_CAPS_ALT 0xa
#define shift_CTRL     0x4

/***********************************/
/* highlight attribute definitions */
/***********************************/
#define KP_HL_NORMAL	0x00
#define KP_HL_UNDER	0x01
#define KP_HL_REVERSE	0x02

/**********************************************
 * modes which appear on the indicator
 **********************************************/

/* basemode */
#define	ALPHANUM        1
#define	HANGUL		2
#define	JAMO		3

/* sizemode */
#define	SINGLE		1
#define	DOUBLE		2

/* hjmode */
#define	HANJA_ON	1
#define HANJA_OFF	2

/* insrepmode */
#define INSERT          1
#define REPLACE         0


/**************************************/
/* required size for echo, aux buffer */
/**************************************/
#define MINECHO         45      /* Mimimum size of Echo Buffer   */

/* for one line format */
#define MINAUX          56      /* Mimimum size of Aux Buffer    */
                                /* determined by one line conversion switch */
/* for multi line format */
#define AUXITEMNUM 10            /* minimum number of item in multi line aux */
#define AUXITEMLEN 21		/* minimum length (byte) of aux item        */

/* maximum length of eum  */
#define MAXEUMLEN   20

/* length of code input field */
#define CODEINPLEN  6

/* 6 blank string to clear code input area when enter key is pressed */
#define	CLEAR_CODEINPUT	"\040\040\040\040\040\040"
 
/**********************************/
/* KIMED process, internal states */
/**********************************/

#define ST_ENG		0
#define ST_HAN		1
#define ST_CODEINP	2
#define ST_JAMO		3
#define ST_SING		4
#define ST_MULTI	5

/* hangeul state  definition */
#define HG_ST_FINAL	0
#define HG_ST_INIT	1
#define HG_ST_INTERIM	2
#define HG_ST_DELETE    7	/* State_Delete in hgcomp.h */

/****************************************/
/* KIM editor's buffer info.		*/
/****************************************/
#define	KIMED_FIXBUF_EMTY		0
#define KIMED_ECHOBUF_EMTY		0
#define KIMED_CURADV_FIRSTPOS		0
#define KIMED_ECHOSVCH_EMTY		0
#define KIMED_ECHOSVCH_SP		-1
#define KIMED_ECHOBUF_SP		0
#define KIMED_CANDBUF_EMTY		0
#define KIMED_CANDCR_FIRSTPOS		0
#define KIMED_NOCHANGED_LEN		0
#define KIMED_AUXBUF_EMTY		0
#define KIMED_AUXCR_FIRST		-1

/***************************************/
/* key categories determined by string */
/***************************************/

#define KEY_FN1		1
#define KEY_FN2		2
#define KEY_SYSTEM	3
#define KEY_SHIFT	4
#define KEY_HJUNI	5
#define KEY_ENTER	6
#define KEY_MODE	7
#define KEY_CHAR	8

/****************/
/* Udict const. */
/****************/
#define	USRDICT		0
#define SYSDICT		(USRDICT+1)
#define STATLENGTH	2

/************/
/* MRU size */
/************/
#define MRUSIZE		2048

/****************/
/* Block size   */
/****************/
#define SIXBLOCK_SIZE	3072
#define SDBLOCK_SIZE	1024
#define UIXBLOCK_SIZE	2048
#define UDBLOCK_SIZE	1024

/**************/
/* Udict mode */
/**************/
#define UD_NONE		0
#define UD_CREAT	1
#define UD_RDONLY	2
#define UD_RDWR		4
#define UD_DICTUT	8

/************************/
/* current file desc is */
/* valid.	   	*/
/************************/
#define VALID_FDESC	1
#define INVALID_FDESC	-1

#endif _h_kedconst_
