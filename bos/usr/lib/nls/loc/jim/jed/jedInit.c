static char sccsid[] = "@(#)72	1.5.1.2  src/bos/usr/lib/nls/loc/jim/jed/jedInit.c, libKJI, bos41J, 9515B_all 4/13/95 21:47:55";
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS:           jedInit(), mkkmpf()
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kmdef.h"
#include "kkcdef.h"
#include "kmpfdef.h"
#include "kmpf.h"
#include "jedexm.h"
#include "jed.h"
#include "jedint.h"

/*
 *	mkkmpf()
 */
KMPF	*mkkmpf( jedprofile *prostr )
{
	KMPF	*kmpf;

        if ( !(kmpf = (KMPF *)malloc( sizeof(KMPF) )) )
                return ( (KMPF *)NULL );

	kmpf->allcand = 0;
	kmpf->insert  = K_INSERT;
	kmpf->kjin    = K_DAACT | K_DAENT | K_DACR;
	kmpf->reset   = K_REACT | K_REENT | K_RECR | K_RERES;

	/* convmode is specified within profile structure */
	if(prostr->convmode == KP_CONV_WORD) {
		kmpf->conversn = K_FUKUGO;
		kmpf->convtype = K_IKKATU;
	} else if(prostr->convmode == KP_CONV_SPHRASE) {
		kmpf->conversn = K_TANBUN;
		kmpf->convtype = K_IKKATU;
	} else if(prostr->convmode == KP_CONV_MPHRASE) {
		kmpf->conversn = K_RENBUN;
		kmpf->convtype = K_IKKATU;
	} else { /* otherwise, look ahead assumed */
		kmpf->conversn = K_RENBUN;
		kmpf->convtype = K_CIKUJI;
	}

	/* numberinput is specified within profile structure */
	if ( prostr->numberinput == KP_JISKUTEN )
		kmpf->kjno = K_KJIS;
	else
		kmpf->kjno = K_KNO;

	kmpf->aux1     = K_AUXNEA;	
	kmpf->aux2     = K_AUXNEA;
	kmpf->aux3     = K_AUXNEA;
	kmpf->aux4     = K_AUXNEA;
	kmpf->aux1maxc = 100;
	kmpf->aux1maxr = 100;
	kmpf->aux2maxc = 0;
	kmpf->aux2maxr = 0;
	kmpf->aux3maxc = 0;
	kmpf->aux3maxr = 0;
	kmpf->aux4maxc = 0;
	kmpf->aux4maxr = 0;

	/* jmnt always assumes BEEPON such that jed can use */
	/* it as an error indicator regardless of jimrc */
	kmpf->beep = K_BEEPON;

	kmpf->regist   = K_DICNG;
	kmpf->mix      = K_MIXNG;
	kmpf->indicat  = 0;
	kmpf->rkc      = K_ROMOFF;
	kmpf->kana     = K_KANA;
	kmpf->kblock   = K_KBLON;
	kmpf->cursout  = K_CURIN;
	kmpf->katakana = K_KANAOF;

	/* alphanum is specified within profile structure */
	if(prostr->alphanum == KP_ALPHAOFF)
		kmpf->alphanum = K_ALPOFF;
        else /* otherwise, ON is assumed */
		kmpf->alphanum = K_ALPON;

        /* learning is specified within profile structure */
	if(prostr->learn == KP_LEARNOFF)
		kmpf->learning = K_LEAOFF;
        else /* otherwise, ON is assumed */
		kmpf->learning = K_LEAON;

	kmpf->kanjinum = K_KJNMON;
	kmpf->pfkreset = K_PFKOFF;
	/* kmpf->convtype is set with kmpf.conversn 	*/

        /* kuten is specified within profile structure */
	if(prostr->kuten == KP_KUTEN78)
		kmpf->kuten = K_JIS78;
        else /* otherwise, ON is assumed */
		kmpf->kuten = K_JIS83;

        /* shmatnum is specified within profile structure */
	kmpf->shmatnum = (char)prostr->shmatnum;

        /* udload is specified within profile structure */
	if(prostr->udload == KP_UDLOADON)
		kmpf->udload = K_UDLOADON;
	else
		kmpf->udload = K_UDLOADOFF;

        /* modereset is specified within profile structure */
	if(prostr->modereset == KP_RESETON)
		kmpf->modereset = K_RESETON;
	else
		kmpf->modereset = K_RESETOFF;

	/* Dictionary names are set by Kanji Monitor correctly.		*/
	kmpf->dnames.sys     = NULL;
	kmpf->dnames.user    = NULL;
	kmpf->dnames.adj     = NULL;

	return ( kmpf );

}

