static char sccsid[] = "@(#)58	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcobro2.c, libKJI, bos411, 9428A410j 6/4/91 15:16:47";
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
 * MODULE NAME:       _Kcobro2
 *
 * DESCRIPTIVE NAME:  SET SOME CANDIDATES TO OUTPUT BUFFER
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS) : Success
 *                    0x0002 (END_CAND): End of candidate
 *                    0x0004 (NO_CAND) : No candidate
 *                    0x7fff (UERROR)  : Unpredictable error
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
short _Kcobro2(z_kcbptr,z_func)

struct KCB    *z_kcbptr;                /* pointer of KCB               */
short         z_func;                   /* function for other candidate */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short           _Kcrsout();   /* Set BTE data on Output Buffer*/
   extern void            _Kcrinit();   /* Clear Output Buffers         */

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short           z_ret   ;    /* Define Area for Return of Own        */
   short           z_rrsout;    /* Define Area for Return of _Kcrsout   */
   short           z_i;         /* counter                              */
   short           z_j;         /* counter                              */
   uschar         *z_seiptr;    /* seisho buffer pointer                */
   uschar         *z_semptr;    /* seisho map buffer pointer            */
   uschar         *z_grmptr;    /* grammer map buffer pointer           */
   uschar         *z_ymmptr;    /* yomi map buffer pointer              */
   short           z_over;      /* buffer over flow flag                */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */

   switch ( z_func )
   {
/*----------------------------------------------------------------------*
 *       FORWARD
 *----------------------------------------------------------------------*/
      case ALLFW:
         /*-------     check BTE is not empty                  ---------*/
         CMOVT(kcb.bthchh,bteptr1 );    /* get top BTE pointer          */
         if( bteptr1 == NULL)           /* if no candidate              */
            return( NO_CAND );          /* return                       */


         /*-------   point the bottom of previous candidates   ---------*/
                                        /* return to first candidate    */
         if ( kcb.posend == kcb.totcand )
         {
            z_i = (short)1;
         }
         else
         {
            for ( z_i = 1; z_i <= kcb.posend; z_i++ )
            {
               CMOVF(kcb.bthchh, bteptr1 );
            } 
         }

         /*--------------   get last of next candidates   --------------*/
                                        /* if last candidate            */
         if ( ( z_i + kcb.reqcand - (short)1 ) >= kcb.totcand )
         {
            kcb.posend = kcb.totcand;
            z_ret = END_CAND;
         }

         else
         {
            kcb.posend = z_i + kcb.reqcand - (short)1;
            z_ret = SUCCESS;
         }

         /*------------   set candidates to output buffer   ------------*/
         _Kcrinit(z_kcbptr);            /* initialize output buffers    */

         for ( z_j = z_i; z_j <= kcb.posend; z_j++ )
         {
            z_seiptr = (uschar *)kcb.seiaddr + kcb.seill;
            z_semptr = (uschar *)kcb.semaddr + kcb.semll;
            z_grmptr = (uschar *)kcb.grmaddr + kcb.grmll;
            z_ymmptr = (uschar *)kcb.ymmaddr + kcb.ymmll;

            *z_seiptr = 0x00;
            *(z_seiptr + 1) = 0x02;

            if ( kcb.env != ENVTAN )
            {
               *z_semptr = 0x00;
               *(z_semptr + 1) = 0x02;
               *z_grmptr = 0x01;
               *z_ymmptr = 0x01;
            }

            z_rrsout = _Kcrsout(z_kcbptr, bteptr1 ,
                                     z_seiptr,z_semptr,z_grmptr,z_ymmptr);

            if(( z_rrsout != SUCCESS)&&
               ( z_rrsout != SEIOVER)&&
               ( z_rrsout != SEMOVER)&&
               ( z_rrsout != YMMOVER)&&
               ( z_rrsout != GRMOVER))
            {
               return( UERROR );
            }

            if ( z_rrsout != SUCCESS )
            {
               kcb.posend = z_j - 1;
               z_ret = SUCCESS;
               break;
            }

            kcb.seill += CHPTTOSH((uschar *)kcb.seiaddr+kcb.seill);
                                        /* add seisho length            */
            kcb.semll += CHPTTOSH((uschar *)kcb.semaddr+kcb.semll);
                                        /* add seisho map length        */
            kcb.grmll += (short)(*((uschar *)kcb.grmaddr+kcb.grmll));
                                        /* add grammer map length       */
            kcb.ymmll += (short)(*((uschar *)kcb.ymmaddr+kcb.ymmll));
                                        /* add yomi map length          */
            CMOVF( kcb.bthchh,bteptr1 );           /* get next BTE pointer         */

         }

         kcb.possta = z_i;

         kcb.outcand = kcb.posend - kcb.possta + 1;

         break;
            
/*----------------------------------------------------------------------*
*       BACKWARD                                                        *
*-----------------------------------------------------------------------*/
      case ALLBW:
         /*-------     check BTE is not empty                  ---------*/
         CMOVL(kcb.bthchh,bteptr1 );    /* get top BTE pointer          */
         if( bteptr1 == NULL)           /* if no candidate              */
            return( NO_CAND );          /* return                       */

         /*-----------   point top of previous candidates   ------------*/
         if ( kcb.possta == 1 )
         {
            z_i = kcb.totcand;
         }

         else
         {
            for ( z_i = kcb.totcand; z_i >= kcb.possta; z_i-- )
            {
               CMOVB(kcb.bthchh, bteptr1 );
            } 
         } 

         /*------------   get last of privious candidates   ------------*/
         if ( ( z_i - kcb.reqcand ) < (short)1 )
         {
            z_ret = END_CAND;
            kcb.possta = 1;
         }
         else
         {
            z_ret = SUCCESS;
            kcb.possta = z_i - kcb.reqcand + 1;
         }

         /*------------   set candidates to output buffer   ------------*/
         z_over = (short)YES;

         while ( z_over == (short)YES )
         {
            z_over = (short)NO;

            _Kcrinit(z_kcbptr);

            for ( z_j = z_i; z_j >= kcb.possta ; z_j-- )
            {
               CMOVB(kcb.bthchh, bteptr1 );
            } 

            for ( z_j = kcb.possta;
                        z_j <= z_i; z_j++ )
            {
               CMOVF(kcb.bthchh, bteptr1 );/* get next BTE pointer      */

               z_seiptr = (uschar *)kcb.seiaddr + kcb.seill;
               z_semptr = (uschar *)kcb.semaddr + kcb.semll;
               z_grmptr = (uschar *)kcb.grmaddr + kcb.grmll;
               z_ymmptr = (uschar *)kcb.ymmaddr + kcb.ymmll;

               *z_seiptr = 0x00;
               *(z_seiptr + 1) = 0x02;

               if ( kcb.env != ENVTAN )
               {
                  *z_semptr = 0x00;
                  *(z_semptr + 1) = 0x02;
                  *z_grmptr = 0x01;
                  *z_ymmptr = 0x01;
               }

               z_rrsout = _Kcrsout(z_kcbptr , bteptr1,
                                     z_seiptr,z_semptr,z_grmptr,z_ymmptr);

               if(( z_rrsout != SUCCESS)&&
                  ( z_rrsout != SEIOVER)&&
                  ( z_rrsout != SEMOVER)&&
                  ( z_rrsout != YMMOVER)&&
                  ( z_rrsout != GRMOVER))
               {
                  return( UERROR );
               }

               if ( z_rrsout != SUCCESS )
               {
                  z_over = (short)YES;
                  kcb.possta++;
                  z_ret = SUCCESS;
                  break;
               }

               kcb.seill += CHPTTOSH((uschar *)kcb.seiaddr+kcb.seill);
                                        /* add seisho length            */
               kcb.semll += CHPTTOSH((uschar *)kcb.semaddr+kcb.semll);
                                        /* add seisho map length        */
               kcb.grmll += (short)(*((uschar *)kcb.grmaddr+kcb.grmll));
                                        /* add grammer map length       */
               kcb.ymmll += (short)(*((uschar *)kcb.ymmaddr+kcb.ymmll));
                                        /* add yomi map length          */


            }
         }

         kcb.posend = z_j - (short)1;

         kcb.outcand = kcb.posend - kcb.possta + (short)1;

         break;

      default:
         return((short)UERROR);
   }

   kcb.currfst = kcb.possta;
   kcb.currlst = kcb.posend;

/*----------------------------------------------------------------------*
*       RETURN                                                          *
*-----------------------------------------------------------------------*/
   return(z_ret);
}
