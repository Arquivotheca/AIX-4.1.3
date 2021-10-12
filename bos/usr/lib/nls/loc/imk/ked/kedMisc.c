static char sccsid[] = "@(#)72	1.1  src/bos/usr/lib/nls/loc/imk/ked/kedMisc.c, libkr, bos411, 9428A410j 5/25/92 15:42:40";
/*
 * COMPONENT_NAME :	(KRIM) - AIX Input Method
 *
 * FUNCTIONS :		kedMisc.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************
 *
 *  Component:    Korean IM ED  
 *
 *  Module:       kedMisc.c
 *
 *  Description:  
 *
 *  Functions:    kedGetEchoBufLen()
 *		  kedIsNormalMode()
 *		  kedIsSuppressedMode()
 *   		  kedSetNeedIndRedraw()
 * 		  kedGetFixBufLen()
 *		  kedGetAuxSize()
 *		  kedGetCurPos()
 *  		  kedGetAuxCurPos()
 * 		  kedGetAuxType()
 * 		  kedGetInterState()
 * 		  kedIsEchoBufChanged()
 *		  kedGetChangeInfoEchoBuf()
 *    		  kedIsAuxBufChanged()
 *		  kedIsCurPosChanged()
 *    		  kedIsAuxCurPosChanged()
 *		  kedIsBeepRequested()
 *		  kedGetInputMode()
 *		  kedSetInterState()
 *		  kedSetInputMode()
 *		  kedIsInputModeChanged()
 *
 *  History:      
 * 
 ******************************************************************/

/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/

#include <stdio.h>
#include "kedconst.h"
#include "ked.h"

/*-----------------------------------------------------------------------*
*	Beginning of get buffer length
*  		kedGetEchoBufLen()
*  		kedGetFixBufLen()
*  		kedGetAuxSize()
*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*
*	Beginning of kedGetEchoBufLen
*-----------------------------------------------------------------------*/
int     kedGetEchoBufLen(kimed)
KIMED   *kimed ;
{
	return (kimed->echoacsz) ;
}

/*-----------------------------------------------------------------------*
*	Beginning of kedIsNormalMode
*-----------------------------------------------------------------------*/
int	kedIsNormalMode(kimed)
KIMED	*kimed;
{
	if (kimed->imode.supnormode == MD_NORM) return (TRUE);
	else return (FALSE);
}

/*-----------------------------------------------------------------------*
*	Beginning of kedIsSuppressedMode
*-----------------------------------------------------------------------*/
int	kedIsSuppressedMode(kimed)
KIMED	*kimed;
{
	if (kimed->imode.supnormode == MD_SUPP) return (TRUE);
	else return (FALSE);
}

/*-----------------------------------------------------------------------*
*	Beginning of kedSetNeedIndRedraw
*-----------------------------------------------------------------------*/
int	kedSetNeedIndRedraw(kimed)
KIMED	*kimed;
{
	kimed->needindrw = ON;
}

/*-----------------------------------------------------------------------*
*	Beginning of kedGetFixBufLen
*-----------------------------------------------------------------------*/
int     kedGetFixBufLen(kimed)
KIMED   *kimed ;
{
	return (kimed->fixacsz) ;
}

/*-----------------------------------------------------------------------*
*	Beginning of kedGetAuxSize
*-----------------------------------------------------------------------*/
AuxSize     kedGetAuxSize(kimed)
KIMED   *kimed ;
{
	return (kimed->auxacsz) ;
}

/*-----------------------------------------------------------------------*
*	Beginning of kedGetCurPos
*-----------------------------------------------------------------------*/
int     kedGetCurPos(kimed)
KIMED   *kimed ;
{
		return (kimed->echocrps) ;
}

/*-----------------------------------------------------------------------*
*	Beginning of kedGetAuxCurPos
*-----------------------------------------------------------------------*/
AuxCurPos     kedGetAuxCurPos(kimed)
KIMED   *kimed ;
{
	return (kimed->auxcrps) ;
}

/*-----------------------------------------------------------------------*
*	Beginning of get auxiliary are type 
*-----------------------------------------------------------------------*/
int     kedGetAuxType(kimed)
KIMED   *kimed ;
{
	switch (kimed->interstate) {
		case ST_MULTI:	 return MULTICAND_USED;
		case ST_CODEINP: return CODEINPUT_USED;
		default: return	 AUXBUF_NOTUSED;
	}
}

/*-----------------------------------------------------------------------*
*	Beginning of kedGetInterState
*-----------------------------------------------------------------------*/
int		kedGetInterState(kimed)
KIMED		*kimed;
{
		return (kimed->interstate);
}

/*-----------------------------------------------------------------------*
*	Beginning of kedIsEchoBufChanged
*-----------------------------------------------------------------------*/
int	     	kedIsEchoBufChanged(kimed)
KIMED   	*kimed ;
{
    return(kimed->echochfg.flag);
}

/*-----------------------------------------------------------------------*
*	Beginning of kedGetChangeInfoEchoBuf
*-----------------------------------------------------------------------*/
EchoBufChanged  kedGetChangeInfoEchoBuf(kimed)
KIMED           *kimed ;
{
    return(kimed->echochfg);
}

/*-----------------------------------------------------------------------*
*	Beginning of kedIsAuxBufChanged
*-----------------------------------------------------------------------*/
int     kedIsAuxBufChanged(kimed)
KIMED   *kimed ;
{
	return ( kimed->auxchfg );
}