/*
 *	jedInit()
 */
int     jedInit(echobufs, echobufa, echosize, auxbufs, auxbufa,
                auxbufsizep, auxformat, prostr)
unsigned char   *echobufs ;    /* echo buffer address                */
unsigned char   *echobufa ;    /* echo attribute buffer address      */
int             echosize ;     /* echo buffer sizes in byte          */
unsigned char   **auxbufs ;    /* pointer to aux buf addresses       */
unsigned char   **auxbufa ;    /* pointer to aux att buf addresses   */
AuxSize         *auxbufsizep;  /* pointer to max aux buffer size     */
int             auxformat;     /* aux format                         */
jedprofile      *prostr;       /* pointer to profile structure       */
{
	/*******************/
	/* local variables */
	/*******************/
	extern void     set_indicator();
	EXT             *ext;
	KMPF            *prof;
	KCB             *kcb;
	FEPCB           *fepcb;
	static  DICTS   dictnames; /* dict name str for exmonitor */
	int		i;

       /*
	*  checks if the arguments are acceptable.
	*/
	/********************/
        /* echo buffer size */
	/********************/
	if(echosize < MINECHO)
	    return(KP_ERR);

	/*******************/
        /* aux buffer size */
	/*******************/
	if(auxformat == KP_LONGAUX) {
	    if((auxbufsizep->itemsize < AUXITEMLEN) ||
	       (auxbufsizep->itemnum < AUXITEMNUM))
		    return(KP_ERR);
        }
	else { /* aux of short format */
	    if((auxbufsizep->itemsize < MINAUX) ||
	       (auxbufsizep->itemnum < 1))
		    return(KP_ERR);
	}

       /*
	*  makes profile temporary and prepares for exkjopen().
	*/
	if ( !(prof = mkkmpf( (jedprofile *)prostr) ) )
		return KP_ERR;
	if ( !(ext = (EXT*)malloc(sizeof(EXT))) )
	{
		free(prof);
		return KP_ERR;
	}

	/****************************************************/
	/* construct extended information for Kanji Monitor */
	/****************************************************/
	ext->prokmpf = prof;

	ext->maxstc = (short)echosize;
	ext->maxstr = 1;
	ext->maxa1c = auxbufsizep->itemsize;
	ext->maxa1r = auxbufsizep->itemnum;
	for( i = 0; prostr->dictstru.sys[i] != NULL; i++ )
 	    dictnames.sys[i] = prostr->dictstru.sys[i];
	dictnames.user = prostr->dictstru.user;
	dictnames.adj = prostr->dictstru.adj;
	ext->dicts = &dictnames;

#if DBG_MSG
for( i = 0; ext->dicts->sys[i] != NULL; i++ )
  fprintf(stderr, "jed:jedInit():ext->sys name[%d] = [%s] being passed to exkj\n",i,ext->dicts->sys[i]);

fprintf(stderr, "jed:jedInit():ext->user name = %s being passed to exkj\n", ext->dicts->user);
fprintf(stderr, "jed:jedInit():ext->adj  name = %s being passed to exkj\n", ext->dicts->adj);
#endif

#ifdef DICTDEBUG
fprintf(stderr, "sys name = %s being passed to exkj\n",
			ext->dicts->sys);
fprintf(stderr, "user name = %s being passed to exkj\n",
			ext->dicts->user);
fprintf(stderr, "adj name = %s being passed to exkj\n",
			ext->dicts->adj);
#endif /* DICTDEBUG */

	/**********************/
	/* open Kanji Monitor */
	/**********************/
	if (exkjopen( &kcb, CSIDJ, ext ))
	{
		free(prof);
		free(ext);
		return KP_ERR;
	}
	free(prof);
	free(ext);

       /*
	*  prepares for exkjinit()
	*/

	if ( !(kcb->string = calloc((unsigned)echosize, sizeof(char))) )
	{
		(void)exkjclos(kcb);
		return KP_ERR;
	}
	kcb->actcol = (short)echosize;
	kcb->actrow = 1;
	kcb->flatsd = MIXEDMODE;
	kcb->repins = INSERT;
	kcb->conv   = INIT_CONV;

	if ( exkjinit(kcb) )
	{
		free( kcb->string );
		(void)exkjclos(kcb);
		return KP_ERR;
	}

       /*
	*  allocates the FEPCB structure
	*/

	if ( !(fepcb = (FEPCB*)malloc(sizeof(FEPCB))) )
	{
		(void)exkjterm(kcb);
		free( kcb->string );
		(void)exkjclos(kcb);
		return KP_ERR;
	}

       /*
	*  initializes values in FEPCB
	*/
	fepcb->kcb      = kcb;
	fepcb->echobufs = echobufs;
	fepcb->echobufa = echobufa;
	fepcb->auxbufs  = auxbufs;
	fepcb->auxbufa  = auxbufa;
	fepcb->echosize = echosize;
	fepcb->echoover = echosize;
	fepcb->auxsize.itemsize = auxbufsizep->itemsize;
	fepcb->auxsize.itemnum = auxbufsizep->itemnum;
	fepcb->echochfg.flag = OFF;
	fepcb->echochfg.chtoppos = 0;
	fepcb->echochfg.chlenbytes = 0;
	fepcb->echocrps = 0;
	fepcb->eccrpsch = OFF;
	fepcb->echoacsz = 0;
	fepcb->auxchfg  = OFF;
	fepcb->auxuse   = NOTUSE;
	fepcb->auxcrps.colpos = -1;
	fepcb->auxcrps.rowpos = -1;
	fepcb->axcrpsch = OFF;
	fepcb->auxacsz.itemsize = 0;
	fepcb->auxacsz.itemnum = 0;
	if(prostr->convmode == KP_CONV_WORD)
	    fepcb->convmode = WORD;
	else if(prostr->convmode == KP_CONV_SPHRASE)
	    fepcb->convmode = SPHRASE;
	else if(prostr->convmode == KP_CONV_MPHRASE)
	    fepcb->convmode = MPHRASE;
        else
	    fepcb->convmode = LOOKAHEAD;
 	fepcb->axconvsw = OFF;
	fepcb->indchfg  = OFF;
	fepcb->shift[0] = kcb->shift1;
	fepcb->shift[1] = kcb->shift2;
	fepcb->shift[2] = kcb->shift3;
	fepcb->alpha = TRUE;
	fepcb->isbeep = OFF;
	if(((jedprofile *)prostr)->kjbeep == KP_BEEPON)
	    fepcb->beepallowed = ON;
        else
	    fepcb->beepallowed = OFF;
	fepcb->dbcsormix = KP_MIX;
	fepcb->auxformat = auxformat;
	fepcb->selection = FALSE;
	fepcb->impanel.item = NULL;
	fepcb->impanel.item_row = 0;
	fepcb->impanel.item_col = 0;

       /*
	*  sets indicator so that it matches present input mode
	*/

	(void)set_indicator(fepcb);
	fepcb->imode.ind3 = KP_NORMALMODE;         /* normal mode */
	fepcb->imode.ind4 = KP_INSERTMODE;         /* always insert mode */

	return ((int)fepcb);
}
