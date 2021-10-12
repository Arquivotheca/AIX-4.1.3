/* @(#)76	1.3  src/bos/usr/lib/nls/loc/jim/jed/jedSetStr.c, libKJI, bos411, 9428A410j 6/6/91 11:02:02 */
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include "jedexm.h"
#include "jed.h"
#include "jedint.h"

/*-----------------------------------------------------------------------*
*	Beginning of procedure
*-----------------------------------------------------------------------*/
int     jedSetString( fepcb, inpstr, inplen )
FEPCB           *fepcb ;
unsigned char   *inpstr ;
int             inplen ;
{
    /*******************/
    /* local variables */
    /*******************/
    extern int      get_state() ;
    extern void     set_imode() ;
    char            *dhtorkc() ;
    KCB         *kcb = fepcb->kcb ;
    int         state ;
    int         i, j ;
    int         c ;
    int         nbytes ;
    char        *str ;
    char        save_shift[3] ;
    unsigned char       attchar ;

   /*
    *  inspects present process state
    */
    if(fepcb->imode.ind3 == KP_SUPPRESSEDMODE)
	return(KP_ERR);

    state = get_state( fepcb ) ;
    if( !( state == INACTIVE || state == ACTIVATING ) )
	return KP_ERR  ;

    if( state == ACTIVATING )
	(void)rkcbufclear( kcb ) ;

   /*
    *   inspects whether length is valid
    */
    if( inplen > fepcb->echoover || inplen % 2 != 0 )
	return KP_ERR  ;

   /*
    *   saves shift state
    */
    for ( i = 0 ; i < 3 ; i++ )
	save_shift[i] = fepcb->shift[i] ;

   /*
    *   set shift state as HIRAGANA, DOUBLE, RKC_ON
    */
    fepcb->shift[0] = HIRAGANA ;
    fepcb->shift[1] = RKC_ON ;
    fepcb->shift[2] = DOUBLE ;
    (void)set_imode(fepcb) ;

   /*
    *   processes received Hiragana string
    *   this accepst double byte hiragana as input
    *   however, since Kanji Monitor can accept only JISCII 8 bit
    *   character code, force Monitor to RKC mode first, 
    *   and input those hiragana with RKC 
    *   on completing shift state should be reset to 
    *   previous one    
    */
    for ( i = 0 ; i < inplen ; i+=2 )
    {
	c = ( inpstr[i] << 8 ) | inpstr[i+1] ;
	if( c < HIRAGANA_A || HIRAGANA_N < c )
	    return KP_ERR ;

	str = dhtorkc( c, &nbytes ) ;

	for ( j = 0 ; j < nbytes ; j++ )
	{
	    kcb->code = *(str+j) ;
	    kcb->type = TYPE1 ;
	    (void)exkjinpr( kcb ) ;
	}
    }

   /*
    *   processes conversion key
    */
    kcb->code = PCONVERT ;
    kcb->type = TYPE2 ;
    (void)exkjinpr( kcb ) ;

   /*
    *   updates echo buffer if it has changed
    */
    if ( kcb->chlen > 0 || kcb->lastch)
    {
	(void)memcpy(fepcb->echobufs, kcb->string, kcb->lastch) ;

	for ( i = 0 ; i < kcb->lastch ; i++ )
	{
	    attchar = REVERSE & kcb->hlatst[i] ;
	    if( attchar == REVERSE )
		attchar = KP_HL_REVERSE ;
	    fepcb->echobufa[i] = attchar | KP_HL_UNDER ;
	}

	fepcb->echochfg.flag = ON ;
	fepcb->echoacsz = kcb->lastch ;
	if ( kcb->curcol != fepcb->echocrps )
	{
		fepcb->echocrps = kcb->curcol ;
		fepcb->eccrpsch = ON ;
	}
    }

   /*
    *   restores shift state
    */
    for ( i = 0 ; i < 3 ; i++ )
	fepcb->shift[i] = save_shift[i];
    (void)set_imode(fepcb) ;

    return KP_OK ;
} /* end of jedSetString */

