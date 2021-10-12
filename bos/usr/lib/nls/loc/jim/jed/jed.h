/* @(#)66	1.3.1.2  src/bos/usr/lib/nls/loc/jim/jed/jed.h, libKJI, bos411, 9428A410j 5/18/93 05:24:05	*/
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : jed internal header
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1992, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _h_jed_
#define _h_jed_

/*-----------------------------------------------------------------------*
*	type definitions
*-----------------------------------------------------------------------*/
typedef int BOOL;

/*-----------------------------------------------------------------------*
*	constant definitions
*-----------------------------------------------------------------------*/
/*********************/
/* generic constants */
/*********************/
#define ON	1
#define	OFF	0
#define TRUE	1
#define FALSE	0

/*****************************/
/* return code to the caller */
/*****************************/
#define KP_OK	0	/* Return Code (successful) */
#define KP_ERR	-1	/* Return Code (error)      */

/******************/
/* aux buf format */
/******************/
#define KP_LONGAUX 0
#define KP_SHORTAUX 1

/********************************/
/* return values of kpProcess() */
/********************************/
#define KP_USED		0	/* passed keysym has been used */
#define KP_NOTUSED	1	/* passed keysym has not been used */
#define KP_UP           2       /* cursor up comes */
#define KP_DOWN         3       /* cursor down comes */
#define KP_REGISTRATION 4       /* (runtime registration) touroku */

/***********************************/
/* return values of kpGetAuxType() */
/***********************************/
#define KP_AUX_NOTUSED	0       /* there is no aux area used */
#define KP_AUX_ALLCAND	1       /* all candidate list        */
#define KP_AUX_KANJINO	2       /* kanji number input        */
#define KP_AUX_MODESET	3       /* mode set window           */

/****************************************************/
/* definitions for the operation of the kpControl() */
/****************************************************/
#define KP_CHANGELEN    1       /* change overflow limit  */
#define KP_RESETAUX     2       /* reset (clear) aux area */
#define KP_SETLANG      3       /* specify DBCS or mix    */
#define KP_SETCURSOR    4       /* set cursor to arbitrary position */
/* constants for SETLANG command argument */
#define     KP_ONLYDBCS     0	    /* only DBCS code points allowed */
#define     KP_MIX          1       /* both SBCS/DBCS allowed        */

/**********************************************/
/* definitions for the inputmode value (indv) */
/**********************************************/
/* ind0 */
#define KP_ALPHANUM	0	/* alphanum input mode */
#define KP_KATAKANA	1	/* katakana input mode */
#define KP_HIRAGANA	2	/* hiragana input mode */
/* ind1 */
#define KP_SINGLE	0	/* hankaku input mode */
#define KP_DOUBLE	1	/* zenkaku input mode */
/* ind2 */
#define KP_ROMAJI_OFF	0	/* romaji off input mode */
#define KP_ROMAJI_ON	1	/* romaji on input mode  */
/* ind3 */
#define KP_NORMALMODE   0       /* normal mode     */
#define KP_SUPPRESSEDMODE 1     /* suppressed mode */
/* ind4 */
/* note that there is no "replace" mode within JIMED Release 1.0 */
#define KP_REPLACEMODE   0      /* replace mode NEVER USED by JIM Ver 1.0 */
#define KP_INSERTMODE    1      /* JIM is always in this mode             */

/********************************/
/* definnition of the InputMode */
/********************************/
typedef	struct	{
	unsigned	ind0	: 4;   /* alpha/hira/kata     */
	unsigned	ind1	: 4;   /* single or double    */
	unsigned	ind2	: 4;   /* romaji on or off    */
	unsigned        ind3    : 4;   /* normal/suppressed   */
	unsigned        ind4    : 4;   /* insert/replace mode */
}	InputMode;

/*********************************************/
/* constant defintions for profile structure */
/*********************************************/
/* convmode */
#define KP_CONV_WORD 0
#define KP_CONV_SPHRASE 1
#define KP_CONV_MPHRASE 2
#define KP_CONV_LOOKAHEAD 3

/* kjbeep */
#define KP_BEEPON 1
#define KP_BEEPOFF 0

/* learn */
#define KP_LEARNON 1
#define KP_LEARNOFF 0

/* alphanum */
#define KP_ALPHAON 1
#define KP_ALPHAOFF 0

/* kuten code */
#define KP_KUTEN78	0
#define KP_KUTEN83	1

/* udload */
#define	KP_UDLOADON	1
#define	KP_UDLOADOFF	0

/* numberinput code */
#define KP_JISKUTEN	0
#define KP_IBMKANJI	1

/* modereset */
#define	KP_RESETON	1
#define	KP_RESETOFF	0

#define SDICT_NUM 16  	/* Max counts of system dictionaries    */
#define SDICT_LEN 80  	/* Max length of system dictionary name */

/*********************/
/* profile structure */
/*********************/
typedef struct {
    char *sys[SDICT_NUM+1];     /* system dict. file name */
    char *user;         	/* user dict. file name   */
    char *adj;          	/* adjunct dict. file name*/
    } DictNames;

typedef struct {
    int convmode;	/* initial conversion mode              */
    int kjbeep;         /* beep is/is not requested by JIMED    */
    int learn;          /* to save learning information or not  */
    int alphanum;       /* to allow alphanum conversion or not  */
    DictNames dictstru; /* dictionary name structure */
    int kuten;		/* Kuten Number Mode */
    int	shmatnum;	/* Max JIM can use the number of shmat          */
    int	udload;		/* load User dictionary or not                  */
    int	numberinput;	/* initial state for JISkuten/IBMKanji number   */
    int	modereset;	/* reset input mode with ESC key                */
    } jedprofile;

/***********************/
/* auxiliary area size */
/***********************/
typedef struct {
    int itemsize;		/* length (byte) of each aux item */
    int itemnum;		/* number of item (line)          */
    } AuxSize;

/*************************************/
/* change information in echo buffer */
/*************************************/
/* the last two fields contain valid valies only when flag is TRUE */
typedef struct {
    BOOL flag;		/* TRUE when there is chagne, FLASE otherwise       */
    int  chtoppos;      /* the first position of change in echo buf in byte */
    int  chlenbytes;    /* changed length from the first position in byte   */
    } EchoBufChanged;

/*********************************/
/* auxiliary are cursor position */
/* both starting from zero       */
/*********************************/
typedef struct {
    int colpos;		/* column position (byte) within a item (line) */
    int rowpos;         /* row (which item) number couting from zero   */
    } AuxCurPos;

/***********************************/
/* highlight attribute definitions */
/***********************************/
#define KP_HL_NORMAL 0x00
#define KP_HL_UNDER 0x01
#define KP_HL_REVERSE 0x02

#endif /* _h_jed_ */
