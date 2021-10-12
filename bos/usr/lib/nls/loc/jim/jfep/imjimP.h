/* @(#)15	1.7.1.2  src/bos/usr/lib/nls/loc/jim/jfep/imjimP.h, libKJI, bos411, 9428A410j 5/18/93 05:35:42	*/
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : internal header file
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

#ifndef _h_imjimP_
#define _h_imjimP_

/*-----------------------------------------------------------------------*
*	Include
*-----------------------------------------------------------------------*/
#include <sys/types.h>
#include <iconv.h>
#include "im.h"			/* input method header file       */
#include "imP.h"
#include "imerrno.h"		/* input method error header file */
#include "jimerrno.h"           /* Japanese input method error    */
#include "imjim.h"		/* JIM public header file	*/
#include "jed.h"
#include "dict.h"
#include "JIMRegister.h"	/* JIM runtime registration header	*/

/*-----------------------------------------------------------------------*
*	generic definitions
*-----------------------------------------------------------------------*/
#ifndef	TRUE
#define TRUE	1
#endif	/* TRUE */
#ifndef	FALSE
#define FALSE	0
#endif	/* FALSE */
#ifndef	NULL
#define NULL	0
#endif	/* NULL */

/*-----------------------------------------------------------------------*
*	structure holding information between JIMED
*-----------------------------------------------------------------------*/
typedef struct {
	int	jeid;			/* JIM ED identification */
	char	*echobufs;		/* echo buffer address        */
	char	*echobufa;		/* echo attribute address     */
	int	echosize;		/* size of above buffers      */
	char	**auxbufs;		/* ptr to aux buf addresses   */
	char	**auxbufa;		/* ptr to aux atr addresses   */
	AuxSize	auxsize;		/* aux buffer sizes           */
}	JIMed, *JIMED;

/*-----------------------------------------------------------------------*
*	JIM IMFEP structure
*-----------------------------------------------------------------------*/
typedef struct	_JIMfep	{
	IMFepCommon	common;		/* IMFEP common */
	int	codeset;		/* UJIS/SJIS/SJISDBCSONLY */
	iconv_t	cd;			/* converter from SJIS to UJIS */
	int	jimver;			/* JIM version  */
	IMKeymap	*immap;		/* JIM keymap   */
	Sdictdata	sdictdata;	/* system dictionaty information */
	Udictinfo	udictinfo;	/* user dictionaty information */
	Fdictinfo	fdictinfo;	/* FUZOKUGO dictionaty information */
}	JIMfep, *JIMFEP;

#define JIM_VERSION ((1 << 16) | 1 )    /* current JIM version */

/*-----------------------------------------------------------------------*
*	JIM IMOBJ structure
*-----------------------------------------------------------------------*/
typedef struct _JIMobject {
	IMObjectCommon  imobject;	/* IM common info            */
	IMTextInfo      textinfo;       /* JIM text information      */
	IMAuxInfo       auxinfo;	/* JIM auxiliary information */
	IMSTR           string;         /* JIM string for GetString  */
	IMSTR           indstr;         /* JIM indicator string      */
	IMIndicatorInfo indinfo;        /* JIM indicator information */
	IMQueryState    q_state;        /* JIM state                 */       
	caddr_t         outstr;         /* string output for Process */
	caddr_t         auxid;          /* aux id created by callback*/
	int		auxidflag;      /* TRUE is aux is created    */
        int   	        codeset;        /* code set                  */
	int             auxstate;    	/* auxiliary area state      */
	int		textauxstate;	/* return value of IMProcess */
	IMBuffer	output;		/* output buffer             */
	JIMed           jedinfo;        /* JIM ED information        */
	int		modereset;	/* mode reset flag           */
	int		supportselection;/* support selection flag   */
	JIMRegister	*registration;	/* runtime registration      */
}	JIMobj, *JIMOBJ;

/*-----------------------------------------------------------------------*
*	constant definitions for JIMFEP internal use
*-----------------------------------------------------------------------*/
#define JIM_SJISDBCS 0                  /* shift JIS without single byte*/
#define JIM_SJISMIX  (JIM_SJISDBCS + 1) /* shift JIS */
#define JIM_EUC      (JIM_SJISMIX + 1)  /* EUC       */

/* used by JIMInit.c to determine the code set */
#define	JIM_CS_IBM932		"IBM-932"
#define	JIM_CS_MODIFIER		"@im=DoubleByte"
#define	JIM_CS_IBMeucJP		"IBM-eucJP"
#define	JIM_CS_IBM301		"IBM-301"
#define	JIM_CS_UJIS		"ujis"
#define	JIM_CS_SJIS		"sjis"
#define	JIM_CS_932_BY_LANG	"Ja"
#define	JIM_CS_eucJP_BY_LANG	"ja"
#define	JIM_CS_OLD_LANG		"Jp"

