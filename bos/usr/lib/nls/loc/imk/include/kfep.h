/* @(#)45        1.3  src/bos/usr/lib/nls/loc/imk/include/kfep.h, libkr, bos411, 9428A410j 3/23/93 20:31:35 */
/*
 * COMPONENT_NAME :	(KRIM) - AIX Input Method
 *
 * FUNCTIONS :		kfep.h
 *
 * ORIGINS :		27
 *
 * (C) COPYRIGHT International Business Machines Corp.  1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _h_kfep_
#define _h_kfep_

/*-----------------------------------------------------------------------*
*	Include
*-----------------------------------------------------------------------*/
#include <sys/types.h>
#include "im.h"			/* input method header file       */
#include "imerrno.h"		/* input method error header file */
#include "kimerrno.h"           /* Korean input method error    */

/*-----------------------------------------------------------------------*
*	generic definitions
*-----------------------------------------------------------------------*/
#ifndef	TRUE
#define TRUE	1
#endif	TRUE
#ifndef	FALSE
#define FALSE	0
#endif	FALSE
#ifndef	NULL
#define NULL	0
#endif	NULL


/*-----------------------------------------------------------------------*
*	KIM IMFEP structure
*-----------------------------------------------------------------------*/
typedef struct _KIMfep {
	IMFepCommon  common;		/* IMFEP common */
	int          codeset;		/* Supports only KIM_DB_CS */
	IMLanguage   language;		/* language	*/
	int    	     kimver;		/* KIM version  */
        IMKeymap     *immap;            /* KIM keymap   */
	} KIMfep, *KIMFEP;

#define KIM_VERSION ((1 << 16) | 1 )    /* current KIM version */

/*-----------------------------------------------------------------------*
*	KIM IMOBJ structure
*-----------------------------------------------------------------------*/
typedef struct _KIMobject {
	IMObjectCommon  imobject;	/* IM common info            */
	IMTextInfo      textinfo;       /* KIM text information      */
	IMAuxInfo       auxinfo;	/* KIM auxiliary information */
	IMSTR           string;         /* KIM string for GetString  */
	IMSTR           indstr;         /* KIM indicator string      */
	IMIndicatorInfo indinfo;        /* KIM indicator information */
	IMQueryState    q_state;        /* KIM state                 */       
	caddr_t         outstr;         /* string output for Process */
	caddr_t         auxid;          /* aux id created by callback*/
	int		auxidflag;      /* TRUE is aux is created    */
        int   	        codeset;        /* code set                  */
	int             auxstate;    	/* auxiliary area state      */
        int             textauxstate;   /* return value of IMProcess */
        IMBuffer        output;         /* output buffer             */
	int		kimed;          /* KIM ED information        */
	} KIMobj, *KIMOBJ;

/*-----------------------------------------------------------------------*
*	constant definitions for KIMFEP internal use
*-----------------------------------------------------------------------*/

#define KIM_NOTEXT    0			/* no text so far */
#define KIM_TEXTON    (KIM_NOTEXT + 1)	/* there has been Text already */

#define KIM_AUXEMTY   	0x00
#define KIM_AUXNOW     	0x01      /* there is aux after the last KIM */
#define KIM_AUXBEFORE  	0x02      /* there was aux before tha last KIM*/


#define KIM_ONEAUXCOL  80 /* TBD */
#define KIM_AUXCOLMAX  21 /* TBD */
#define KIM_AUXROWMAX  10 /* TBD */

#define KIM_BEEPPER ON 
#define KIM_NOBEEP  OFF 

/*-----------------------------------------------------------------------*
*	macro definitions for KIMFEP internal use
*-----------------------------------------------------------------------*/
#define SAVEAUXSTATE(x) ((x) <<= 1)

/*-----------------------------------------------------------------------*
*	KIM profile structure definition
*-----------------------------------------------------------------------*/
typedef struct {
    char *sys;           /* system dictionary file name  */
    char *usr;           /* user dictionary file name    */
    } KIMDictNames;

typedef struct {
    int  initbase;
    int  initsize;       	/* initial state of baja or jeonja */
    int  inithanja;        	/* initial state for HHC   */
    int	 initbeep;
    int  initinsrep;
    int  initlearning;       	/* save learning information or note  */
    KIMDictNames dictname; 	/* dictionary names structure */
    int   initacm;		/* initial sate of Auto Commit Mode */
    } kimprofile;

/*-----------------------------------------------------------------------*
* KIM unique indicator states
*-----------------------------------------------------------------------*/
#define KIM_SH_NULL        0
#define KIM_SH_MASK     0x07
#define KIM_SH_ALPHA    0x00
#define KIM_SH_HANGUL   0x01
#define KIM_SH_JAMO     0x02
#define KIM_SH_HANJAMASK  0x08
#define KIM_SH_HANJAOFF   0x00
#define KIM_SH_HANJAON    0x08

