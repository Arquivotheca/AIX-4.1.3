/* @(#)74	1.3  src/bos/usr/lib/nls/loc/jim/jed/jedMisc.c, libKJI, bos411, 9428A410j 6/6/91 11:01:31 */
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
#include "jedexm.h"
#include "jed.h"
#include "jedint.h"

/*-----------------------------------------------------------------------*
*	Beginning of get buffer length
*  		jedGetEchoBufLen()
*  		jedGetAuxSize()
*-----------------------------------------------------------------------*/

int     jedGetEchoBufLen(fepcb)
FEPCB   *fepcb ;
{
	return (fepcb->echoacsz) ;
}

AuxSize     jedGetAuxSize(fepcb)
FEPCB   *fepcb ;
{
	return (fepcb->auxacsz) ;
}

/*-----------------------------------------------------------------------*
*	Beginning of get cursor positions
*  		jedGetCurPos()
*  		jedGetAuxCurPos()
*-----------------------------------------------------------------------*/

int     jedGetCurPos(fepcb)

FEPCB   *fepcb ;

{
		return (fepcb->echocrps) ;
}

AuxCurPos     jedGetAuxCurPos(fepcb)
FEPCB   *fepcb ;
{
	KCB     *kcb = fepcb->kcb ;

	return (fepcb->auxcrps) ;
}

/*-----------------------------------------------------------------------*
*	Beginning of get auxiliary are type 
*-----------------------------------------------------------------------*/
int     jedGetAuxType(fepcb)

FEPCB   *fepcb ;

{
	int     auxtype ;

	if ( fepcb->axconvsw == ON )
		auxtype = KP_AUX_MODESET ;
	switch( kjauxkind(fepcb->kcb) )
	{
		case AX_ALLCAN :
			auxtype = KP_AUX_ALLCAND ;
			break ;
		case AX_KJCODE :
			auxtype = KP_AUX_KANJINO ;
			break ;
		default :
			auxtype = KP_AUX_NOTUSED ;
	} /* end of switch */
	return auxtype ;
}

/*-----------------------------------------------------------------------*
*	Beginning of is buffer changed 
*  		jedIsEchoBufChanged()
*  		jedIsAuxBufChanged()
*-----------------------------------------------------------------------*/
EchoBufChanged     jedIsEchoBufChanged(fepcb)
FEPCB   *fepcb ;
{
    return(fepcb->echochfg);
}


int     jedIsAuxBufChanged(fepcb)
FEPCB   *fepcb ;
{
	if ( fepcb->auxchfg )
		return TRUE ;
	else
		return FALSE ;
}

/*-----------------------------------------------------------------------*
*	Beginning of is cursor position changed 
*  		jedIsCurPosChanged()
*  		jedIsAuxCurPosChanged()
*-----------------------------------------------------------------------*/
int     jedIsCurPosChanged(fepcb)
FEPCB   *fepcb ;
{
	if (fepcb->eccrpsch)
		return TRUE ;
	else
		return FALSE ;
}


int     jedIsAuxCurPosChanged(fepcb)
FEPCB   *fepcb ;
{
	if (fepcb->axcrpsch)
		return TRUE ;
	else
		return FALSE ;
}

/*-----------------------------------------------------------------------*
*	Beginning of is beep changed 
*  		jedIsBeepRequested
*-----------------------------------------------------------------------*/
int jedIsBeepRequested(fepcb)
FEPCB   *fepcb ;
{
    if(fepcb->beepallowed == ON)  {
	if (fepcb->isbeep)
		return TRUE ;
	else
		return FALSE ;
    }
    else /* requesting beep is not allowed */
	return(FALSE);
}

/*-----------------------------------------------------------------------*
*	Beginning of input mode 
* 		jedGetInputMode()
*  		jedSetInputMode()
*  		jedIsInputModeChanged()
*-----------------------------------------------------------------------*/
InputMode	jedGetInputMode(fepcb)
FEPCB   *fepcb ;
{
	return (fepcb->imode);
}