#define JIM_NOTEXT    0			/* no text so far */
#define JIM_TEXTON    (JIM_NOTEXT + 1)	/* there has been Text already */

#define JIM_AUXNOW     0x01             /* there is aux after the last JIM */
#define JIM_AUXBEFORE  0x02             /* there was aux before tha last JIM*/

#define JIM_ONEAUXCOL  80 /* TBD */
#define JIM_AUXCOLMAX  50 /* TBD */
#define JIM_AUXROWMAX  16 /* TBD */

#define JIM_BEEPPER 50 /* TBD */
#define JIM_NOBEEP -100 /* TBD */

/*-----------------------------------------------------------------------*
*	macro definitions for JIMFEP internal use
*-----------------------------------------------------------------------*/
#define SAVEAUXSTATE(x) ((x) <<= 1)

/*-----------------------------------------------------------------------*
*	JIM profile structure definition
*-----------------------------------------------------------------------*/
typedef struct {
    char **sys;                 /* system dictionary file name          */
    char *usr;           	/* user dictionary file name    	*/
    char *adj;           	/* adjunct dictionary file name 	*/
} JIMDictNames;

typedef struct {
    int  initchar;       /* initial state of character shift  */
    int  initsize;       /* initial state of single or double */
    int  initrkc;        /* initial state for RKC   */
    int  conversion;     /* initial conversion mode */
    int  kjbeep;         /* editor requests beep or not        */
    int  learning;       /* save learning information or note  */
    int  alphanum;       /* to allow/not to allow for alpha numeric conv. */
    JIMDictNames dictname; /* dictionary names structure */
    int  kuten;   	/* Kuten Code Input 		 */
    int  shmatnum;    	/* Max JIM can use the number of shmat          */
    int  udload;        /* load User dictionary or not                  */
    int  numberinput;   /* initial state for JISkuten/IBMKanji number   */
    int  modereset;     /* reset input mode with ESC key                */
    int  supportselection;/* support selection ON/OFF                   */
} jimprofile;

/*-----------------------------------------------------------------------*
*	JIM profile/profile structure constant definition
*-----------------------------------------------------------------------*/
/* initchar */
#define JIM_INITCHAR "initchar"
#define JIM_INITALPHA "alphanum"
#define JIM_INITKATA "katakana"
#define JIM_INITHIRA "hiragana"
#define JIM_ALPHA 1
#define JIM_HIRA  (JIM_ALPHA + 1)
#define JIM_KATA  (JIM_HIRA + 1)
/* initsize */
#define JIM_INITSIZE "initsize"
#define JIM_INITSGL "single"
#define JIM_INITDBL "double"
#define JIM_SINGLE 1
#define JIM_DOUBLE (JIM_SINGLE + 1)
/* initrkc */
#define JIM_INITRKC "initrkc"
#define JIM_INITON "on"
#define JIM_INITOFF "off"
#define JIM_RKCON  1
#define JIM_RKCOFF (JIM_RKCON + 1)
/* conversion */
#define JIM_PROCONV "conversion"
#define JIM_CONVWORD "word"
#define JIM_CONVSPHRASE "sphrase"
#define JIM_CONVMPHRASE "mphrase" 
#define JIM_CONVLOOKAHEAD "look-ahead"
#define JIM_WORD      1
#define JIM_SPHRASE   (JIM_WORD + 1)
#define JIM_MPHRASE   (JIM_SPHRASE + 1)
#define JIM_LOOKAHEAD (JIM_MPHRASE + 1)
/* kjbeep */
#define JIM_PROBEEP "kjbeep"
#define JIM_BEEPON  1
#define JIM_BEEPOFF (JIM_BEEPON + 1)
/* learn */
#define JIM_PROLEARN "learning"
#define JIM_LEARNON  1
#define JIM_LEARNOFF (JIM_LEARNON + 1)
/* apha num */
#define JIM_PROALPHA "alphanum"
#define JIM_ALPHAON  1
#define JIM_ALPHAOFF (JIM_ALPHAON + 1)
/* Kuten */
#define JIM_PROKUTEN "kuten"
#define JIM_JIS78    "JIS78"
#define JIM_JIS83    "JIS83"
#define JIM_KUTEN78  1
#define JIM_KUTEN83  (JIM_KUTEN78 + 1)
/* shmatnum */
#define JIM_PROSHNUM "shmatnum"
#define JIM_SHNUMDEF  1
/* udload */
#define JIM_PROUDLOAD "udload"
#define JIM_UDLOADON  1
#define JIM_UDLOADOFF (JIM_UDLOADON + 1)
/* numberinput */
#define JIM_PRONUMBER "numberinput"
#define JIM_KUTENINPUT  "JISkuten"
#define JIM_KANJIINPUT  "IBMkanji"
#define JIM_JISKUTEN  1
#define JIM_IBMKANJI  (JIM_JISKUTEN + 1)
/* modereset */
#define JIM_PRORESET "modereset"
#define JIM_RESETON  1
#define JIM_RESETOFF (JIM_RESETON + 1)
/* supportselection */
#define JIM_SUPPORTSELECTION "supportselection"
#define JIM_SELECTIONON  1
#define JIM_SELECTIONOFF (JIM_SELECTIONON + 1)