/*-----------------------------------------------------------------------*
*	KIM profile/profile structure constant definition
*-----------------------------------------------------------------------*/
/*****************/
/* initbase      */
/*****************/
#define KIM_INITBASE 		"initbase"
#define KIM_I_ALPHANUM 		"alphanum"
#define	KIM_I_HANGEUL		"hangeul"
#define KIM_I_JAMO		"jamo"
#define KIM_ALPHANUM	 	1
#define KIM_HANGEUL		(KIM_ALPHANUM+1)
#define	KIM_JAMO		(KIM_HANGEUL+1)

/*****************/
/* initsize */
/*****************/
#define KIM_INITSIZE 		"initsize"
#define KIM_I_BANJA		"banja"
#define KIM_I_JEONJA 		"jeonja"
#define KIM_BANJA 		1
#define KIM_JEONJA 		(KIM_BANJA + 1)

/****************/
/* inithanja 	*/
/****************/
#define KIM_INITHANJA 		"inithanja"
#define KIM_I_HJON 		"on"
#define KIM_I_HJOFF 		"off"
#define KIM_HANJAON  		1
#define KIM_HANJAOFF 		(KIM_HANJAON + 1)

/****************/
/* initbeep 	*/
/****************/
#define KIM_INITBEEP 		"initbeep"
#define KIM_I_BPON		"on"
#define KIM_I_BPOFF		"off"
#define KIM_BEEPON  		1
#define KIM_BEEPOFF 		(KIM_BEEPON + 1)

/****************/
/* learn 	*/
/****************/
#define KIM_INITLEARN 		"initlearning"
#define KIM_I_LEARNON		"on"
#define KIM_I_LEARNOFF		"off"
#define KIM_LEARNON		1
#define KIM_LEARNOFF 		(KIM_LEARNON + 1)

/************************/
/* insert or replace 	*/
/************************/
#define	KIM_INITINSREP		"initinsrep"
#define KIM_I_INSERT		"insert"
#define	KIM_I_REPLACE		"replace"
#define	KIM_INSERT		1
#define	KIM_REPLACE		(KIM_INSERT+1)

/****************/
/* ACM 		*/
/****************/
#define	KIM_INITACM		"initacm"
#define KIM_I_ACMON		"on"
#define	KIM_I_ACMOFF		"off"
#define	KIM_ACMON		1
#define	KIM_ACMOFF		(KIM_ACMON + 1)

/****************/
/* system dict  */
/****************/
#define	KIM_SYS_DICTIONARY	"dict.ks"

/*-----------------------------------------------------------------------*
*	KIM indicator string code point definitions
*-----------------------------------------------------------------------*/
/* max length among defined below */
#define KIM_INDSTR_MAXLEN    	100 
#define KIM_INDSTR_EMTY		0
/* long format  */

			/* English 0xbfb5b9aea1a1 */
#define KIM_SJIND_ALPHA "\277\265\271\256\241\241" 
			/* HANGUL   0xc7d1b1dba1a1*/
#define KIM_SJIND_HANGUL  "\307\321\261\333\241\241" 
			/* JAMO   0xc0dab8f0a1a1*/
#define KIM_SJIND_JAMO  "\300\332\270\360\241\241" 
			/* JEONJA  0xc0fcc0daa1a1*/
#define KIM_SJIND_JEON   "\300\374\300\332\241\241" 
			/* BANJA  0xb9ddc0daa1a1*/
#define KIM_SJIND_BAN   "\271\335\300\332\241\241" 
			/* HANJA ON 0xc7d1c0daa1a1*/
#define KIM_SJIND_RD    "\307\321\300\332\241\241"                    
			/* HANJA OFF 0xa1a1 */
#define KIM_SJIND_DSP   "\241\241\241\241\241\241"                    
			/* R  0x52 single byte    */
#define KIM_SJIND_RS    "\122"                         
			/*    0x20 single byte    */
#define KIM_SJIND_SSP    "\040"                         

#define KIM_SJIND_REP   "\264\353\303\274\241\241" 

#define KIM_SJIND_INS   "\273\360\300\324\241\241" 
#define KIM_SJIND_SPACE "\241\241\241\241\241\241"                    

/* short format  for both MIX or DBCSONLY */

			/* English 0xbfb5b9ae*/
#define KIM_SJSIND_ALPHA "\277\265\271\256" 
			/* HANGUL   0xc7d1b1db
#define KIM_SJSIND_HANGUL  "\307\321\261\333" 
			/* JAMO   0xc0dab8f0*/
#define KIM_SJSIND_JAMO  "\300\332\270\360" 
			/* JEONJA  0xc0fcc0da*/
#define KIM_SJSIND_JEON   "\300\374\300\332" 
			/* BANJA  0xb9ddc0da*/
#define KIM_SJSIND_BAN   "\271\335\300\332" 
			/* HANJA ON 0xc7d1c0da*/
