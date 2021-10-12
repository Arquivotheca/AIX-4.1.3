static char sccsid[] = "@(#)05	1.1  src/bos/usr/lib/nls/loc/CN.im/cnedclear.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:33:22";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: CNedClear
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATION ***************************/
/*                                                                            */
/* MODULE NAME:         CNedClear                                             */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method Clear                            */
/*                                                                            */
/* FUNCTION:            CNedClear : Reset Internal Used Data Structure.       */
/*                                                                            */
/* MODULE TYPE:         C                                                     */
/*                                                                            */
/* COMPILER:            AIX C                                                 */
/*                                                                            */
/* AUTHOR:                                                                    */
/*                                                                            */
/* STATUS:              Chinese Input Method Version 1.0                      */
/*                                                                            */
/* CHANGE ACTIVITY:                                                           */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/
/*----------------------------------------------------------------------------*/
/*                      include files                                         */
/*----------------------------------------------------------------------------*/
#include "cned.h"
#include "cnedinit.h"
/*----------------------------------------------------------------------------*/
/*                      Beginning of procedure                                */
/*----------------------------------------------------------------------------*/
int     CNedClear( fepcb )
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
        fepcb->flag  = OFF ;
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
        fepcb->inputlen = 0;
        memset(fepcb->rcinbuf,NULL,fepcb->echosize);
        memset(fepcb->pyinbuf,NULL,fepcb->echosize);
        memset(fepcb->eninbuf,NULL,fepcb->echosize);
        memset(fepcb->fssinbuf,NULL,fepcb->echosize);
        memset(fepcb->fsinbuf,NULL,fepcb->echosize);
        memset(fepcb->abcinbuf,NULL,fepcb->echosize);
        memset(fepcb->udinbuf,NULL,fepcb->echosize);
        fepcb->ret = 0;

        if(fepcb->imode.ind0 ==ROW_COLUMN_MODE) 
        {
           RcInitial(fepcb);
        }

        return ( OK );

} /* end of cnedClear  */
