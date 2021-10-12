static char sccsid[] = "@(#)38	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcldtsc.c, libKJI, bos411, 9428A410j 6/4/91 15:11:50";
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
 * MODULE NAME:       _Kcldtsc
 *
 * DESCRIPTIVE NAME:  User Dictionary Data Area Search
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0b10(RQ_RECOVER): user dict should be recovered
 *                    0x00ff(U_NOKAN ): Kana Data none
 *                    0x01ff(U_NOKNJ ): Kana Data Exist & Kanji Data none
 *                    0x02ff(U_KANKNJ): Kana Data Exist & Kanji Data Exist
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
short  _Kcldtsc ( dicdata, kanadata, kanalen, intkjdata, kjlen,
                                            datapos, datapos1 )

uschar *dicdata;       /* Pointer to RRN                                */
uschar *kanadata;      /* Pointer to Input Yomi Data                    */
short   kanalen;       /* Input Yomi Data Length                        */
uschar *intkjdata;     /* Pointer to Kanji Data                         */
short   kjlen;         /* Input Kanji Data Length                       */
short  *datapos;       /* Pointer to Data Position 0                    */
short  *datapos1;      /* Pointer to Data Position 1                    */

{
/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
   int       i,              /*  Counter Work 1                         */
             j,              /*  Counter Work 2                         */
             rc,             /*  Memory Compare return Code Area        */
             length;         /*  Yomi Data Compare Length Set Area      */
   usshort   rl,             /*  RRN Length                             */
             prelen;         /*  Previous Length Set Area               */
   uschar    knl,            /*  1 Entry Length                         */
             dl,             /*  Kanji Data Length                      */
             kjdt[U_KNJ_MX]; /*  Kanji Data Get Work                    */

/*----------------------------------------------------------------------*  
 *      1.1 Initialize
 *----------------------------------------------------------------------*/ 
   *datapos  = NULL;                    /* Clear Relative Byte Address -1*/
   *datapos1 = NULL;                    /* Clear Relative Byte Address -2*/
   rl  = CHPTTOSH(dicdata);             /* GET RRN Active Data's Size   */
   if ( rl <= NULL )  return( RQ_RECOVER ); /* This RRN is Empty        */

/*----------------------------------------------------------------------*  
 *      2.1 Data Search Process
 *----------------------------------------------------------------------*/ 
   for ( *datapos1 = U_RLLEN; *datapos1 < rl;
                              *datapos1 = ( *datapos1 + knl + dl) )
   {
   /*-------------------------------------------------------------------*  
    *      2.2 Search Yomi Data
    *-------------------------------------------------------------------*/ 
      *datapos = *datapos1 + U_KNLLEN;  /* Set Yomi Data Address        */
      knl      = *(dicdata + *datapos1);/* Set Yomi Data Length         */

                                        /* Set memcmp Length for Small  */
      length = ( (knl - U_KNLLEN) < kanalen ) ? ( knl - U_KNLLEN ) : kanalen;

                                        /* Yomi Data Compare            */
      rc = memcmp ( kanadata, (char *)(dicdata + *datapos), length);

      *datapos = *datapos1 + knl;       /* Set Kanji Data Header Address*/
      dl       = *(dicdata + (*datapos));/* Get Data Length in this Entry*/

   /*-------------------------------------------------------------------*  
    *      2.3 Search Kanji Data
    *-------------------------------------------------------------------*/ 
      if ( rc == NULL  &&  kanalen == ( knl - U_KNLLEN ) )
      {
                                        /* Set Kanji Data First Address */
         *datapos += U_DLLEN + U_RSVLEN;
         for ( i = dl - U_DLLEN - U_RSVLEN, j = 0; i > 0; i -= U_1KJ_L)
         {
                                        /* Set Kanji Data for Work Area */
            kjdt[j]   = *(dicdata + (*datapos));
            (*datapos)++;
            kjdt[j+1] = *(dicdata + (*datapos));
            (*datapos)++;
            j += U_1KJ_L;               /* Kanji Counter Count Up       */
                                        /* Kanji Data End Check (1 Data)*/
            if ( (kjdt[j-U_1KJ_L] & U_MSB_M) == U_MSB_M )
            {
                             /* Patern-3 Yomi Exist & Kanji Exist(RC=3) */
               if ( (memcmp(kjdt,intkjdata,kjlen)==NULL) && (j==kjlen) )
               {
                  *datapos -= j;        /* 1 Before Entry Address Set   */
                  return(U_KANKNJ);     /* Yomi & Kanji Exist           */
               }
               *datapos += U_RSVLEN;    /* Reserve Length Increment     */
               i        -= U_RSVLEN;    /* Reserve Length Decrement     */
               j         = 0;           /* 1 Kanji Length Clear         */
            }
         }
         *datapos -= U_RSVLEN;/* Reserve Length Decrement               */
         return(U_NOKNJ);     /* Patern-2 Yomi Exist & Kanji None (RC=2)*/
      }
      if ( rc < NULL || ( rc == NULL  &&  kanalen < ( knl - U_KNLLEN ) ) )
      {
         if ( *datapos1 == U_RLLEN )
            *datapos = U_RLLEN;         /* First Entry Address Set      */
         else
         {
            *datapos   = *datapos1;     /* Current Entry address set    */
            *datapos1 -= prelen;        /* Before Entry address set     */
         }
         return(U_NOKAN);               /* Patern-1 Yomi none(RC=0)     */
      }
      prelen = knl + dl ;               /* Previous Data Length Set     */
   }


   *datapos  = rl;                      /* SET RRN Deactive header addr.*/
   *datapos1 = rl - knl - dl;           /* SET RRN Deactive header addr.*/
   return(U_NOKAN);                     /* Patern-1 Yomi none(RC=0)     */
}
