static char sccsid[] = "@(#)12	1.4.1.1  src/bos/usr/lpp/jls/dictutil/kudcmrua.c, cmdKJI, bos411, 9428A410j 7/23/92 00:58:52";
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: kudcmrua
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kudcmrua
 *
 * DESCRIPTIVE NAME:    insert of MRU area
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            insert of MRU area.
 *
 * NOTES:               NA.
 *
 *
 * MODULE TYPE:         Procedure
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        1096 Decimal Bytes.
 *
 *  ATTRIBUTE:          Reentrant
 *
 * ENTRY POINT:         Module Entry Point Name
 *
 *  PURPOSE:            See Function.
 *
 *  LINKAGE:            kudcmrua(udpnt, kanaln, kanadt, kanjiln, kanjidt) ;
 *
 *  INPUT:              udpnt   : user dictionary buffer pointer.
 *                      kanadt  : kana data.
 *                      kanaln  : kana length.
 *                      kanjidt : kanji data.
 *                      kanjiln : kanji length.
 *
 *  OUTPUT:             NA.
 *
 * EXIT-NORMAL:         Return Codes Returned to Caller.
 *                              NA.
 *
 * EXIT-ERROR:          Wait State Codes.
 *                              NA.
 *
 * EXTERNAL REFERENCES: See Below
 *
 *  ROUTINES:           Internal Subroutines.
 *                              NA.
 *                      Kanji Project Subroutines.
 *                              kumvch
 *                      Standard Library.
 *                              calloc
 *                              free
 *                              memcpy
 *                      Advanced Display Graphics Support Library(GSL).
 *                              NA.
 *
 *  DATA AREAS:         NA.
 *
 *  CONTROL BLOCK:      NA.
 *
 * TABLES:              Table Defines.
 *                              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              IDENTIFY:Module Identify Create.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */

/*
 *      include Standard.
 */
#include <stdio.h>      /* Standard I/O Package.                        */
/*#include <memory.h>*/ /* Memory Package                               */

/*
 *      include Kanji Project.
 */
#include "kut.h"    /* Utility Define File                          */

/*
 *      Copyright Identify.
 */
static char *cprt1="5601-125 COPYRIGHT IBM CORP 1989           ";
static char *cprt2="LICENSED MATERIAL-PROGRAM PROPERTY OF IBM  ";

void kudcmrua(udpnt, kanadt, kanaln, kanjidt, kanjiln)

 uchar   *udpnt   ;   /* user dictionary buffer pointer */
 uchar   *kanadt  ;   /* kana data                      */
 uchar    kanaln  ;   /* kana length                    */
 uchar   *kanjidt ;   /* kanji data                     */
 uchar    kanjiln ;   /* kanji length                   */

{
/* char    *calloc()         ; */
 void     free()           ;
 int      step_no          ;  /* step number            */
 short    mru_len          ;  /* MRU length             */
 char    *mru_dat          ;  /* MRU data area          */
 int      pnt              ;  /* copy position          */
 int      total_len        ;  /* total length           */
 int      l                ;  /* loop counter           */
 uchar   *dummy1           ;  /* dummy buffer           */
 uchar    kanjiwk[U_WORK]  ;  /* kanji data work buffer */
 uint     ptr1, ptr3       ;  /* MRU area check pointer */

/* if error occured , break out  */
 for( step_no = NULL ; step_no <= U_STEPNO ; step_no++)
 {
        switch(step_no)    /* select process by step number */
        {
                case U_STEP0 : /* MRU data read */
                         /* calculate entry total length */
                         total_len = kanaln + kanjiln + U_KNL + U_KJL ;
                         /* get MRU length */
                         mru_len = GMRULEN( udpnt ) ;
                         /* if initial MRU */
                         if(mru_len == NULL)
                         {
                                mru_len =  U_MLEN ; /* plus MRL itself */
                         }
                         break ;

                case U_STEP1 : /* MRU area check */
                         /* allocate MRU data area */
                         mru_dat = calloc((mru_len + total_len),
                                           sizeof(uchar)) ;
                         /* MRU data copy */
                         memcpy(mru_dat, udpnt, mru_len) ;
                         if((total_len + mru_len) > U_MRUMAX)
                         {
                                /* check loop */
                                for(ptr1 = sizeof(short);
                                    ptr1 < (U_MRUMAX - total_len);)
                                {
                                        /* copy before point */
                                        ptr3 = ptr1 ;
                                        /* get KNL */
                                        ptr1 += *(mru_dat + ptr1) ;
                                        /* get KNL + KJL */
                                        ptr1 += *(mru_dat + ptr1) ;
                                }
                                /* delete the oldest KNL */
                                mru_len = ptr3 ;
                         }
                         break ;

                case U_STEP2 : /* kanji data convert */
                         /* insert data copy */
                         memcpy(kanjiwk, kanjidt, (int)kanjiln) ;
                         /* kanji data convert */
                         for(l = NULL; l < (kanjiln - sizeof(short));
                             l += U_SHORT)
                         {
                                *(kanjiwk + l) &= U_CONV ;/* top bits off */
                         }
                         break ;


                case U_STEP3 : /* entry insert */
                         kumvch(mru_dat,    /* area move  */
                                (int)U_TPOSI,
                                (int)(mru_len - sizeof(short)),
                                (int)U_BACKWD,
                                total_len,
                                (int)FALSE,
                                dummy1,
                                NULL,
                                NULL ) ;
                         mru_len += total_len ; /* plus MRU length */
                         pnt = U_FTOP ;   /* set top position */
                         /* set MRU length */
                         SMRULEN(mru_dat, mru_len) ;
                         pnt += U_MLEN ; /* position up */
                         /* set kana length */
                         *(mru_dat + pnt) = kanaln + sizeof(kanaln) ;
                         pnt++ ;                 /* position  1 up */
                                                 /* copy kana data */
                         memcpy((mru_dat + pnt), kanadt, (int)kanaln) ;
                         pnt += kanaln ;         /* position up */
                         /* set kanji length */
                         *(mru_dat + pnt) = kanjiln + sizeof(kanjiln) ;
                         pnt++ ;                   /* position 1 up */
                                                   /* copy kanji data */
                         memcpy((mru_dat + pnt), kanjiwk, (int)kanjiln) ;
                         break ;

                case U_STEP4 : /* MRU data write */
                         /* MRU data copy */
                         memcpy(udpnt, mru_dat, (int)mru_len) ;
                         free(mru_dat) ; /* free allocated area */
                         break ;
        }

 }
 return ; /* return */
}