/*-----------------------------------------------------------------------*
*	Beginning of kedIsCurPosChanged
*-----------------------------------------------------------------------*/
int     kedIsCurPosChanged(kimed)
KIMED   *kimed ;
{
	return (kimed->eccrpsch);
}

/*-----------------------------------------------------------------------*
*	Beginning of kedIsAuxCurPosChanged
*-----------------------------------------------------------------------*/
int     kedIsAuxCurPosChanged(kimed)
KIMED   *kimed ;
{
	return (kimed->axcrpsch);
}

/*-----------------------------------------------------------------------*
*	Beginning of kedIsBeepRequested
*-----------------------------------------------------------------------*/
int kedIsBeepRequested(kimed)
KIMED   *kimed ;
{
	return (kimed->isbeep);
}

/*-----------------------------------------------------------------------*
*	Beginning of kedGetInputMode
*-----------------------------------------------------------------------*/
InputMode	kedGetInputMode(kimed)
KIMED   	*kimed;
{
	return (kimed->imode);
}

/*-----------------------------------------------------------------------*
*	Beginning of kedSetInterState
*-----------------------------------------------------------------------*/
int		kedSetInterState(kimed, imode)
KIMED		*kimed;
InputMode	imode;
{
		switch(imode.basemode) {
			case MD_ENG: 
				kimed->interstate = ST_ENG;
				break;
			case MD_HAN:		
				kimed->interstate = ST_HAN;
				break;
			case MD_JAMO:
				kimed->prevhemode = ST_ENG;
				kimed->interstate = ST_JAMO;
				break;
			default:
				return (KP_ERR);
		}
}

/*-----------------------------------------------------------------------*
*	Beginning of kedSetInputMode
*-----------------------------------------------------------------------*/
int		kedSetInputMode(kimed, imode)
KIMED		*kimed;
InputMode	imode;
{
	/*
         *****************************************************
	 * First, checks if all fields contain valid values.
         *****************************************************
         */
	switch(imode.basemode) {
		case MD_ENG:
		case MD_HAN:
		case MD_JAMO:
			break;
		default:
			return (KP_ERR);
	}
	switch(imode.sizemode) {
		case MD_BANJA:
		case MD_JEONJA:
			break;
		default:
			return (KP_ERR);
	}
	switch(imode.hjmode) {
		case MD_HJON:
		case MD_HJOFF:
			break;
		default:
			return (KP_ERR);
	}
	switch(imode.supnormode) {
		case MD_NORM:
		case MD_SUPP:
			break;
		default:
			return (KP_ERR);
	}
	switch(imode.insrepmode) {
		case MD_INSERT:
		case MD_REPLACE:
			break;
		default:
			return (KP_ERR);
	}

	/*
	 ***********************************************
	 * Updates kimed->imode, and updates indicator.
	 ***********************************************
	 */
	/* basemode */
	switch(imode.basemode) {
		case MD_ENG:
			if (kimed->imode.basemode != MD_ENG) {
				kimed->imode.basemode = MD_ENG;
				kimed->indchfg = ON;
			}
			break;
		case MD_HAN:
			if (kimed->imode.basemode != MD_HAN) {
				kimed->imode.basemode = MD_HAN;
				kimed->indchfg = ON;
			}
			break;
		case MD_JAMO:
			if (kimed->imode.basemode != MD_JAMO) {
				kimed->imode.basemode = MD_JAMO;
				kimed->indchfg = ON;
			}
			break;
		default:
			break;
	} 

	/* sizemode */
	switch(imode.sizemode) {
		case MD_BANJA:
			if (kimed->imode.sizemode != MD_BANJA) {
				kimed->imode.sizemode = MD_BANJA;
				kimed->indchfg = ON;
			}
			break; 
		case MD_JEONJA:
			if (kimed->imode.sizemode != MD_JEONJA) {
				kimed->imode.sizemode = MD_JEONJA;
				kimed->indchfg = ON;
			}
		default:
			break;
	}

	/* hjmode */
	switch(imode.hjmode) {
		case MD_HJOFF:
			if (kimed->imode.hjmode != MD_HJOFF) {
				kimed->imode.hjmode = MD_HJOFF;
				kimed->indchfg = ON;
			}
			break;
		case MD_HJON:
			if (kimed->imode.hjmode != MD_HJON) {
				kimed->imode.hjmode = MD_HJON;
				kimed->indchfg = ON;
			}
		default:
			break;
	}

        /* insrepmode */
        switch(imode.insrepmode) {
                case MD_INSERT:
			if (kimed->imode.insrepmode != MD_INSERT) {
                        	kimed->imode.insrepmode = MD_INSERT;
				kimed->indchfg = ON;
			}
                        break;
                case MD_REPLACE:
			if (kimed->imode.insrepmode != MD_REPLACE) {
                        	kimed->imode.insrepmode = MD_REPLACE;
				kimed->indchfg = ON;
			}
                default:
                        break;
        }

	/* supnormode */
	switch(imode.supnormode) { 
		case MD_NORM:
			kimed->imode.supnormode = MD_NORM;
			break;
		case MD_SUPP:
			kimed->imode.supnormode = MD_SUPP;
		default:
			break;
	}

	return (KP_OK);
}

/*-----------------------------------------------------------------------*
*	Beginning of kedIsInputModeChanged
*-----------------------------------------------------------------------*/
int	kedIsInputModeChanged( kimed )
KIMED	*kimed;
{
	return	(kimed->indchfg);
}
