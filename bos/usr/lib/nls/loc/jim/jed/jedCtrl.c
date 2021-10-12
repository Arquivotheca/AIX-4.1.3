/* @(#)69	1.3  src/bos/usr/lib/nls/loc/jim/jed/jedCtrl.c, libKJI, bos411, 9428A410j 6/6/91 11:00:32 */
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

#include <sys/types.h>
#include <string.h>
#include "jedexm.h"
#include "jed.h"
#include "jedint.h"

static int	p_changelen();
static int	p_resetaux();
static int	p_setlang();
static int	p_setcursor();
/*-----------------------------------------------------------------------*
*	Beginning of procedure
*-----------------------------------------------------------------------*/
int     jedControl( fepcb, op, arg )
FEPCB   *fepcb ;
int     op ;
char    *arg ;
{
    int         ret = KP_OK;

    /***********************************************************/
    /* dispatch appropriate routine based on command specified */
    /***********************************************************/
    switch( op )
    {
        case KP_CHANGELEN:
	    ret = p_changelen(fepcb, arg);
	break;
	case KP_RESETAUX:
	    ret = p_resetaux(fepcb, arg);
	break;
	case KP_SETLANG:
	    ret = p_setlang(fepcb, arg);
	break;
	case KP_SETCURSOR:
	    ret = p_setcursor(fepcb, arg);
	break;
	default :
	    ret = KP_ERR ;
    }

    return ret ;
} /* end of jedCtrl */

/*-----------------------------------------------------------------------*
*	Beginning of change overflow limit length
*-----------------------------------------------------------------------*/
static p_changelen(fepcb, arg)
FEPCB *fepcb;
char *arg;
{
    /* overflow limit should meet following criteria */
    if((int)arg > fepcb->echosize)
	return(KP_ERR);
    else if((int)arg < 0)
	return(KP_ERR);
    else if((int)arg < fepcb->echoacsz)
	return(KP_ERR);
    else {
	fepcb->echoover = (int)arg;
	return(KP_OK);
    }
}

/*-----------------------------------------------------------------------*
*	Beginning of reset auxiliary area
*-----------------------------------------------------------------------*/
static p_resetaux(fepcb, arg)
FEPCB *fepcb;
char *arg;
{
    /*************************************/
    /* have Kanji Monitor reset aux area */
    /*************************************/
    if(fepcb->kcb->axuse1) {
	fepcb->kcb->code = PRESET;
	fepcb->kcb->type = TYPE2;
	exkjinpr(fepcb->kcb);
	exkjinpr(fepcb->kcb);
    }

    /***************/
    /* reset flags */
    /***************/
    fepcb->auxuse = NOTUSE;
    fepcb->axconvsw = OFF;
    fepcb->auxchfg = ON;
    fepcb->axcrpsch = ON;
    fepcb->auxacsz.itemsize = 0;
    fepcb->auxacsz.itemnum = 0;

    return(KP_OK);
}

/*-----------------------------------------------------------------------*
*	Beginning of set language (DBCS or mix)
*-----------------------------------------------------------------------*/
static p_setlang(fepcb, arg)
FEPCB *fepcb;
char *arg;
{
    int ret;

    if((int)arg == KP_ONLYDBCS) {
	fepcb->dbcsormix = KP_ONLYDBCS;
	fepcb->shift[2] = DOUBLE;  /* force shift state to double byte */
	set_imode(fepcb);
	set_indicator(fepcb);
	ret = KP_OK;
    }
    else if((int)arg == KP_MIX) {
	fepcb->dbcsormix = KP_MIX;
	ret = KP_OK;
    }
    else
	ret = KP_ERR;
    return(ret);
}

/*-----------------------------------------------------------------------*
*	Beginning of set cursor position 
*-----------------------------------------------------------------------*/
static p_setcursor(fepcb, arg)
FEPCB *fepcb;
char *arg;
{ 
    int pos;
    char attchar;

    /*************************************************/
    /* cursor can be placed only within current data */
    /*************************************************/
    if((int)arg > fepcb->echosize)
	return(KP_ERR);
    else if((int)arg < 0)
	return(KP_ERR);
    else if((int)arg > fepcb->echoacsz)
	return(KP_ERR);
    else { /* input arg is valid */
	fepcb->kcb->setcsc = (int)arg;
	fepcb->kcb->setcsr = 1;
	(void)exkjcrst(fepcb->kcb);
    }

    /*****************************/
    /* cursor position changed ? */
    /*****************************/
    if ( fepcb->kcb->curcol != fepcb->echocrps )
    {
	    fepcb->echocrps = fepcb->kcb->curcol ;
	    fepcb->eccrpsch = ON ;
    }

   /*
    * updates echo buffer if it has changed
    */
    if ( fepcb->kcb->chlen > 0 )
    {
	    (void)memcpy(fepcb->echobufs, fepcb->kcb->string, 
				    fepcb->kcb->lastch) ;

	    for ( pos = 0 ; pos < fepcb->kcb->lastch ; pos++ )
	    {
		attchar = REVERSE & fepcb->kcb->hlatst[pos] ;
		if( attchar == REVERSE )
		    attchar = KP_HL_REVERSE ;
		fepcb->echobufa[pos] = attchar | KP_HL_UNDER ;
	    }


	    fepcb->echochfg.flag = ON ;
	    fepcb->echochfg.chtoppos = fepcb->kcb->chpos ;
	    fepcb->echochfg.chlenbytes = fepcb->kcb->chlen ;
	    fepcb->echoacsz = fepcb->kcb->lastch ;
    }
	return (KP_OK);
}
