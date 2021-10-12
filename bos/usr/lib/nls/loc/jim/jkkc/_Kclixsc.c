static char sccsid[] = "@(#)40	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kclixsc.c, libKJI, bos411, 9428A410j 6/4/91 15:12:25";
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
 * MODULE NAME:       _Kclixsc
 *
 * DESCRIPTIVE NAME:  User Dictionary Index Area Search
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
void  _Kclixsc ( dicindex, kana, kanalen, indxpos, indxpos1, lastfg )

uschar  *dicindex;      /* Pointer to Index Data Area                   */
uschar  *kana;          /* Pointer to Yomi Data                         */
short   kanalen;        /* Length of Yomi Data                          */
short   *indxpos;       /* Index Position.                              */
short   *indxpos1;      /* Index Position 1.                            */
uschar  *lastfg;        /* Last Flag.                                   */

{
/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   int     len;            /* Memory Compare Length                     */
   int     rc;             /* Memory Compare return Code                */
   usshort knl;            /* Yomi Length Save Area                     */
   usshort il;             /* Index Active Area Data's Size Save Area   */

   *lastfg  = U_FOF;           /* Reset Last Flag                       */
   il = CHPTTOSH(dicindex);    /* Get Active INDEX Area's Size          */
                               /* Initial Position Set  ( First Entry ) */
   *indxpos1 = U_ILLEN + U_STSLEN + U_HARLEN + U_NARLEN;
   *indxpos  = *indxpos1;

   while ( 1 )
   {
       knl = *(dicindex + *indxpos);       /* Get Index Yomi Length     */
                                           /* Set Memory Compare Length */
       len = ( kanalen < knl ) ? kanalen : knl;
                                           /* Memory Compare            */
       rc = memcmp ( kana, (uschar *)(dicindex + *indxpos + 1), len );
                                      /* Check Compare return Code      */
       if ( ( rc == 0 ) && ( kanalen <= len ) )  break;
       if ( rc < 0 )  break;
                                      /* Active Index Area Length Check */
       if ( ( *indxpos + knl + U_RRNLEN ) >= il )
       {
           *lastfg = U_FON;           /* Set Last Flag                  */
           break;
       }
                                      /* Set Index Area Position        */
       *indxpos1  = *indxpos;         /* Previous Position Set          */
       *indxpos  += knl + U_RRNLEN;   /* 1 Entry Length Add             */
   }

   return;

}
