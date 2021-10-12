static char sccsid[] = "@(#)10	1.2 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcbaddp.c, libKJI, bos411, 9428A410j 6/4/91 10:10:16";
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
 * MODULE NAME:       _Kcbaddp
 *
 * DESCRIPTIVE NAME:  ADD NEW BTE AFTER ALREADY EXISTED PATH
 *
 * FUNCTION:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS)    : Success
 *                    0x1804(PTEOVER)    : PTE overflow
 *                    0x7FFF(UERROR )    : Fatal error
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
short _Kcbaddp(z_kcbptr,z_bteptr,z_minp)

struct KCB   *z_kcbptr;                 /* pointer of KCB               */
struct BTE   *z_bteptr;                 /* pointer of BTE               */
struct {
          short        pen;
          struct PTE   *pteptr;
       }  z_minp[];                     /* buffer of minimum penalties  */
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern struct RETCGET  _Kccget();    /* Get  Any Kinds of Table Entry*/
   extern struct RETCSWPP _Kccswpp();   /* Deleta Disused PTE           */
   extern short           _Kcbbpen();   /* Culculate Penalty BTE to BTE */
   extern void            _Kccfree();   /* Free Any Kinds of Table Entry*/

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcjte.h"   /* Jiritsugo Table Entry (JTE)                  */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */
#include   "_Kcpte.h"   /* Path Table Entry (PTE)                       */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */
#include   "_Kcyce.h"   /* Yomi Code table Entry (YCE)                  */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_MAX_PEN    255

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   struct RETCGET  z_rcget ;    /* Define Area for Return of _Kccget    */
   struct RETCSWPP z_rcswpp;    /* Define Area for Return of _Kccswpp   */
   short            z_i;                /* counter                      */
   short            z_i1;               /* counter 1                    */
   short            z_i2;               /* counter 2                    */
   short            z_trapen;           /* penalty of connection        */
   short            z_pen;              /* penalty of path              */
   short            z_stap;             /* start pos of yomi            */
   short            z_endp;             /* end pos of yomi              */
   short            z_minpen;           /* minimam penalty              */
   short            z_pos;              /* hinshi                       */
   short            z_endflg;           /* end flag                     */
   short            z_act;              /* active flag                  */

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   kcbptr1 = z_kcbptr;                  /* set base pointer of KCB      */
   gpwptr1 = kcb.gpwgpe;                /* set base pointer of GPW      */
   bteptr2 = z_bteptr;                  /* set base pointer of BTE      */

/*----------------------------------------------------------------------*  
 *      RETURN IF INCONSISTENCY OCCURS ABOUT CUT POINT
 *----------------------------------------------------------------------*/ 
   /*-------  get start-position & end-position of yomi-code  ----------*/
   if ( bte2.stap == 0 )
      z_stap = 0;
   else
   {
      mceptr1 = kcb.mchmce + bte2.stap - 1;
      z_stap = ( mce.yceaddr + 1 ) - kcb.ychyce;
   }

   if(bte2.endp >= 0)
   {
      mceptr1 = kcb.mchmce + bte2.endp;
      z_endp = mce.yceaddr - kcb.ychyce;
   }
   else
      z_endp = 0;
   
   /*----------  if the BTE cross from before yomill2 to after yomill2,
                        return NORMAL  ---------------------------------*/
   if (  ( z_stap + 1 <= kcb.ymill2 - gpw.accfirm) &&
         ( z_endp + 1 >  kcb.ymill2 - gpw.accfirm)&&
         ( kcb.mchacmce != bte2.endp )  )
      return((short)SUCCESS);

   /*----------  if 1st phrase is specifid then penalty
                                 of them decrease down   ---------------*/
   if ((kcb.ymill2 != 0 ) &&            /* It is recuting operation     */
       (z_endp + gpw.accfirm + 1 <= kcb.ymill2))
                                        /* It is in recuting area       */
   {
      if(bte2.jteaddr != NULL)          /* if Jiritsugo is exists       */
         bte2.pen += 15;                /* add penalty                  */
      if(bte2.fzkflg != F_FLG_NOEXT)    /* if Fuzokugo homonym exists   */
         bte2.pen += 15;                /* add penalty                  */
      if(bte2.fzkflg & F_FLG_TWO)       /* if two homonym exits         */
         bte2.pen += 15;                /* add penalty                  */
   }
   /*----------  if any PTEs don't exist, return NORMAL  ---------------*/

   if ( CACTV( kcb.pthchh ) == 0 )
      return((short)SUCCESS);

