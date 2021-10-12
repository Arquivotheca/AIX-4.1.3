static char sccsid[] = "@(#)68	1.2  src/bos/usr/lib/nls/loc/imk/ked/kedCtrl.c, libkr, bos411, 9428A410j 7/21/92 00:40:02";
/*
 * COMPONENT_NAME :	(KRIM) - AIX Input Method
 *
 * FUNCTIONS :		kedCtrl.c
 *
 * ORIGINS :		27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991
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
 *  Module:       kedCtrl.c
 *
 *  Description:  Performs control operations on the IMED 
 *                 
 *  Functions:    kedControl()
 *		  p_changelen()
 *		  p_resetaux()
 *		  p_setlang()
 *	   	  p_setcursor()
 *
 *  History:      
 * 
 ******************************************************************/

/*-----------------------------------------------------------------------*
*	Include files
*-----------------------------------------------------------------------*/

#include <sys/types.h>
#include "kedconst.h"
#include "ked.h"

/*-----------------------------------------------------------------------*
*	Beginning of procedure
*-----------------------------------------------------------------------*/
int     kedControl( kimed, op, arg )
KIMED   *kimed ;
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
	    ret = p_changelen(kimed, arg);
	break;
	case KP_RESETAUX:
	    ret = p_resetaux(kimed, arg);
	break;
	case KP_SETLANG:
	    ret = p_setlang(kimed, arg);
	break;
	case KP_SETCURSOR:
	    ret = p_setcursor(kimed, arg);
	break;
	default :
	    ret = KP_ERR ;
    }

    return ret ;
} /* end of kedCtrl */

/*-----------------------------------------------------------------------*
*	Beginning of change overflow limit length
*-----------------------------------------------------------------------*/
static p_changelen(kimed, arg)
KIMED *kimed;
char *arg;
{
    /* overflow limit should meet following criteria */
    if((int)arg > kimed->echosize)
	return(KP_ERR);
    else if((int)arg < 0)
	return(KP_ERR);
    else if((int)arg < kimed->echoacsz)
	return(KP_ERR);
    else {
	kimed->echoover = (int)arg;
	return(KP_OK);
    }
}

/*-----------------------------------------------------------------------*
*	Beginning of reset auxiliary area
*-----------------------------------------------------------------------*/
static p_resetaux(kimed, arg)
KIMED *kimed;
char *arg;
{
    int   i ;
    char  **aux_str ;
    /***************/
    /* reset flags */
    /***************/
    if (kimed->auxuse == AUXBUF_NOTUSED)
       return(KP_OK) ; 
    if (kimed->interstate == ST_MULTI)
        {

           kimed->echocrps += 2 ;
           kimed->eccrpsch = ON ;
           for(i = kimed->echosvchsp; i<kimed->echocrps; i++)
                   kimed->echobufa[i] = KP_HL_UNDER ;
           kimed->echochfg.flag = ON ;
           kimed->echochfg.chtoppos = kimed->echosvchsp ;
           kimed->echochfg.chlenbytes = kimed->echocrps - kimed->echosvchsp ;
           kimed->echosvchsp = -1 ;
           kimed->echosvchlen = 0 ;
         }  
           
    aux_str = kimed->auxbufs ;
    for(i=0;i<kimed->auxacsz.itemnum;i++)
	memset(*(aux_str)++,NULL,kimed->auxacsz.itemsize) ;
    aux_str = kimed->auxbufa ;
    for(i=0;i<kimed->auxacsz.itemsize;i++)
	memset(*(aux_str)++,NULL,kimed->auxacsz.itemsize) ; 
    kimed->auxuse = AUXBUF_NOTUSED;
    kimed->auxchfg = ON;
    kimed->axcrpsch = ON;
    kimed->auxacsz.itemsize = 0;
    kimed->auxacsz.itemnum = 0;
    kimed->auxcrps.colpos = -1 ;
    kimed->auxcrps.rowpos = -1 ;

    switch(kimed->imode.basemode)
        {
            case MD_HAN  :   kimed->interstate = ST_HAN ;
                             break ;
            case MD_ENG  :   kimed->interstate = ST_ENG ;
                             break ;
            case MD_JAMO :   kimed->interstate = ST_JAMO ;
         }
    return(KP_OK);
}

/*--------------------------*
 *	Sets imode.	 
*--------------------------*/
static 		p_setlang(kimed, arg)
KIMED 		*kimed;
char 		*arg;
{
    int ret;

    if((int)arg == MD_HAN) {
	ret = KP_OK;
    }
    return(ret);
}

/*-----------------------------------------------------------------------*
*	Beginning of set cursor position :
*	used when kedProcess return KP_UP or KP_DOWN
*-----------------------------------------------------------------------*/
static p_setcursor(kimed, arg)
KIMED *kimed;
char *arg;
{ 
	/*************************************************/
	/* cursor can be placed only within current data */
	/*************************************************/
	if((int)arg > kimed->echosize)
		return(KP_ERR);
	else if((int)arg < 0)
		return(KP_ERR);
	else if((int)arg > kimed->echoacsz)
		return(KP_ERR);
	else
	{
		kimed->echocrps = (int)arg;
		kimed->eccrpsch = ON ;
		return (KP_OK);
	}
}
