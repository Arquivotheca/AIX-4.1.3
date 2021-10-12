/* @(#)43        1.2  src/bos/usr/lib/nls/loc/imk/include/ked.h, libkr, bos411, 9428A410j 3/23/93 20:30:44 */
/*
 * COMPONENT_NAME :	(libKR) - AIX Input Method
 *
 * FUNCTIONS :		ked.h
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

#ifndef _h_ked_
#define _h_ked_

#include "kedconst.h"
#include "Hhclist.h"
#include <sys/types.h>

typedef int BOOL;

/*********************************/
/* definitions for the InputMode */
/*********************************/
typedef struct {
        unsigned int    basemode        : 4;    /* HG/Eng/JAMO */
        unsigned int    sizemode        : 4;    /* Jeonja/Banja */
        unsigned int    hjmode          : 4;    /* HJ key ON/OFF        */
        unsigned int    supnormode      : 4;    /* Normal/Suppressed    */
        unsigned int    insrepmode      : 4;    /* Insert/Replace       */
} InputMode ;


/***********************/
/* auxiliary area size */
/***********************/
typedef struct {
    int itemsize;               /* length (byte) of each aux item */
    int itemnum;                /* number of item (line)          */
    } AuxSize;

/*********************/
/* File info for KIM */
/*********************/
typedef struct {
   ushort   status;
   int	    fdesc;
   int	    fdstat;
   int	    dbunit;
   int	    ixsz;
   ushort   *ixb;
}  DICTINFO;

/**************************************/
/* change information in echo buffer */
/**************************************/
/* te last two fields contain valid valies only when flag is TRUE */
typedef struct {
    BOOL flag;  /* TRUE when there is chagne, FLASE otherwise       */
    int  chtoppos;    /* the first position of change in echo buf in byte */
    int  chlenbytes;  /* changed length from the first position in byte   */
} EchoBufChanged;

/*********************************/
/* auxiliary are cursor position */
/* both starting from zero       */
/*********************************/
typedef struct {
    int colpos;         /* column position (byte) within a item (line) */
    int rowpos;         /* row (which item) number couting from zero   */
    } AuxCurPos;

typedef struct {
    unsigned int state;
    unsigned int cmpshg;
    unsigned int cmplhg;
    } Hg_Status_Buf;    /* for support deletion by eum-so */


/*-----------------------------------------------------------------------*
*	structure definitions
*-----------------------------------------------------------------------*/
/********************************/
/* KIMED internal control block */
/********************************/

typedef struct {
	unsigned char   *echobufs ; /* pointer to echo buff.                */
	unsigned char   *echobufa ; /* pointer to echo att. buff.           */
	unsigned char   *fixbuf ; /* pointer to edit-end buff.            */
/* now, aux can be in multi line format */
	unsigned char   **auxbufs ; /* pointer to aux-area buff. array      */
	unsigned char   **auxbufa ; /* pointer to aux-area att. buff. array */
	unsigned char   **candbuf ; /* pointer to candidate buff. array     */
	unsigned short	*cand_src ; /* pointer to candidate source flag.    */
	unsigned char	*echosvch ; /* character for searching dict.        */
				    /* and restoring echobuf.               */

	int             echosize ;  /* echo buff. size                      */
	int             echoacsz ;  /* active length in echo buff.          */
	int		echoover ;
        int             curadv   ;
	EchoBufChanged  echochfg ;  /* changed flag for echo buff.          */
				    /*  also contains where changed         */
	int             echocrps ;  /* cursor position in echo buff.        */
	int             eccrpsch ;  /* changed flag for cursor pos. (echo)  */
	int		echosvchlen;
	int		echosvchsp;

	List		mrulist;
	DICTINFO	udict;
	DICTINFO	sdict;
	int		learn;
	int		acm;

	int		fixsize ; /* fix buff. size			    */
	int             fixacsz ; /* active length in fix buff.             */
	int		candgetfg;  /* cand get mode.			    */
	int		candsize ;  /* candidate buff. size		    */
	int		candcrpos ; /* cursor position in candidate buf.    */

/* aux size is two dimesional */
	AuxSize         auxsize ;   /* aux-area buff. size                  */
	AuxSize         auxacsz ;   /* active length in aux-area buff.      */
	int             auxchfg ;   /* changed flag for aux-area buff.      */
	int		auxformat;
/* aux cursor position is two dimesional */
	AuxCurPos       auxcrps ;   /* cursor position in aux-area buff.    */
	int             axcrpsch ;  /* changed flag for cursor pos. (aux)   */
	int             auxuse  ;   /* aux-area use flag                    */
				    /* 0 aux buff. is not used		    */
				    /* 1 multiple candidate		    */
				    /* 2 code input			    */

	int             indchfg ;   /* changed flag for indicator buff.     */
	int		needindrw;

	InputMode	imode;	    /* input mode                           */
	int		prevhemode; /* in case mode change to JAMO, save   */
				     /* current Eng/HG for the time JAMO    */
				     /* mode ends			    */

	int             isbeep;     /* is beep requested last time          */

	int		hgstate ;  /* initial cons. interim, final. beep   */
	int		interstate ; /* HG state		   */
				     /* Eng state		   */
				     /* code input state	   */
				     /* JAMO state		   */
				     /* single candidate state	   */
				     /* multiple candidate state   */
	BOOL    missing_char_flag ;     /*** for missing interim char in KS code */
	Hg_Status_Buf hg_status_buf[5];  /* support deletion by eum-so */
	int	hg_status_ps;  /* current position in hg_status_buf */
}    KIMED ;

/*********************/
/* profile structure */
/*********************/
typedef struct {
    char *sys;          /* system dict. file name */
    char *user;         /* user dict. file name   */
    } DictNames;

typedef struct {
    int 	learn;          /* to save learning information or not  */
    int		acm;		/* ACM ON/OFF */
    DictNames 	dictstru; 	/* dictionary name structure */
    } kedprofile;

int kedProcess();
#endif _h_ked_