/*----------------------------------------------------------------------*
 *      SEARCH PTE WHICH HAS MINIMAM PENALTY
 *----------------------------------------------------------------------*/
   /*-----------------  get the pointer of first PTE  ------------------*/
   CMOVT(kcb.pthchh,pteptr1);

   if ( pteptr1 == NULL  )
      return((short)UERROR);

   /*------------  initialize for searching minimum penalty  -----------*/
   pteptr2 = pteptr1;                      /* pteptr2 is minmam           */
   z_minpen = 0x7fff;

   for ( z_i2 = 0; z_i2 < 5 ; z_i2++ )
      if ( pte2.bteaddr[z_i2 + 1] == NULL )
         break;

   /*-----------------------   CHECK ALL PTEs   ------------------------*/
   FOR_FWD(kcb.pthchh,pteptr1,z_endflg)
   {
      LST_FWD(pteptr1,z_endflg);
      /*----------   IF END POS OF MORA IN PATS IS
                         BEFORE END OF MORA IN BUNSETU   ---------------*/
      if (pte.endp == bte2.stap -1 )
      {
         /*---------------  search last BTE in PTE  --------------------*/
         for ( z_i = 0; z_i < 6 ; z_i++ )
            if ( pte.bteaddr[z_i] == NULL )
               break;
                                        /* set base pointer of BTE      */
         bteptr1 = pte.bteaddr[z_i-(short)1];

         /*-------------  caliculate penalty of the PTE  ---------------*/
                                        /* get penalty of connection    */
         z_trapen = _Kcbbpen(kcbptr1,bteptr1,bteptr2);

                                        /* calc total penalty of path   */
         z_pen = pte.pen + bte2.pen + z_trapen;

         /*---  IF PENALTY OF PATH IS SMALLER THE PENALTY THRESHOLD  ---*/
         if ( ( (z_pen >= 0) && (z_pen <= Z_MAX_PEN) )  && ( z_i < 6 ) )
         {
            if(z_pen < z_minpen)        /* if the minimam penalty path  */
            {
               z_minpen = z_pen;
               pteptr2 = pteptr1;
               for ( z_i2 = 0; z_i2 < 5 ; z_i2++ )
                  if ( pte.bteaddr[z_i2+1] == NULL )
                     break;
            }

            else if(z_pen == z_minpen)       /* if the same panalty as min.  */
            {
               for ( z_i1 = 0; z_i1 < 5 ; z_i1++ )
                  if ( pte.bteaddr[z_i1+1] == NULL )
                     break;
               jteptr2 = (*pte.bteaddr[z_i1]).jteaddr;
               jteptr1 = (*pte2.bteaddr[z_i2]).jteaddr;
               if((*pte2.bteaddr[z_i2]).stap==(*pte.bteaddr[z_i1]).stap)
               {                         /* J1&J2 has same start pos.   */
                  if ( jteptr1 != NULL )
                  {                      /* J1 is not dummy             */
                     if((jte.dflag[0] & 0x06)!=0x06)
                     {                   /* J1 is not MRU word          */
                        if(jteptr2 == NULL)
                        {                /* J2 is dmy                   */
                           pteptr2 = pteptr1;
                           z_i2 = z_i1;
                        }
                        else
                        {
                           if(jte.len < jte2.len)
                           {            /* J2 is longer than J1         */
                              pteptr2 = pteptr1;
                              z_i2 = z_i1;
                           }
                        }               /* end else                     */
                     }
                  }
               }
            }
         }
      }
   }