/*-----------------------------------------------------------------------*
*	JIM indicator string code point definitions
*-----------------------------------------------------------------------*/
/* max length among defined below */
#define JIM_INDSTRMAXLEN    100 /* TBD */
/*****************/
/* SJIS encoding */
/*****************/
/* long format  for both MIX or DBCSONLY */
			/* EI SUU 0x897090948140 */
#define JIM_SJIND_ALPHA "\211\160\220\224\201\100" 
			/* HIRA   0x82a982c88140*/
#define JIM_SJIND_HIRA  "\202\251\202\310\201\100" 
			/* KANA   0x834a83698140*/
#define JIM_SJIND_KATA  "\203\112\203\151\201\100" 
			/* zenkaku  0x91538a708140*/
#define JIM_SJIND_ZEN   "\221\123\212\160\201\100" 
			/* hankaku  0x94bc8a708140*/
#define JIM_SJIND_HAN   "\224\274\212\160\201\100" 
			/* R  0x8271  double byte */
#define JIM_SJIND_RD    "\202\161"                    
			/*    0x8140 double byte  */
#define JIM_SJIND_DSP   "\201\100"                    
			/* R  0x52 single byte    */
#define JIM_SJIND_RS    "\122"                         
			/*    0x20 single byte    */
#define JIM_SJIND_SSP    "\040"                         

/* short format  for both MIX or DBCSONLY */
			/* EI SUU 0x89709094*/
#define JIM_SJSIND_ALPHA "\211\160\220\224" 
			/* HIRA   0x82a982c8*/
#define JIM_SJSIND_HIRA  "\202\251\202\310" 
			/* KANA   0x834a8369*/
#define JIM_SJSIND_KATA  "\203\112\203\151" 
			/* zenkaku  0x9153*/
#define JIM_SJSIND_ZEN   "\221\123" 
			/* hankaku  0x94bc*/
#define JIM_SJSIND_HAN   "\224\274" 
			/* R  0x8271  double byte */
#define JIM_SJSIND_RD    "\202\161"                    
			/*    0x8140 double byte  */
#define JIM_SJSIND_DSP   "\201\100"                    
			/* R  0x52 single byte    */
#define JIM_SJSIND_RS    "\122"                         
			/*    0x20 single byte    */
#define JIM_SJSIND_SSP    "\040"                         

/* indicator string lengths for defined above */
#define JIM_SJD_LONGINDLEN  (sizeof(JIM_SJIND_ALPHA) - 1 \
			     + sizeof(JIM_SJIND_ZEN) - 1 \
			     + sizeof(JIM_SJIND_RD) - 1 )
#define JIM_SJM_LONGINDLEN  (sizeof(JIM_SJIND_ALPHA) - 1 \
			     + sizeof(JIM_SJIND_ZEN) - 1 \
			     + sizeof(JIM_SJIND_RS) - 1 )
#define JIM_SJD_SHORTINDLEN (sizeof(JIM_SJSIND_ALPHA) - 1 \
			    + sizeof(JIM_SJSIND_ZEN) - 1 \
			    + sizeof(JIM_SJSIND_RD) - 1)
#define JIM_SJM_SHORTINDLEN (sizeof(JIM_SJSIND_ALPHA) - 1 \
			    + sizeof(JIM_SJSIND_ZEN) - 1 \
			    + sizeof(JIM_SJSIND_RS) - 1)

/*
 *	Mask of valid state bits
 */
#define	JIM_VALIDBITS	(ShiftMask|LockMask|ControlMask|Mod1Mask|Mod2Mask)

#endif /* _h_imjimP_ */
