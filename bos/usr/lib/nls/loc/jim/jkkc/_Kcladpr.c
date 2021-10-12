static char sccsid[] = "@(#)35	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcladpr.c, libKJI, bos411, 9428A410j 6/4/91 12:59:48";
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
 * MODULE NAME:       _Kcladpr
 *
 * DESCRIPTIVE NAME:  User dictionary Data Add
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
#include "_Kcmap.h"

/************************************************************************
 *      START OF FUNCTION                                                 
 ************************************************************************/ 
void  _Kcladpr ( mode, rbpt, rbpt1, length )

short    mode;                  /* Mode to Add type                     */
uschar  *rbpt;                  /* Pointer to Data (from Data) for RRN  */
uschar  *rbpt1;                 /* Pointer to Data (to   Data) for RRN  */
short    length;                /* Move to Data Length                  */

{

/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern void            _Kclmvch();   /* Move Character in a Dimemsion*/

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   usshort       rl,          /* From Data Active Data's Size Save Area */
                 rl1;         /* To   Data Active Data's Size Save Area */
   int           pos,                   /* Start Position(top) Work     */
                 len,                   /* Move Length(byte)   Work     */
                 i;                     /* Counter Work                 */
   uschar       *st1,                   /* Pointer to String 1          */
                *st2,                   /* Pointer to String 2          */
                 clrstr[U_REC_L];       /* Clear Data String            */


/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
/*----------------------------------------------------------------------*  
 *      1.1 Initialize
 *----------------------------------------------------------------------*/ 
   rl  = CHPTTOSH(rbpt);        /* GET from RRN Active Data's Size      */
   rl1 = CHPTTOSH(rbpt1);       /* GET to   RRN Active Data's Size      */
                                /* To RRN Active Data's Size is Zero    */
   if ( rl1 <= 0 )  rl1 = U_RLLEN;
                                /* Set to NULL Clear Data Area          */
   for ( i = 0; i < U_REC_L; i++ )  clrstr[i] = NULL;

/*----------------------------------------------------------------------*  
 *      2.1 Data Add Process (mode==1) Data Add to Left
 *----------------------------------------------------------------------*/ 
   if ( mode == U_DADDLF )
   {
      st1 = rbpt1 + rl1;                /* Copy To Address Set          */
      st2 = rbpt  + U_RLLEN;            /* Copy From Address Set        */
      memcpy ( st1, st2, length );/* Entry Copy From RRN -> To RRN      */

      pos = U_RLLEN + length;     /* Move Character Position Set        */
      len = rl - pos;             /* Move Character Length Set          */
                                  /* Character Move after Null padding  */
      _Kclmvch ( rbpt, pos, len, U_FORWD, length,
                           TRUE, clrstr, clrstr[0], len );

      rl1 += length;              /* Update RRN Active Data's Area Size */
      rl  -= length;              /* Update RRN Active Data's Area Size */
   }

/*----------------------------------------------------------------------*  
 *      2.2 Data Add Process (mode==2) Data Add to Right
 *----------------------------------------------------------------------*/ 
   if ( mode == U_DADDRT )
   {
      pos = U_RLLEN;              /* Move Character Position Set        */
      len = rl1;                  /* Move Character Length Set          */
                                  /* Character Move after Null padding  */
      _Kclmvch ( rbpt1, pos, len, U_BACKWD, length,
                            TRUE, clrstr, clrstr[0], length );

      st1 = rbpt1 + U_RLLEN;      /* Copy To Data Address Set           */
      st2 = rbpt  + rl - length;  /* Copy From Data Address Set         */
      memcpy ( st1, st2, length );/* Entry Copy From RRN -> To RRN      */

      rl1 += length;              /* Update RRN Active Data's Area Size */
      rl  -= length;              /* Update RRN Active Data's Area Size */
                                  /* Character Move after Null padding  */
      _Kclmvch ( rbpt, rl, length, U_BACKWD, length,
                           TRUE, clrstr, clrstr[0], length );
   }

/*----------------------------------------------------------------------*  
 *      2.3 Data Add Process (mode==3) Data Add to New
 *----------------------------------------------------------------------*/ 
   if ( mode == U_DADDNW )
   {
      st1 = rbpt1 + U_RLLEN;      /* Copy To Data Address Set           */
      st2 = rbpt  + U_RLLEN;      /* Copy From Data Address Set         */
      memcpy ( st1, st2, length );/* Entry Copy From RRN -> To RRN      */

      pos = U_RLLEN + length;     /* Move Character Position Set        */
      len = rl - pos;             /* Move Character Length Set          */
                                  /* Character Move after Null padding  */
      _Kclmvch ( rbpt, pos, len, U_FORWD, length,
                            TRUE, clrstr, clrstr[0], len );

      rl1  = length + U_RLLEN;    /* Update RRN Active Data's Area Size */
      rl  -= length;              /* Update RRN Active Data's Area Size */
   }

/*----------------------------------------------------------------------*  
 *      Return Code.
 *----------------------------------------------------------------------*/ 
   SHTOCHPT ( rbpt,  rl );           /* SET From RRN Active Data's Size */
   SHTOCHPT ( rbpt1, rl1 );          /* SET To   RRN Active Data's Size */
   return;
}
