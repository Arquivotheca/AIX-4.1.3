static char sccsid[] = "@(#)63	1.3.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcofkkc.c, libKJI, bos411, 9428A410j 7/23/92 03:16:34";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 * 
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcofkkc
 *
 * DESCRIPTIVE NAME:  ROOT OF KKC. EACH FUNCTION BRANCH FROM THIS ROUTINE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS):     success
 *                    0x7fff(UERROR):      unpredictable error
 *                        :                     :
 *                        :                     :
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */
#include   "_Kcrcb.h"                   /* Define Return Code Structure */
 
 
/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short _Kcofkkc(z_kcbptr)
 
struct KCB      *z_kcbptr;              /* get address of KCB           */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
/*--------------------   FIRST CONVERSION   ----------------------------*/
   extern short   _Kcoicnv();           /* first conversion  mode(0,1,2)*/
 
#if defined(_SEP)
   extern short   _Kcaicnv();           /* first conversion  mode(15)   */
   extern short   _Kckicnv();           /* first conversion  mode(17)   */
#else
   extern short   _Kcmicnv();           /* first conversion  mode(14)   */
#endif
 
#if defined(_ABR)
   extern short   _Kcabcnv();           /* first conversion  mode(16)   */
#endif
 
/*---------------------   NEXT CONVERSION   ----------------------------*/
   extern short   _Kconcnv();           /* next  conversion  mode(0,1,2)*/
#if defined(_SEP)
   extern short   _Kcancnv();           /* next  conversion  mode(15)   */
   extern short   _Kckncnv();           /* next  conversion  mode(17)   */
#else
   extern short   _Kcmncnv();           /* next  conversion  mode(14)   */
#endif
 
/*-------------------   PREVIOUS CONVERSION   --------------------------*/
   extern short   _Kcopcnv();           /* prev. conversion  mode(0,1,2)*/
#if defined(_SEP)
   extern short   _Kcapcnv();           /* prev. conversion  mode(15)   */
   extern short   _Kckpcnv();           /* prev. conversion  mode(17)   */
#else
   extern short   _Kcmpcnv();           /* prev. conversion  mode(14)   */
#endif
 
/*----------------   RETURN TO FIRST CONVERSION   ----------------------*/
   extern short   _Kcofcnv();           /* retrun to first   mode(0,1,2)*/
 
/*-----------------   RETURN TO LAST CONVERSION   ----------------------*/
   extern short   _Kcolcnv();           /* retrun to last    mode(0,1,2)*/
 
/*---------------   ALL CANDIDATES OPEN CONVERSION   -------------------*/
   extern short   _Kcobrop();           /* browse open/fwd.  mode(0,1,2)*/
#if defined(_SEP)
   extern short   _Kcabrop();           /* browse open/fwd.  mode(15)   */
   extern short   _Kckbrop();           /* browse open/fwd.  mode(17)   */
#else
   extern short   _Kcmbrop();           /* browse open/fwd.  mode(14)   */
#endif
 
/*--------------   ALL CANDIDATES FORWARD CONVERSION   -----------------*/
   extern short   _Kcobrof();           /* browse forward    mode(0,1,2)*/
 
/*--------------   ALL CANDIDATES BACKWARD CONVERSION   ----------------*/
   extern short   _Kcobrob();           /* Browse backward   mode(0,1,2)*/
 
/*----------------   SINGLE KANJI OPEN CONVERSION   --------------------*/
   extern short   _Kcsbrop();   /* Single kanji open           0x08     */
 
/*----------------------   LEARNING ON MEMORY   ------------------------*/
   extern short   _Kc0lern();   /* Learning                    0x0B     */
 
/*---------------------------   INQUIRY   ------------------------------*/
#if defined(_INQ)
   extern short   _Kc0dici();   /* User Dictionary inquirery   0x0C     */
#endif
 
/*------------------------   REGISTORATION   ---------------------------*/
   extern short   _Kc0dicr();   /* User Dictionary Addtion     0x0D     */
 
