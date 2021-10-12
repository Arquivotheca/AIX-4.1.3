static char sccsid[] = "@(#)43	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kclrepl.c, libKJI, bos411, 9428A410j 6/4/91 15:13:10";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or 
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kclrepl
 *
 * DESCRIPTIVE NAME:  Index Area Data Replace
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       VOID
 *
 ******************** END OF SPECIFICATIONS *****************************/
/*----------------------------------------------------------------------*  
 *      INCLUDE FILES                                                     
 *----------------------------------------------------------------------*/ 
#if defined(_AGC) || defined(_GSW)
#include   "_Kcfnm.h"                   /* Define Names file            */
#endif
#include   "_Kcmap.h"                   /* Define Constant file         */

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
void  _Kclrepl ( rbpt, rbreppos, rbreplen, repdata, replen )

uschar *rbpt;                   /* Pointer to INDEX Area     (to   Data)*/
short   rbreppos;               /* Replace Start Postion     (to   Data)*/
short   rbreplen;               /* Replace Length            (to   Data)*/
uschar *repdata;                /* Pointer to Replace Data   (from Data)*/
short   replen;                 /* Replace Data Length       (from Data)*/

{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern void            _Kclmvch();   /* Move Character in a Dimemsion*/

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   usshort       il;                    /* IL Length for Work           */
   short         rc;                    /* return code Work             */
   int           pos,                   /* Start Position(top) Work     */
                 len,                   /* Move Length(byte)   Work     */
                 dist,                  /* Move Distance (byte)         */
                 i;                     /* Counter Work                 */
   uschar       *st1,                   /* Pointer to String 1          */
                 clrstr[U_REC_L];       /* Clear Data String            */


/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
/*----------------------------------------------------------------------*  
 *      1.1 Initialize
 *----------------------------------------------------------------------*/ 
   il  = CHPTTOSH(rbpt);       /* Active INDEX Area's Size Get          */
   for ( i = 0; i < U_REC_L; i++ )  clrstr[i] = NULL;

/*----------------------------------------------------------------------*  
 *      2.1 Move Data Index Area
 *----------------------------------------------------------------------*/ 
   dist = replen - rbreplen;        /* Calculation Move Distance        */
   pos  = rbreppos + rbreplen;      /* Calculation Move Position        */
   len  = il - pos;                 /* Calculation Move Length          */
   if ( dist > 0 )
                                    /* INDEX Area BACKWARD Move         */
      _Kclmvch ( rbpt, pos, len, U_BACKWD, dist,
                           TRUE, clrstr, clrstr[0], len );

   if ( dist < 0 )
                                    /* INDEX Area FORWARD Move          */
      _Kclmvch ( rbpt, pos, len, U_FORWD, -dist,
                          TRUE, clrstr, clrstr[0], len );

/*----------------------------------------------------------------------*  
 *      2.2 Replace Data Index Area
 *----------------------------------------------------------------------*/ 
   st1  = rbpt + rbreppos;          /* To Data Address Set              */
                                    /* INDEX Area Data Replace          */
   memcpy ( st1, repdata, replen );

/*----------------------------------------------------------------------*  
 *      Return & Value set
 *----------------------------------------------------------------------*/ 
   il += replen - rbreplen;         /* Active INDEX Area's Size Calc.   */
   SHTOCHPT(rbpt, il);              /* Set Active INDEX Area's Size     */
   return;
}