#define KIM_SJSIND_RD    "\307\321\300\332"                    
			/* HANJA OFF 0xa1a1 */
#define KIM_SJSIND_DSP   "\241\241"                    
			/* R  0x52 single byte    */
#define KIM_SJSIND_RS    "\122"                         
			/*    0x20 single byte    */
#define KIM_SJSIND_SSP   "\040"                         


/* indicator string lengths for defined above */
#define KIM_SJD_LONGINDLEN  (sizeof(KIM_SJIND_ALPHA) - 1 \
			     + sizeof(KIM_SJIND_JEON) - 1 \
			     + sizeof(KIM_SJIND_INS) - 1 \
			     + sizeof(KIM_SJIND_RD) - 1 )
#define KIM_SJM_LONGINDLEN  (sizeof(KIM_SJIND_ALPHA) - 1 \
			     + sizeof(KIM_SJIND_JEON) - 1 \
			     + sizeof(KIM_SJIND_RS) - 1 )
#define KIM_SJD_SHORTINDLEN (sizeof(KIM_SJSIND_ALPHA) - 1 \
			    + sizeof(KIM_SJSIND_JEON) - 1 \
			    + sizeof(KIM_SJSIND_INS) - 1 \
			    + sizeof(KIM_SJSIND_RD) - 1)
#define KIM_SJM_SHORTINDLEN (sizeof(KIM_SJSIND_ALPHA) - 1 \
			    + sizeof(KIM_SJSIND_JEON) - 1 \
			    + sizeof(KIM_SJSIND_RS) - 1)


/***********************************/
/* Auxiliary Window title message. */
/***********************************/
#define	TI_ALLCAND1	"\307\321\300\332\241\241\272\257\310\257"	
#define	TI_ALLCAND2     "\271\370\310\243\241\241\310\304\272\270" 		
#define TI_CODEINP 	"\304\332\265\345 \300\324\267\302 \242\241         "		
#define TI_REMAINSZ     "\263\262\300\272\241\241\310\304\272\270\274\366\241\241"
 	
/****************************************/
/* Auxbuf format and Aux Buffer info. */
/****************************************/
#define KIM_AUX_LONGFORMAT	0
#define	KIM_AUX_SHORTFORMAT	1
/*************
#define KIM_AUX_ROWMAX		10
#define KIM_AUX_COLMAX		21 
***************/
#define KIM_AUXTITL_EMTY	0

/****************************************/
/* String info			        */
/****************************************/
#define	KIM_STRBUF_EMTY		0

/****************************************/
/* Text information.			*/
/****************************************/
#define	KIM_TXT_EMTY		0
#define KIM_TXT_MINWIDTH	0
#define KIM_TXT_NOCHANGED_LEN	0
#define	KIM_TXT_CURFIRST	0

/****************************************/
/* Aux Information. */
/****************************************/
#define KIM_AUXID_NOTSETTED	FALSE

/***********************************/
/* Panel info 		 	   */
/***********************************/
#define KIM_NOPANEL		'\0'
#define	KIM_NOPANEL_ROW		0
#define KIM_NOPANEL_COL		0

/***********************************/
/* Message Text size.		   */
/***********************************/
#define	KIM_MSGTXT_MAXROW	13
#define KIM_MSGTXT_MAXCOL	50
#define KIM_MSGTXT_MINWIDTH	0
#define KIM_MSGTXT_EMTY		0
#define KIM_MSG_CURFIRST	-1
#define	KIM_MSG_CUROFF		0
#define	KIM_MSG_CURON		1
#define KIM_MSG_ROWPOS_FIRST	-1
#define KIM_MSG_COLPOS_FIRST	-1

/***********************************/
/* Aux window title info.	   */
/***********************************/
#define KIM_CODEINP_WINWIDTH	21
#define KIM_CODEINP_WINMSGNUM	0
#define KIM_TITL1_ALLCAND	3
#define KIM_TITL2_ALLCAND	12
#define KIM_WINWIDTH_ALLCAND	(KIM_TITL1_ALLCAND+KIM_TITL2_ALLCAND) 	
#define KIM_WINMSGNUM_ALLCAND	3

#define TI_ALLCAND1_LEN	(sizeof(TI_ALLCAND1) -1 )
#define TI_ALLCAND2_LEN	(sizeof(TI_ALLCAND2) -1 )
#define TI_CODEINP_LEN	(sizeof(TI_CODEINP) -1 )
#define TI_REMAINSZ_LEN (sizeof(TI_REMAINSZ) -1 ) 	

/*****************/
/* EUC encoding */
/*****************/
/* long format  */
/* TBD TBD  */

/* short format  */
/* TBD TBD  */

/* indicator string lengths for defined above */
/* TBD TBD  */

/*-----------------------------------------------------------
 *     Mask of valid state bits
 *-----------------------------------------------------------*/
#define KIM_VALIDBITS \
        (ShiftMask|LockMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask)

#endif _h_kfep_