/*-----------------------------------------------------------------------*
*  dhtorkc()
*      This routine converts double-byte Hiragana to single-byte kana
*      which correspond to alphabets in RKC_ON mode.
*-----------------------------------------------------------------------*/
static  char    *dhtorkc( dhira, nbytes )
int         dhira ;
int         *nbytes ;
{
    static  char    htorkc_xa[]     = { 0xbb, 0xc1, 0x00 };
    static  char    htorkc_a[]      = { 0xc1, 0x00 };
    static  char    htorkc_xi[]     = { 0xbb, 0xc6, 0x00 };
    static  char    htorkc_i[]      = { 0xc6, 0x00 };
    static  char    htorkc_xu[]     = { 0xbb, 0xc5, 0x00 };
    static  char    htorkc_u[]      = { 0xc5, 0x00 };
    static  char    htorkc_xe[]     = { 0xbb, 0xb2, 0x00 };
    static  char    htorkc_e[]      = { 0xb2, 0x00 };
    static  char    htorkc_xo[]     = { 0xbb, 0xd7, 0x00 };
    static  char    htorkc_o[]      = { 0xd7, 0x00 };

    static  char    htorkc_ka[]     = { 0xc9, 0xc1, 0x00 };
    static  char    htorkc_ga[]     = { 0xb7, 0xc1, 0x00 };
    static  char    htorkc_ki[]     = { 0xc9, 0xc6, 0x00 };
    static  char    htorkc_gi[]     = { 0xb7, 0xc6, 0x00 };
    static  char    htorkc_ku[]     = { 0xc9, 0xc5, 0x00 };
    static  char    htorkc_gu[]     = { 0xb7, 0xc5, 0x00 };
    static  char    htorkc_ke[]     = { 0xc9, 0xb2, 0x00 };
    static  char    htorkc_ge[]     = { 0xb7, 0xb2, 0x00 };
    static  char    htorkc_ko[]     = { 0xc9, 0xd7, 0x00 };
    static  char    htorkc_go[]     = { 0xb7, 0xd7, 0x00 };

    static  char    htorkc_sa[]     = { 0xc4, 0xc1, 0x00 };
    static  char    htorkc_za[]     = { 0xc2, 0xc1, 0x00 };
    static  char    htorkc_si[]     = { 0xc4, 0xc6, 0x00 };
    static  char    htorkc_zi[]     = { 0xc2, 0xc6, 0x00 };
    static  char    htorkc_su[]     = { 0xc4, 0xc5, 0x00 };
    static  char    htorkc_zu[]     = { 0xc2, 0xc5, 0x00 };
    static  char    htorkc_se[]     = { 0xc4, 0xb2, 0x00 };
    static  char    htorkc_ze[]     = { 0xc2, 0xb2, 0x00 };
    static  char    htorkc_so[]     = { 0xc4, 0xd7, 0x00 };
    static  char    htorkc_zo[]     = { 0xc2, 0xd7, 0x00 };

    static  char    htorkc_ta[]     = { 0xb6, 0xc1, 0x00 };
    static  char    htorkc_da[]     = { 0xbc, 0xc1, 0x00 };
    static  char    htorkc_ti[]     = { 0xb6, 0xc6, 0x00 };
    static  char    htorkc_di[]     = { 0xbc, 0xc6, 0x00 };
    static  char    htorkc_xtu[]    = { 0xbb, 0xb6, 0xc5, 0x00 };
    static  char    htorkc_tu[]     = { 0xb6, 0xc5, 0x00 };
    static  char    htorkc_du[]     = { 0xbc, 0xc5, 0x00 };
    static  char    htorkc_te[]     = { 0xb6, 0xb2, 0x00 };
    static  char    htorkc_de[]     = { 0xbc, 0xb2, 0x00 };
    static  char    htorkc_to[]     = { 0xb6, 0xd7, 0x00 };
    static  char    htorkc_do[]     = { 0xbc, 0xd7, 0x00 };

    static  char    htorkc_na[]     = { 0xd0, 0xc1, 0x00 };
    static  char    htorkc_ni[]     = { 0xd0, 0xc6, 0x00 };
    static  char    htorkc_nu[]     = { 0xd0, 0xc5, 0x00 };
    static  char    htorkc_ne[]     = { 0xd0, 0xb2, 0x00 };
    static  char    htorkc_no[]     = { 0xd0, 0xd7, 0x00 };

    static  char    htorkc_ha[]     = { 0xb8, 0xc1, 0x00 };
    static  char    htorkc_ba[]     = { 0xba, 0xc1, 0x00 };
    static  char    htorkc_pa[]     = { 0xbe, 0xc1, 0x00 };
    static  char    htorkc_hi[]     = { 0xb8, 0xc6, 0x00 };
    static  char    htorkc_bi[]     = { 0xba, 0xc6, 0x00 };
    static  char    htorkc_pi[]     = { 0xbe, 0xc6, 0x00 };
    static  char    htorkc_hu[]     = { 0xb8, 0xc5, 0x00 };
    static  char    htorkc_bu[]     = { 0xba, 0xc5, 0x00 };
    static  char    htorkc_pu[]     = { 0xbe, 0xc5, 0x00 };
    static  char    htorkc_he[]     = { 0xb8, 0xb2, 0x00 };
    static  char    htorkc_be[]     = { 0xba, 0xb2, 0x00 };
    static  char    htorkc_pe[]     = { 0xbe, 0xb2, 0x00 };
    static  char    htorkc_ho[]     = { 0xb8, 0xd7, 0x00 };
    static  char    htorkc_bo[]     = { 0xba, 0xd7, 0x00 };
    static  char    htorkc_po[]     = { 0xbe, 0xd7, 0x00 };

    static  char    htorkc_ma[]     = { 0xd3, 0xc1, 0x00 };
    static  char    htorkc_mi[]     = { 0xd3, 0xc6, 0x00 };
    static  char    htorkc_mu[]     = { 0xd3, 0xc5, 0x00 };
    static  char    htorkc_me[]     = { 0xd3, 0xb2, 0x00 };
    static  char    htorkc_mo[]     = { 0xd3, 0xd7, 0x00 };

    static  char    htorkc_xya[]    = { 0xbb, 0xdd, 0xc1, 0x00 };
    static  char    htorkc_ya[]     = { 0xdd, 0xc1, 0x00 };
    static  char    htorkc_xyu[]    = { 0xbb, 0xdd, 0xc5, 0x00 };
    static  char    htorkc_yu[]     = { 0xdd, 0xc5, 0x00 };
    static  char    htorkc_xyo[]    = { 0xbb, 0xdd, 0xd7, 0x00 };
    static  char    htorkc_yo[]     = { 0xdd, 0xd7, 0x00 };

    static  char    htorkc_ra[]     = { 0xbd, 0xc1, 0x00 };
    static  char    htorkc_ri[]     = { 0xbd, 0xc6, 0x00 };
    static  char    htorkc_ru[]     = { 0xbd, 0xc5, 0x00 };
    static  char    htorkc_re[]     = { 0xbd, 0xb2, 0x00 };
    static  char    htorkc_ro[]     = { 0xbd, 0xb2, 0x00 };

    static  char    htorkc_xwa[]    = { 0xbb, 0xc3, 0xc1, 0x00 };
    static  char    htorkc_wa[]     = { 0xc3, 0xc1, 0x00 };
    static  char    htorkc_wi[]     = { 0xc3, 0xc6, 0x00 };
    static  char    htorkc_we[]     = { 0xc3, 0xb2, 0x00 };
    static  char    htorkc_wo[]     = { 0xc3, 0xd7, 0x00 };
    static  char    htorkc_nn[]     = { 0xd0, 0xd0, 0x00 };

    static  char    *htorkctbl[] = {
	    /* A */
	    htorkc_xa, htorkc_a, htorkc_xi, htorkc_i, htorkc_xu,
	    htorkc_u, htorkc_xe, htorkc_e, htorkc_xo, htorkc_o,
	    /* KA */
	    htorkc_ka, htorkc_ga, htorkc_ki, htorkc_gi, htorkc_ku,
	    htorkc_gu, htorkc_ke, htorkc_ge, htorkc_ko, htorkc_go,
	    /* SA */
	    htorkc_sa, htorkc_za, htorkc_si, htorkc_zi, htorkc_su,
	    htorkc_zu, htorkc_se, htorkc_ze, htorkc_so, htorkc_zo,
	    /* TA */
	    htorkc_ta, htorkc_da, htorkc_ti, htorkc_di, htorkc_xtu,
	    htorkc_tu, htorkc_du, htorkc_te, htorkc_de, htorkc_to,
	    htorkc_do,
	    /* NA */
	    htorkc_na, htorkc_ni, htorkc_nu, htorkc_ne, htorkc_no,
	    /* HA */
	    htorkc_ha, htorkc_ba, htorkc_pa, htorkc_hi, htorkc_bi,
	    htorkc_pi, htorkc_hu, htorkc_bu, htorkc_pu, htorkc_he,
	    htorkc_be, htorkc_pe, htorkc_ho, htorkc_bo, htorkc_po,
	    /* MA */
	    htorkc_ma, htorkc_mi, htorkc_mu, htorkc_me, htorkc_mo,
	    /* YA */
	    htorkc_xya, htorkc_ya, htorkc_xyu, htorkc_yu, htorkc_xyo,
	    htorkc_yo,
	    /* RA */
	    htorkc_ra, htorkc_ri, htorkc_ru, htorkc_re, htorkc_ro,
	    /* WA */
	    htorkc_xwa, htorkc_wa, htorkc_wi, htorkc_we, htorkc_wo,
	    htorkc_nn
    };

    int             offset ;
    char            *str ;

    offset = dhira - HIRAGANA_A ;

    *nbytes = 0 ;
    str = htorkctbl[offset] ;
    while ( str[*nbytes] != 0 )
	(*nbytes) ++ ;

    return str ;

}
