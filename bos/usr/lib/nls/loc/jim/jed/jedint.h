/* @(#)79	1.4.1.1  src/bos/usr/lib/nls/loc/jim/jed/jedint.h, libKJI, bos411, 9428A410j 7/23/92 01:49:12	*/
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

#include "im.h"
#include "imP.h"

#ifndef _h_jedint_
#define _h_jedint_
/*-----------------------------------------------------------------------*
*	structure definitions
*-----------------------------------------------------------------------*/
/********************************/
/* JIMED internal control block */
/********************************/

typedef struct {
	KCB             *kcb ;      /* pointer to KCB structure             */
	unsigned char   *echobufs ; /* pointer to echo buff.                */
	unsigned char   *echobufa ; /* pointer to echo att. buff.           */
	unsigned char   **auxbufs ; /* pointer to aux-area buff. array      */
	unsigned char   **auxbufa ; /* pointer to aux-area att. buff. array */
	EchoBufChanged  echochfg ;  /* changed flag for echo buff.          */
				    /*  also contains where changed         */
	int             echocrps ;  /* cursor position in echo buff.        */
	int             eccrpsch ;  /* changed flag for cursor pos. (echo)  */
	int             echoacsz ;  /* active length in echo buff.          */
	int             echosize ;  /* echo buff. size                      */
	int             echoover;   /* overflow limit within echo buffer    */
	int             auxchfg ;   /* changed flag for aux-area buff.      */
	int             auxuse  ;   /* aux-area use flag                    */
  	int             axconvsw ;  /* aux-area convsw state flag           */
	AuxCurPos       auxcrps ;   /* cursor position in aux-area buff.    */
	int             axcrpsch ;  /* changed flag for cursor pos. (aux)   */
	AuxSize         auxacsz ;   /* active length in aux-area buff.      */
	AuxSize         auxsize ;   /* aux-area buff. size                  */
	char            convmode ;  /* conversion mode                      */
	char            tconvmode ; /* conversion mode                      */
	char            shift[3] ;  /* shift state                          */
	char		alpha;      /* PROCESSOFF : alpha or katakana       */
	int             indchfg ;   /* changed flag for indicator buff.     */
	InputMode	imode;	    /* input mode                           */
	int             isbeep;     /* is beep requested last time          */
	int             beepallowed;/* is beep allowed ?                    */
	int             dbcsormix;  /* DBCS only or mix                     */
	int		auxformat;  /* aux buffer format                    */
	int		selection;  /* selection is supported ?             */
	IMPanel		impanel;    /* IMPanel for list box                 */
	char		title[40];  /* title (SJIS) for list box            */
}    FEPCB ;

/*-----------------------------------------------------------------------*
*	constant definitions
*-----------------------------------------------------------------------*/
/**************************************/
/* required size for echo, aux buffer */
/**************************************/
#define MINECHO         45      /* Mimimum size of Echo Buffer   */
/* we will have to change following limits for multi line format */
/* for one line format */
#define MINAUX          56      /* Mimimum size of Aux Buffer    */
                                /* determined by one line conversion switch */
/* for multi line format */
#define AUXITEMNUM 5            /* minimum number of item in multi line aux */
#define AUXITEMLEN 22		/* minimum length (byte) of aux item        */

/* conversion switch size */
#define MCONVAUXNUM 5
#define MCONVAUXLEN 20
 
/***********************************/
/* constants between Kanji Monitor */
/***********************************/
#define CSIDJ           370     /* CSID for Japanese      */
#define INIT_CONV       1       /* conversion flag in KCB */

#define AX_NONUSE       -1      /* Aux type returned by kjauxkind() */
#define AX_ALLCAN       0       /* Aux type returned by kjauxkind() */
#define AX_KJCODE       1       /* Aux type returned by kjauxkind() */
#define AX_CONVSW       2       /* Aux type returned by kjauxkind() */

/************************************************/
/* DBCS Hiragana of the first, and the last one */
/************************************************/
#define HIRAGANA_A      0x829f
#define HIRAGANA_N      0x82f1

/**********************************/
/* JIMED process, internal states */
/**********************************/
/* we need some of new states */
#define PROCESSOFF      0       /* suppressed mode                         */
#define INACTIVE        1       /* there is no char in echo buf, rkc stack */
#define ACTIVATING      2       /* no char in echo , but some in rkc stack */
#define ACTIVE          3       /* there is some chars in echo buf         */
#define CONVSW          4       /* conv mode swtich window is on           */
#define ACTKNJ          5       /* kanji number while active               */
#define INACTKNJ        6       /* kanji number while inaction             */
#define ALLCANDS        7       /* all candidates list                     */

/***************************************/
/* key categories determined by string */
/***************************************/
#define KEY_SHIFT	0	/* A/K/H, RKC, F/H shift */
#define KEY_CHAR	1	/* characters */
#define KEY_CONV	2	/* convert key */
#define KEY_NOCONV	3	/* no-convert key, bunsetsu yomi key */
#define KEY_PRECAND	4	/* previous candidate key */
#define KEY_ALLCAND	5	/* all candidate key */
#define KEY_KJNUM	6	/* kanji number input key */
#define KEY_CONVSW	7	/* conversion mode switch key */
#define KEY_REGST	8	/* word registration key */
#define KEY_ENTER	9	/* enter key */
#define KEY_BACKSP	10	/* back space key */
#define KEY_CURSOR	11	/* cursor left/right */
#define KEY_ERASE	12	/* ErEOF, ErInput, Delete keys */
#define KEY_RESET	13	/* reset key */
#define KEY_CURUP	14	/* cursor up key */
#define KEY_CURDOWN	15	/* cursor down key */
#define KEY_OTHERS	16	/* others */

#define	EUC_SS2		0x8e
#define	EUC_SS3		0x8f

#define	ES_KANA		0x20
#define	ES_RKC		0x40

#endif /* _h_jedint_ */
