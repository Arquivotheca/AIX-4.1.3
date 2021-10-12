static char sccsid[] = "@(#)06	1.2  src/bos/usr/lib/nls/loc/imt/tfep/tedclear.c, libtw, bos411, 9428A410j 9/17/93 09:18:58";
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: TedClear
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
/* MODULE NAME:         TedClear                                              */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method Clear                            */
/*                                                                            */
/* FUNCTION:            TedClear : Reset Internal Used Data Structure.        */
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
int     TedClear( fepcb )
FEPCB   *fepcb ;
{
        /*******************/
        /* local variables */
        /*******************/
        int i;
        char **toindexs, **toindexa;

        /*********************************************/
        /* clear echo buffers, string and attributes */
        /*********************************************/
        (void)memset(fepcb->echobufs, NULL, fepcb->echosize) ;
        (void)memset(fepcb->echobufa, NULL, fepcb->echosize) ;

        /*********************************/
        /* clear edit end (fixed) buffer */
        /*********************************/
        (void)memset(fepcb->edendbuf, NULL, fepcb->echosize) ;

        /**************************************************/
        /* clear aux buffers, both string and attributes  */
        /**************************************************/
        toindexs = fepcb->auxbufs;
        toindexa = fepcb->auxbufa;
        for(i = 0; i < fepcb->auxsize.itemnum; i++) {
            (void)memset(*toindexs++, NULL, fepcb->auxsize.itemsize) ;
            (void)memset(*toindexa++, NULL, fepcb->auxsize.itemsize) ;
        }

        /********************************/
        /* update internal information  */
        /********************************/

        fepcb->edendacsz = 0;
        fepcb->auxacsz.itemsize   = 0 ;
        fepcb->auxacsz.itemnum   = 0 ;
        fepcb->echochfg.flag  = ON ;
        fepcb->echochfg.chtoppos  = 0 ;
        fepcb->echochfg.chlenbytes  = fepcb->echoacsz ;
        fepcb->echoacsz  = 0;
        fepcb->indchfg = ON;
        fepcb->auxchfg   = ON ;
        fepcb->auxuse    = NOTUSE ;
        fepcb->echocrps  = 0 ;
        fepcb->auxcrps.colpos   = -1 ;
        fepcb->auxcrps.rowpos   = -1 ;
        fepcb->eccrpsch  = ON ;
        fepcb->auxcrpsch  = ON ;
        fepcb->isbeep = OFF;
        fepcb->echoover = fepcb->echosize ;
        fepcb->imode.ind2 = SELECT_OFF;
/*      fepcb->imode.ind5 = BLANK;      ==> commented by Jim Roung      */
        fepcb->starpos = 0;
        fepcb->inputlen = 0;
        memset(fepcb->preinpbuf,NULL,fepcb->echosize);
        memset(fepcb->curinpbuf,NULL,fepcb->echosize);
        memset(fepcb->ctrl_r_buf,NULL,fepcb->echosize);
/*      memset(fepcb->radeucbuf,NULL,fepcb->echosize);  ===> Jim Roung  */
        fepcb->ret = 0;

        if(fepcb->imode.ind0 == TSANG_JYE_MODE) /* added by Jim Roung   */
           StjFreeCandidates(fepcb);

        if(fepcb->imode.ind0 == PHONETIC_MODE)  /* added by Jim Roung   */
        {
           PhFreeCandidates(fepcb);
           PhInitial(fepcb);
        }

        return ( OK );

} /* end of jedClear  */