/*------------------------   LEARNING IN FILE  -------------------------*/
#ifdef _AIX
   extern short   _Kc0lrnw();   /* Writhe MRU data             0x13     */
#endif
 
/*--------------------------   INITIALIZE   ----------------------------*/
   extern short   _Kc0init();   /* Initialize work storage     0x16     */
 
/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
 
/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short               z_rc;            /* declare return code          */
   short               z_mode = 0;

#if defined(_SEP)
#else
#   if defined(_OLD)
       short               z_bak;
#   endif
#endif
/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */
 
#if defined(_SEP)
#else
#   if defined(_OLD)
       z_bak = kcb.mode;
       _Kctmix1(kcbptr1);
#   endif
#endif

/*----------------------------------------------------------------------*
 *      BRANCH EACH FUNCTION
 *----------------------------------------------------------------------*/
   switch( kcb.func )                   /* check function code          */
   {
      /*----------------------------------------------------------------*
       *       INITIAL CONVERSION       0x00
       *----------------------------------------------------------------*/
      case FUNCONV:                     /* FUNCONV ( 0x00 )             */
         switch(kcb.mode)
         {
            case MODFUKU :              /* Fukugou-go          0x00     */
            case MODTAN  :              /* Bunsetsu (Single)   0x01     */
            case MODREN  :              /* Bunsetsu (Multiple) 0x02     */
#if defined(_OLD)
      if (kcb.ymill1 == kcb.ymill2) kcb.ymill2 = 0;
#endif
               z_rc=_Kcoicnv(z_kcbptr);
               break;
 
#if defined(_SEP)
            case MODALPHA:              /* Alphabet            0x0F     */
               z_rc=_Kcaicnv(z_kcbptr);
               break;
            case MODKANSU:              /* Kansuji             0x11     */
               z_rc=_Kckicnv(z_kcbptr);
               break;
#else
            case MODALKAN:              /* Alphabet & Kansuji  0x0E     */
               z_rc=_Kcmicnv(z_kcbptr);
               break;
#endif
#if defined(_ABR)
            case MODRYAKU:              /* Ryakushou           0x10     */
               z_rc=_Kcabcnv(z_kcbptr);
               break;
#endif
            default:                    /* Invalid mode                 */
               z_rc = UERROR;
               break;
         }
         break;
 
      /*----------------------------------------------------------------*
       *       NEXT    CONVERSION
       *----------------------------------------------------------------*/
      case FUNNXTCV:                    /* FUNNXTCV ( 0x01 )            */
         switch(kcb.mode)
         {
            case MODFUKU :              /* Fukugou-go          0x00     */
            case MODTAN  :              /* Bunsetsu (Single)   0x01     */
            case MODREN  :              /* Bunsetsu (Multiple) 0x02     */
               z_rc=_Kconcnv(z_kcbptr);
               break;
 
#if defined(_SEP)
            case MODALPHA:              /* Alphabet            0x0F     */
               z_rc=_Kcancnv(z_kcbptr);
               break;
            case MODKANSU:              /* Kansuji             0x11     */
               z_rc=_Kckncnv(z_kcbptr);
               break;
#else
            case MODALKAN:              /* Alphabet & Kansuji  0x0E     */
               z_rc=_Kcmncnv(z_kcbptr);
               break;
#endif
            default:                    /* Invalid mode                 */
               z_rc = UERROR;
               break;
         }
         break;
 
      /*----------------------------------------------------------------*
       *       PRIVIOUS CONVERSION
       *----------------------------------------------------------------*/
      case FUNPRVCV:                    /* FUNPRVCV ( 0x02 )            */
         switch(kcb.mode)
         {
            case MODFUKU :              /* Fukugou-go          0x00     */
            case MODTAN  :              /* Bunsetsu (Single)   0x01     */
            case MODREN  :              /* Bunsetsu (Multiple) 0x02     */
               z_rc=_Kcopcnv(z_kcbptr);
               break;
 
#if defined(_SEP)
            case MODALPHA:              /* Alphabet            0x0F     */
               z_rc=_Kcapcnv(z_kcbptr);
               break;
            case MODKANSU:              /* Kansuji             0x11     */
               z_rc=_Kckpcnv(z_kcbptr);
               break;
#else
            case MODALKAN:              /* Alphabet & Kansuji  0x0E     */
               z_rc=_Kcmpcnv(z_kcbptr);
               break;
#endif
            default:                    /* Invalid mode                 */
               z_rc = UERROR;
         }
         break;
 
      /*----------------------------------------------------------------*
       *       RETURN TO FIRST CANDIDATE
       *----------------------------------------------------------------*/
      case FUNRTFCN:                    /* FUNRTFCN ( 0x03 )            */
         switch(kcb.mode)
         {
            case MODFUKU :              /* Fukugou-go          0x00     */
            case MODTAN  :              /* Bunsetsu (Single)   0x01     */
            case MODREN  :              /* Bunsetsu (Multiple) 0x02     */
#if defined(_SEP)
            case MODALPHA:              /* Alphabet            0x0F     */
            case MODKANSU:              /* Kansuji             0x11     */
#else
            case MODALKAN:              /* Alphabet & Kansuji  0x0E     */
#endif
               z_rc=_Kcofcnv(z_kcbptr);
               break;
            default:                    /* Invalid mode                 */
               z_rc = UERROR;
         }
         break;
 
      /*----------------------------------------------------------------*
       *       RETURN TO LAST CANDITATE
       *----------------------------------------------------------------*/
      case FUNRTLCN:                    /* FUNRTLCN ( 0x04 )            */
         switch(kcb.mode)
         {
            case MODFUKU :              /* Fukugou-go          0x00     */
            case MODTAN  :              /* Bunsetsu (Single)   0x01     */
            case MODREN  :              /* Bunsetsu (Multiple) 0x02     */
#if defined(_SEP)
            case MODALPHA:              /* Alphabet            0x0F     */
            case MODKANSU:              /* Kansuji             0x11     */
#else
            case MODALKAN:              /* Alphabet & Kansuji  0x0E     */
#endif
               z_rc=_Kcolcnv(z_kcbptr);
               break;
            default:                    /* Invalid mode                 */
               z_rc = UERROR;
         }
         break;
 
      /*----------------------------------------------------------------*
       *       ALL CANDIDATE OPEN
       *----------------------------------------------------------------*/
      case FUNALLOP:                    /* FUNALLOP ( 0x05 )            */
         switch(kcb.mode)
         {
            case MODFUKU :              /* Fukugou-go          0x00     */
            case MODTAN  :              /* Bunsetsu (Single)   0x01     */
            case MODREN  :              /* Bunsetsu (Multiple) 0x02     */
               z_rc=_Kcobrop(z_kcbptr);
               break;
 
#if defined(_SEP)
            case MODALPHA:              /* Alphabet            0x0F     */
               z_rc=_Kcabrop(z_kcbptr);
               break;
            case MODKANSU:              /* Kansuji             0x11     */
               z_rc=_Kckbrop(z_kcbptr);
               break;
#else
            case MODALKAN:              /* Alphabet & Kansuji  0x0E     */
               z_rc=_Kcmbrop(z_kcbptr);
               break;
#endif
            default:                    /* Invalid mode                 */
               z_rc = UERROR;
         }
         break;
 
      /*----------------------------------------------------------------*
       *       ALL CANDIDATE FORWARD
       *----------------------------------------------------------------*/
      case FUNALLFW:                    /* FUNALLFW ( 0x06 )            */
         switch(kcb.mode)
         {
            case MODFUKU :              /* Fukugou-go          0x00     */
            case MODTAN  :              /* Bunsetsu (Single)   0x01     */
            case MODREN  :              /* Bunsetsu (Multiple) 0x02     */
#if defined(_SEP)
            case MODALPHA:              /* Alphabet            0x0F     */
            case MODKANSU:              /* Kansuji             0x11     */
#else
            case MODALKAN:              /* Alphabet & Kansuji  0x0E     */
#endif
               z_rc=_Kcobrof(z_kcbptr);
               break;
            default:                    /* Invalid mode                 */
               z_rc = UERROR;
         }
         break;
 
      /*----------------------------------------------------------------*
       *       ALL CANDI DATE BACKWARD
       *----------------------------------------------------------------*/
      case FUNALLBW:                    /* FUNALLBW ( 0x07 )            */
         switch(kcb.mode)
         {
            case MODFUKU :              /* Fukugou-go          0x00     */
            case MODTAN  :              /* Bunsetsu (Single)   0x01     */
            case MODREN  :              /* Bunsetsu (Multiple) 0x02     */
#if defined(_SEP)
            case MODALPHA:              /* Alphabet            0x0F     */
            case MODKANSU:              /* Kansuji             0x11     */
#else
            case MODALKAN:              /* Alphabet & Kansuji  0x0E     */
#endif
               z_rc=_Kcobrob(z_kcbptr);
               break;
            default:                    /* Invalid mode                 */
               z_rc = UERROR;
         }
         break;
 
      /*----------------------------------------------------------------*
       *       SINGLE KANJI OPEN
       *----------------------------------------------------------------*/
      case FUNSGLOP:                    /* FUNSGLOP ( 0x08 )            */
          z_rc=_Kcsbrop(z_kcbptr);
          break;
 
      /*----------------------------------------------------------------*
       *       SINGLE KANJI FORWARD
       *----------------------------------------------------------------*/
      case FUNSGLFW:                    /* FUNSGLFW ( 0x09 )            */
          z_rc=_Kcobrof(z_kcbptr);
          break;
 
      /*----------------------------------------------------------------*
       *       SINGLE KANJI BACKWARD
       *----------------------------------------------------------------*/
      case FUNSGLBW:                    /* FUNSGLBW ( 0x0A )            */
          z_rc=_Kcobrob(z_kcbptr);
          break;
 
      /*----------------------------------------------------------------*
       *       LEARNING
       *----------------------------------------------------------------*/
      case FUNLEARN:                    /* FUNLEARN ( 0x0B )            */
          z_rc=_Kc0lern(z_kcbptr);
          break;
 
#if defined(_INQ)
      /*----------------------------------------------------------------*
       *       USER DICTIONARY INQ.
       *----------------------------------------------------------------*/
      case FUNDCTIQ:                    /* FUNDCTIQ ( 0x0C )            */
          z_rc=_Kc0dici(z_kcbptr);
          break;
#endif
 
      /*----------------------------------------------------------------*
       *       USER DICTIONARY ADDTION
       *----------------------------------------------------------------*/
      case FUNDCTAD:                    /* FUNDCTAD ( 0x0D )            */
          z_rc=_Kc0dicr(z_kcbptr);
          break;
 
      /*----------------------------------------------------------------*
       *       WRITE MRU DATA
       *----------------------------------------------------------------*/
#ifdef _AIX
      case FUNLRNWR:                    /* FUNLRNWR ( 0x13 )            */
          z_rc=_Kc0lrnw(z_kcbptr);
          break;
#endif
 
      /*----------------------------------------------------------------*
       *       INITIALIZE WORK STORAGE
       *----------------------------------------------------------------*/
      case FUNINITW:                    /* FUNINITW ( 0x16 )            */
         z_rc=_Kc0init(z_kcbptr);
         break;
 
      /*----------------------------------------------------------------*
       *       INVALID FUNCTION
       *----------------------------------------------------------------*/
      default:                  /* Invalid function code                */
         z_rc = UERROR;
         break;
   }
 
/*----------------------------------------------------------------------*
 *       RETURN
 *----------------------------------------------------------------------*/
#if defined(_SEP)
#else
#   if defined(_OLD)
       _Kctmix2(kcbptr1, z_bak);
#   endif
#endif
   return(z_rc);
}
