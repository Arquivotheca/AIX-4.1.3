static char sccsid[] = "@(#)08	1.2  src/bos/usr/lib/nls/loc/imt/tfep/tedctrl.c, libtw, bos411, 9428A410j 9/17/93 09:19:06";
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: TedControl
 *		p_changelen
 *		p_resetaux
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATION ***************************/
/*                                                                            */
/* MODULE NAME:         TedCtrl                                               */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method Editor Control                   */
/*                                                                            */
/* FUNCTION:            TedCtrl : Interface for TIM FEP                       */
/*                                                                            */
/* MODULE TYPE:         C                                                     */
/*                                                                            */
/* COMPILER:            AIX C                                                 */
/*                                                                            */
/* AUTHOR:              Andrew Wu                                             */
/*                                                                            */
/* STATUS:              Chinese Input Method Version 1.0                      */
/*                                                                            */
/* CHANGE ACTIVITY:                                                           */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/
/*----------------------------------------------------------------------------*/
/*                      include files                                         */
/*----------------------------------------------------------------------------*/
#include "ted.h"
#include "tedinit.h"
/*----------------------------------------------------------------------------*/
/*                      Beginning of procedure                                */
/*----------------------------------------------------------------------------*/

int     TedControl( fepcb, op, arg )
FEPCB   *fepcb ;
int     op ;
char    *arg ;
{
    int         ret = OK ;

    /***********************************************************/
    /* dispatch appropriate routine based on command specified */
    /***********************************************************/
    switch( op )
    {
        case Reset_Aux:
             ret = p_resetaux(fepcb, arg);
             break;
        case Change_Length:
             ret = p_changelen(fepcb,arg);
             break;
        default :
            ret = ( ERROR ) ;
    }

    return( ret );
} /* end of jedCtrl */

/*----------------------------------------------------------------------------*/
/*                Beginning of reset auxiliary area                           */
/*----------------------------------------------------------------------------*/

static p_resetaux(fepcb, arg)
FEPCB *fepcb;
char *arg;
{
    int i;
    char **toindexs, **toindexa;

    /**************************************************/
    /* clear aux buffers, both string and attributes  */
    /**************************************************/
    toindexs = fepcb->auxbufs;
    toindexa = fepcb->auxbufa;
    for(i = 0; i < fepcb->auxsize.itemnum; i++)
    {
        (void)memset(*toindexs++, NULL, fepcb->auxsize.itemsize) ;
        (void)memset(*toindexa++, NULL, fepcb->auxsize.itemsize) ;
    }

    /***************/
    /* reset flags */
    /***************/
    fepcb->auxuse = NOTUSE;
    fepcb->auxchfg = ON;
    fepcb->auxcrpsch = ON;
    fepcb->auxacsz.itemsize = 0;
    fepcb->auxacsz.itemnum =  0;
    fepcb->imode.ind2 = SELECT_OFF;

    return(OK);
}

/*----------------------------------------------------------------------------*/
/*           Beginning of change overflow limit length                        */
/*----------------------------------------------------------------------------*/

static p_changelen(fepcb, arg)
FEPCB *fepcb;
char *arg;
{
    /* overflow limit should meet following criteria */

    if((int)arg > fepcb->echosize)
        return(ERROR);
    else if((int)arg < 0)
        return(ERROR);
    else if((int)arg < fepcb->echoacsz)
        return(ERROR);
    else {
        fepcb->echoover = (int)arg;
        return(OK);
    }
}