int	jedSetInputMode(fepcb, imode)
FEPCB	*fepcb ;
InputMode	imode;
{
	extern	void	set_imode(), set_indicator();

	/***************************************************/
        /* first, check if all fields contain valid values */
	/***************************************************/
	switch( imode.ind0 )
	{
	case KP_ALPHANUM :
	case KP_KATAKANA :
	case KP_HIRAGANA :
		break ;
	default :
		return KP_ERR ;
	}

	switch( imode.ind1 )
	{
	case KP_SINGLE :
	case KP_DOUBLE :
		break ;
	default :
		return KP_ERR ;
	}

	switch( imode.ind2 )
	{
	case KP_ROMAJI_OFF :
	case KP_ROMAJI_ON :
		break ;
	default :
		return KP_ERR ;
	}

	switch( imode.ind3 )
	{
	case KP_NORMALMODE :
	case KP_SUPPRESSEDMODE :
		break ;
	default :
		return KP_ERR ;
	}

	switch( imode.ind4 )
	{
	case KP_INSERTMODE :
		break ;
	default :
		return KP_ERR ;
	}

	/****************************************************/
	/* second, tell Kanji monitor, and update indicator */
	/****************************************************/
	switch( imode.ind0 )
	{
	case KP_ALPHANUM :
		if ( fepcb->shift[0] != ALPHANUM )
		{
			fepcb->shift[0] = ALPHANUM ;
			set_imode(fepcb) ;
			set_indicator(fepcb) ;
			fepcb->indchfg = ON ;
		}
		break ;

	case KP_KATAKANA :
		if ( fepcb->shift[0] != KATAKANA )
		{
			fepcb->shift[0] = KATAKANA ;
			set_imode(fepcb) ;
			set_indicator(fepcb) ;
			fepcb->indchfg = ON ;
		}
		break ;

	case KP_HIRAGANA :
		if ( fepcb->shift[0] != HIRAGANA )
		{
			fepcb->shift[0] = HIRAGANA ;
			set_imode(fepcb) ;
			set_indicator(fepcb) ;
			fepcb->indchfg = ON ;
		}
		break ;

	} /* end of switch ind0 */

	switch( imode.ind1 )
	{
	case KP_SINGLE :
		if ( fepcb->shift[2] != SINGLE )
		{
			fepcb->shift[2] = SINGLE ;
			set_imode(fepcb) ;
			set_indicator(fepcb) ;
			fepcb->indchfg = ON ;
		}
		break ;

	case KP_DOUBLE :
		if ( fepcb->shift[2] != DOUBLE )
		{
			fepcb->shift[2] = DOUBLE ;
			set_imode(fepcb) ;
			set_indicator(fepcb) ;
			fepcb->indchfg = ON ;
		}
		break ;
	} /* end of switch ind1 */

	switch( imode.ind2 )
	{
	case KP_ROMAJI_OFF :
		if( fepcb->shift[1] != RKC_OFF )
		{
			fepcb->shift[1] = RKC_OFF ;
			set_imode(fepcb) ;
			set_indicator(fepcb) ;
			fepcb->indchfg = ON ;
			}
		break ;

	case KP_ROMAJI_ON :
		if( fepcb->shift[1] != RKC_ON )
		{
			fepcb->shift[1] = RKC_ON ;
			set_imode(fepcb) ;
			set_indicator(fepcb) ;
			fepcb->indchfg = ON ;
		}
		break ;
	} /* end of switch ind2 */

	switch( imode.ind3 )
	{
	case KP_NORMALMODE :
		fepcb->imode.ind3 = KP_NORMALMODE;
		break ;

	case KP_SUPPRESSEDMODE :
		fepcb->imode.ind3 = KP_SUPPRESSEDMODE;
		break ;
	} /* end of switch ind3 */

	/* now, no meaning for ind4 */

	return KP_OK ;

}


int	jedIsInputModeChanged( fepcb )
FEPCB	*fepcb;
{
	return	fepcb->indchfg;
}
