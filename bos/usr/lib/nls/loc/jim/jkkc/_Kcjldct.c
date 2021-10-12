static char sccsid[] = "@(#)95	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjldct.c, libKJI, bos411, 9428A410j 6/4/91 12:53:47";
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
 * MODULE NAME:       _Kcjldct
 *
 * DESCRIPTIVE NAME:  GET A WORD FROM LONG WORD DICTIONARY
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x7fff (UERROR):  unpredictable error
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
short _Kcjldct(z_kcbptr,z_stpos,z_length)
struct KCB      *z_kcbptr;              /* get address of JTE           */
uschar z_stpos;                         /* start position of input mora */
uschar z_length;                        /* length of input mora         */
{ 
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern short           _Kcjsjrt();   /* Set Data on JTE              */
   extern void            _Kccfree();   /* Free Any Kinds of Table Entry*/

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */
#include   "_Kcjle.h"   /* Jiritsugo Long-word table Entry (JLE)        */
#include   "_Kcjkj.h"   /* Jiritsugo KanJi hyoki table entry (JKJ)      */
 
/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define  Z_FOUND     NO
#define  Z_NOTFD     YES

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short           z_rjsjrt;    /* Define Area for Return of _Kcjsjrt   */
   short           z_i;         /* define loop counter                  */
   short           z_flag;      /* define found flag                    */
   short           z_endflg;    /* define end   flag                    */
   short           z_endflgk;   /* define end   flag                    */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */ 
   mceptr1 = kcb.mchmce + z_stpos;      /* set base address of MCE      */

   mce.jdok = JD_COMPLETE;              /* set flag complete            */

/*----------------------------------------------------------------------*  
 *       SEARCH HEAD OF LONG WORD TABLE                                    
 *----------------------------------------------------------------------*/ 
   FOR_FWD(kcb.jlhchh,jleptr1,z_endflg)
   {
      LST_FWD(jleptr1,z_endflg);
                                        /* set pointer                  */
/*----------------------------------------------------------------------*  
 *       CHECK START POSITION OF INPUT MORA IS SAME AS LONG WORD           
 *----------------------------------------------------------------------*/ 
      if (jle.stap != z_stpos)          /* compare start position       */
      {                                 /* not equal                    */
         continue;                      /* proceed on next entry        */
      }                                 /* endif                        */
/*----------------------------------------------------------------------*  
*       CHECK MORA IS SAME OR NOT                                       *  
*-----------------------------------------------------------------------*/ 
      else                              /* same start position          */
      {
         z_flag = Z_FOUND;              /* set up flag for searching    */
         for(z_i=0,mceptr2=kcb.mchmce+z_stpos+3;
             z_i<z_length-3;z_i++,mceptr2++)
         {                              /* compare all yomi             */
            if (jle.yomi[z_i] != mce2.code)
                                        /* if long word  is not same as */
            {                           /* input mora yomi              */
               z_flag = Z_NOTFD;        /* set flag and continue        */
               break;                   /* break loop                   */
            }                           /* endif (compare yomi)         */
         }                              /* end of loop                  */
         if (z_flag != Z_NOTFD)         /* if not same word             */
         {
            if (z_length == jle.len)    /* compare input and long on len*/
            {
               z_rjsjrt=_Kcjsjrt(z_kcbptr,/* call jiritsu-go set routine*/
                           jle.hinpos,  /* hinsi pointer                */
                           jle.dflag,   /* dflag                        */
                           jle.stap,    /* start postion                */
                           jle.len,     /* length                       */
                           jle.dtype,   /* dtype                        */
                           jle.jkjaddr);/* kanji pointer                */
               if ( z_rjsjrt != SUCCESS )
                  return( z_rjsjrt );

               mce.jdok = JD_COMPLETE;  /* set OK flag of completation  */

               jleptr2 = jleptr1;
               CMOVB(kcb.jlhchh,jleptr1);
               _Kccfree(&kcb.jlhchh,jleptr2);
               continue;                /* go to next entry             */
                                        /* free one entry of long word  */
            }                           /* endif(length == jle.len)     */
            else
            {                           /* if (length != jle.len)       */
               mce.jdok = JD_LONG;      /* set OK flag of completation  */
            }                           /* endelse(length == jle.len)   */
         }                              /* endif same input as long word*/

         else                           /* else                         */
         {                              /* free non-hitting JLE & it's  */
                                        /* JKJ                          */
            jkjptr1 = jle.jkjaddr;

            FOR_FWD_MID(kcb.jkhchh,jkjptr1,z_endflgk)
            {
               if ((jkj.kj[0] & 0x80) == 0x00 )
                  z_endflgk = ON;
                               /* set flag on if end of hyouki */
               jkjptr2  = jkjptr1;
                               /* get last jiritu hyouki ptr   */
               CMOVB(kcb.jkhchh,jkjptr1);
                               /* free jiritu-hyoki pointer    */
               _Kccfree(&kcb.jkhchh,jkjptr2);

            }

            jleptr2 = jleptr1;
            CMOVB(kcb.jlhchh,jleptr1);
            _Kccfree(&kcb.jlhchh,jleptr2);
                                        /* free one entry of long word  */
            continue;                   /* go to next entry             */
         }                              /* endelse not same word        */
      }                                 /* endif (jle.stap != z_stpos)  */
   }
   return( SUCCESS );
}
