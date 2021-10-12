static char sccsid[] = "@(#)07  1.4  src/bos/usr/lib/nls/loc/imt/tfep/tedclose.c, libtw, bos411, 9428A410j 4/21/94 01:55:40";
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: TedClose
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
/* MODULE NAME:         TedClose                                              */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method Editor Close                     */
/*                                                                            */
/* FUNCTION:            TedClose : Free Internal Used Data Structure.         */
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
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/
/*----------------------------------------------------------------------------*/
/*                      include files                                         */
/*----------------------------------------------------------------------------*/
  #include "ted.h"
  #include "tedinit.h"
/*----------------------------------------------------------------------------*/
/*                      external reference                                    */
/*----------------------------------------------------------------------------*/
  extern  StjFreeCandidates();
  extern  PhFreeCandidates();
/*----------------------------------------------------------------------------*/
/*                      Beginning of procedure                                */
/*----------------------------------------------------------------------------*/

int      TedClose(fepcb)
FEPCB   *fepcb ;
{
         free(fepcb->preinpbuf); /* free internal structure pointer space */
         free(fepcb->curinpbuf);
         free(fepcb->preedbuf);
         free(fepcb->ctrl_r_buf);
         free(fepcb->radeucbuf);
         TedFreeDictionaryName(fepcb);
         StjFreeCandidates(fepcb); /* free Simplied Tsang-Jye structure   */
         PhFreeCandidates(fepcb);  /* free Phonetic structure             */
         if(fepcb->mi.phsysmi != NULL) free(fepcb->mi.phsysmi);
         if(fepcb->mi.phusrmi != NULL) free(fepcb->mi.phusrmi);
         if(fepcb->fd.phsysfd != NULL) fclose(fepcb->fd.phsysfd);
         if(fepcb->fd.tjsysfd != NULL) fclose(fepcb->fd.tjsysfd);
         if(fepcb->fd.tjusrfd != NULL) fclose(fepcb->fd.tjusrfd);
         if(fepcb->fd.phusrfd != NULL) fclose(fepcb->fd.phusrfd);
         if(fepcb->stjstruct.stjcand != NULL) free(fepcb->stjstruct.stjcand);
         if(fepcb->strokestruct.strokecand != NULL)
            free(fepcb->strokestruct.strokecand);
         if(fepcb->phstruct.phcand != NULL) free(fepcb->phstruct.phcand);
         if(fepcb->phstruct.curptr != NULL) free(fepcb->phstruct.curptr);
         if (fepcb->learning)                             /* V410 */
            free_learn_mem(fepcb);                        /* V410 */
         if (fepcb->iconv_flag)                           /* @big5 */
            CloseIconv(fepcb->iconv_flag);                /* @big5 */
         free(fepcb);              /* free control block                  */
         return( OK );
}