/*----------------------------------------------------------------------*
 *      SET NEXT PTE
 *----------------------------------------------------------------------*/
   pteptr1 = pteptr2;
   if (pte.endp == bte2.stap -1 )
   {
   /*----------   IF END POS OF MORA IN PATS IS
                         BEFORE END OF MORA IN BUNSETU   ---------------*/
                                        /* set base pointer of BTE      */
      bteptr1 = pte.bteaddr[z_i2];

                                        /* get penalty of connection    */
      z_trapen = _Kcbbpen(kcbptr1,bteptr1,bteptr2);

                                        /* calc total penalty of path   */
      z_pen = pte.pen + (short)bte2.pen + (short)z_trapen;

      z_pos = bte2.hinr;

      z_act = CSTTS( z_minp[z_pos].pteptr );

      if ( ( z_pen <= ( z_minp[z_pos].pen + 3 ) )/* new pte less than   */
             || ( z_act != ON ) )       /* old minimam pte              */
      {
      /*----------------------------------------------------------------*
       *        TEST NESSECITY OF THE PTE
       *----------------------------------------------------------------*/
         if ( ( z_minp[z_pos].pteptr != NULL )
           && ( z_act == ON )
           && ( ( z_minp[z_pos].pen - 3 ) > z_pen ) )
         {
            pteptr2 = z_minp[z_pos].pteptr;
            for ( z_i = 0; z_i < 6; z_i++ )
            {                           /* about all BTE included a PTE */
               if ( pte2.bteaddr[z_i] == (struct BTE *)NULL )
                  break;
               bteptr1 = pte2.bteaddr[z_i];
               bte.usage--;             /* decreaze BTE usage count     */
               if((bte.usage == 0)&&(bte.jteaddr!=NULL))
               {                        /* if BTE usage is zero         */
                  jteptr1 = bte.jteaddr;
                  jte.usage--;          /* decreaze JTE usage count     */
                  _Kccfree(&kcb.bthchh,bteptr1);
               }                        /* delete BTE                   */
            }
            _Kccfree(&kcb.pthchh,z_minp[z_pos].pteptr);
         }                              /* delete PTE                   */

         z_rcget = _Kccget(&kcb.pthchh);  /* get NEW PTE                */
         if ( z_rcget.rc == GET_EMPTY )
         {
            z_rcswpp = _Kccswpp(kcbptr1,pteptr1);
            if (z_rcswpp.num == 0)
               return( PTEOVER );
            if(pteptr1 != z_rcswpp.pteptr)
               return( SUCCESS );
            z_rcget = _Kccget(&kcb.pthchh);  /* get NEW PTE          */
         }
         else if ( ( z_rcget.rc != GET_TOP_MID ) && ( z_rcget.rc != GET_LAST ) )
            return( UERROR );

         pteptr2                        /* set base pointer of new PTE  */
            = (struct PTE *)z_rcget.cheptr;

         if ( ( z_pen < z_minp[z_pos].pen ) || ( z_act != ON ) )
         {
            z_minp[z_pos].pteptr = pteptr2;
            z_minp[z_pos].pen = z_pen;
         }

         for ( z_i = 0; ; z_i++ )
         {
            if ( pte.bteaddr[z_i] == NULL )
               break;
                                        /* set bunsetu node             */
            pte2.bteaddr[z_i] = pte.bteaddr[z_i];
            bteptr1 = pte.bteaddr[z_i];

            bte.usage++;                /* inc usage counter            */
         }

         pte2.bteaddr[z_i] = bteptr2;   /* set bunsetu node             */
         bte2.usage++;                  /* inc usage counter            */
         pte2.endp = bte2.endp;         /* set end pos of path          */
         pte2.pen = z_pen;              /* set total penalty            */

         if(z_i == 5)                   /* if number of BTE is beyond 5 */
            gpw.kakuflg = ON;           /* kakutai flag ON              */
         for ( z_i++; z_i < 6; z_i++ )  /* initialize remained BTE addr.*/
            pte2.bteaddr[z_i] = NULL;
      }
   }
   else
   {                                    /* free unused BTE              */
      if(bte2.jteaddr != NULL)
         (*bte2.jteaddr).usage--;
      _Kccfree(&kcb.bthchh,bteptr2);
   }

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return( SUCCESS );
}
