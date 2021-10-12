static char sccsid[] = "@(#)18	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcbproc.c, libKJI, bos411, 9428A410j 7/23/92 02:52:25";
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
 * MODULE NAME:       _Kcbproc
 *
 * DESCRIPTIVE NAME:  CREATE BUNSETSU to CONNECT JTE and FTE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS)    : Success
 *                    0x1404(FTEOVER)    : FTE overflow
 *                    0x1704(BTEOVER)    : BTE overflow
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
#include   "_Kchin.t"                   /* Hinshi Conversion Table      */

/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short _Kcbproc(z_kcbptr,z_bstend,z_diccon)

struct  KCB  *z_kcbptr;
short        z_bstend;
uschar       z_diccon;
{
/*----------------------------------------------------------------------*  
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/ 
   extern struct RETCGET  _Kccget ();   /* Get  Any Kinds of Table Entry*/
   extern short           _Kccinsb();   /* Insert Before     Table Entry*/
   extern short           _Kccpurg();   /* Purge Table Entry            */
   extern struct RETBPATH _Kcbpath();   /* Set BTE on PTEs              */
   extern short           _Kcfjclc();   /* Get Penalty Between JTE-FTE  */
   extern struct RETBSBST _Kcbsbst();   /* Set Date in BTE              */
   extern short           _Kcbchkp();   /* Check J-F Conection          */
   extern short           _Kcjjpen();   /* Getting Penalty Between JTEs */
   extern void            _Kccfree();   /* Free Any Kinds of Table Entry*/

/*----------------------------------------------------------------------*  
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/ 
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcbte.h"   /* Bunsetsu Table Entry (BTE)                   */
#include   "_Kcfae.h"   /* Fuzokugo Attribute table Entry (FAE)         */
#include   "_Kcfte.h"   /* Fuzokugo Table Entry (FTE)                   */
#include   "_Kcfkx.h"   /* Fuzokugo Kj hyoki eXchange table entry (FKX) */
#include   "_Kcjte.h"   /* Jiritsugo Table Entry (JTE)                  */
#include   "_Kcjkj.h"   /* Jiritsugo KanJi hyoki table entry (JKJ)      */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */
#include   "_Kcgpw.h"   /* General Purpose Workarea (GPW)               */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/ 
#define Z_MASK     0x03

#define Z_MAX_PEN  0x7FFF               /* Max penalry                  */
#define Z_MAX_STP  0x7F                 /* Max start position           */
#define Z_NUM_HIN  50                   /* Number of kinds of hinsi     */

#define Z_HINR_DMY 127                  /* Number of kinds of hinsi     */

/*----------------------------------------------------------------------*  
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/ 
   struct RETCGET  z_rcget ;    /* Define Area for Return of _Kccget    */
   struct RETBSBST z_rbsbst;    /* Define Area for Return of _Kcbsbst   */
   struct RETBPATH z_rbpath;    /* Define Area for Return of _Kcbpath   */
   short           z_rcpurg;    /* Define Area for Return of _Kccpurg   */
   short           z_rcinsb;    /* Define Area for Return of _Kccinsb   */

   short           z_pen1;      /* return code for _Kcfjclc             */
   short           z_pen2;      /*                                      */
   short           z_jpen;      /* return code for _Kcjjpen             */
   short           z_kk;        /* return code for _Kcbchkp             */

   uschar          z_hinl;      /* leftward hinshi                      */
   uschar          z_hinr;      /* rightward hinshi                     */
   uschar          z_stap;      /* tempolary start position             */
   uschar          z_minstp;    /* fte minimum start position           */
   short           z_i;         /* loop counter                         */
   short           z_endflgj;   /* loop end flag for JTE                */
   short           z_endflgf;   /* loop end flag for FTE                */
   short           z_endflgk;   /* loop end flag for JKJ                */

   struct   {                   /* minimum penalry table                */
               short        pen;        /* min. penalty                 */
               struct PTE   *pteptr;    /* PTE pointer which has min.pen*/
            }   z_minpen[Z_NUM_HIN];

/*----------------------------------------------------------------------*  
 *      START OF PROCESS
 *----------------------------------------------------------------------*/ 
   /*-------------------------------------------------------------------*
    *     INITIALIZE MINIMAM PENALTY TABLE ( z_minpen[i] )
    *         i: codes of the right charcter
    *-------------------------------------------------------------------*/
   for ( z_i = 0; z_i < Z_NUM_HIN; z_i++ )
   {
      z_minpen[z_i].pen = Z_MAX_PEN;
      z_minpen[z_i].pteptr = NULL;
   }
   z_minstp = Z_MAX_STP;
   /*-------------------------------------------------------------------*
    *     SET BASE ADDRES OF KCB & GPW
    *-------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* establish address    to KCB  */
   gpwptr1 = kcb.gpwgpe;                /* establish address    to GPW  */

   /*-------------------------------------------------------------------*
    *     INITIALIZE RETURN CODE
    *-------------------------------------------------------------------*/

/*----------------------------------------------------------------------*  
 *      LOOK INTO PHRASE DEFINITION
 *----------------------------------------------------------------------*/ 
   /*-------------------------------------------------------------------*
    *     A phrase(Bunsetsu ) doesn't end
    *           before NN, xtsu, WO, -,
    *                  xa, xi, xu, xe, xo, xya, xyu, xyo, xka & xke.
    *     Because next phrase isn't able to begin these letters.
    *-------------------------------------------------------------------*/
   if( z_bstend < kcb.mchacmce-1)       /* mora doesn't end at the last */
   {
      mceptr1 = kcb.mchmce + z_bstend + 1;
      if (mce.code == M_NN)
      {
         return( SUCCESS );
      }
      else if (( mce.code == M_XTSU ) ||
	       ( mce.code == M_WO   ) ||
	       ( mce.code == 0x74   ) ||
	       ( mce.code == 0x7A   ) ||
	       ( mce.code == M_VU   ) ||
	       ( mce.code == M_XA   ) ||
	       ( mce.code == M_XI   ) ||
	       ( mce.code == M_XU   ) ||
	       ( mce.code == M_XE   ) ||
	       ( mce.code == M_XO   ))
      {
         if (mce.code != M_VU)
         {
            return( SUCCESS );
         }
      }
   }

/*----------------------------------------------------------------------*
 *      When single-phrase(TANBUNSETSU) conversion performed,
 *           phrases don't have adjective words exept last phrase.
 *----------------------------------------------------------------------*/
   if( ( kcb.mode == MODTAN ) && ( z_bstend != kcb.mchacmce-1 ) )
   {
      z_rcpurg = _Kccpurg(&kcb.fthchh); /* reset no of fuzokugo         */

   }

/*----------------------------------------------------------------------*
 *      When compound-word(FUKUGOU-GO) conversion performed,
 *           phrases don't have adjective words.
 *----------------------------------------------------------------------*/
   else if( kcb.mode == MODFUKU )
   {
      z_rcpurg = _Kccpurg(&kcb.fthchh); /* reset no of fuzokugo         */
   }

/*----------------------------------------------------------------------*
 *   MAKE DUMMY FTE for BTEs WHICH DON'T HAVE ADJECTIVE WORD
 *----------------------------------------------------------------------*/
   /*-------------------------------------------------------------------*
    *     GET ONE ENTRY FOR DUMMY FTE
    *-------------------------------------------------------------------*/
   z_rcget = _Kccget(&kcb.fthchh);      /* obtain a FTE entry           */

   if ( z_rcget.rc == GET_EMPTY )
      return( FTEOVER );
   else if ( ( z_rcget.rc != GET_TOP_MID ) && ( z_rcget.rc != GET_LAST ) )
      return( z_rcget.rc );

   /*-------------------------------------------------------------------*
    *     INSERT THE ENTRY TOP OF THE FTE ACTIVE CHAIN
    *-------------------------------------------------------------------*/
   fteptr1 =(struct FTE *)z_rcget.cheptr;/* establish address  to FTE   */

   z_rcinsb = _Kccinsb(&kcb.fthchh,fteptr1,(struct FTE *)NULL);
                                        /* insert at the top of FTE     */
   if ( z_rcinsb != SUCCESS )
      return( z_rcinsb );

   /*-------------------------------------------------------------------*
    *     SET PARAMETERS OF DUMMY FTE
    *-------------------------------------------------------------------*/
   fte.stap     = z_bstend +1;          /* mora pos                     */
   fte.setsu[0] = 0x00;         /* link information                     */
   fte.setsu[1] = 0x38;         /* Ichidan_taigen,Ichidan,Sahen_meishi  */
   fte.setsu[2] = 0x3F;         /* Meishi , Rentai , Fukushi,Setsuzoku  */
   fte.hinl     = NULL;                 /* hinshi left                  */
   fte.hinr     = Z_HINR_DMY;           /* hinshi right                 */
   fte.fhtx     = -1;                   /* no FKJ ptr                   */
   fte.pen      = 0;                    /* penalty                      */

/*----------------------------------------------------------------------*
 *      CHECK AND FREE UNABILABLE FTE
 *----------------------------------------------------------------------*/
   /*-------------------------------------------------------------------*
    *     LOOP OF FTE
    *-------------------------------------------------------------------*/
   FOR_FWD(kcb.fthchh,fteptr1,z_endflgf)
   {
      LST_FWD(fteptr1,z_endflgf);

      if (z_minstp > fte.stap)          /* for performance              */
         z_minstp = fte.stap;
      /*----------------------------------------------------------------*
       *     IF FTE HAS MORE THAN 2 KANJI HYOKI
       *                        THE FTE ISN'T CORRECT.
       *----------------------------------------------------------------*/
      if( fte.fhtx >= 0 )               /* if kanji-hyoki exists        */
      {
         fkxptr1 = (struct FKX *)(kcb.fkxfkx + fte.fhtx);

         if( fkx.flag & FKXCONT )       /* follow one kanji-hyoki       */
         {
            if( (*(fkxptr1 + 1)).flag & FKXCONT )
                                        /* follow two kanji-hyoki       */
            {
               fteptr2 = fteptr1;
               CMOVB(kcb.fthchh,fteptr1);
               _Kccfree(&kcb.fthchh,fteptr2);
               continue;
            }
         }
      }

      /*----------------------------------------------------------------*
       *     THE FTE CAN CREATE BTE BY ITSELF
       *----------------------------------------------------------------*/
      if ( ( (fte.setsu[2] & Z_MASK) == Z_MASK ) &&
                                        /* the 24th bit on              */
                ( fte.stap != z_bstend + 1 ) )
                                        /* not dummy fte                */
      {
  /*     z_pen1 = _Kcfjclc(z_kcbptr,(short)25,(struct JTE *)NULL,fteptr1);*/
                                        /* obtain penalty for connection*/
         z_pen1 = fte.pen + 0     ;     /*                              */
         z_hinr = hn_hinr[fte.hinr];
         faeptr1 = kcb.faefae + fte.hinl;
         if (fae.hin <= 22 )
            z_hinl = hn_hinl[fae.hin];
         else
            z_hinl = UNDEF;             /* fukushi 22 decimal           */
         z_rbsbst = _Kcbsbst(z_kcbptr,
                           z_hinl,      /* left  hinsi                  */
                           z_hinr,      /* right hinsi                  */
                           fte.stap,    /* mora start position          */
                           z_bstend,    /* mora end   position          */
                   (struct  JTE  *)NULL,/* JTE pointer                  */
                           z_pen1,      /* calculated pen               */
                           fteptr1);    /* FTE pointer                  */

         if ( z_rbsbst.rc != SUCCESS )
            return(z_rbsbst.rc);
                                        /* save BTE pointer in GPW      */
         gpw.lastbte = (struct BTE *)z_rbsbst.bteptr;

         if( kcb.env == ENVNONE )       /* if Initial conversion        */
         {                              /* then create path             */
            z_rbpath = _Kcbpath(kcbptr1,z_rbsbst.bteptr,z_bstend,z_minpen);

            if ( z_rbpath.rc != SUCCESS )
               return(z_rbpath.rc);
         }
      }
   }

/*----------------------------------------------------------------------*
 *      PROCESS FUZOKUGO ENTRY UNTIL THE LAST
 *----------------------------------------------------------------------*/
   /*-------------------------------------------------------------------*
    *     INITIALIZE
    *-------------------------------------------------------------------*/
   CMOVT(kcb.jthchh,jteptr1);
   if (jteptr1 == NULL )                /* if JTE does not exist        */
      return( SUCCESS );


   /*-------------------------------------------------------------------*
    *     LOOP OF JTE
    *-------------------------------------------------------------------*/
   FOR_FWD(kcb.jthchh,jteptr1,z_endflgj)
   {
      LST_FWD(jteptr1,z_endflgj);
      /*----------------------------------------------------------------*
       *     FREE JTE FOR PERFORMANCE
       *       if JTE end position is shorter as 7 mora than bte end pos
       *       and the JTE is not used then delete JTE and corresponding
       *       JKJ.
       *----------------------------------------------------------------*/
      if (((short)(jte.stap + jte.len) < z_bstend - 7)&&
          (jte.usage == 0)&&
          (kcb.env == ENVNONE))
      {
         if ( jte.jkjaddr != 0 )
         {
            jkjptr1 = jte.jkjaddr;

            FOR_FWD_MID(kcb.jkhchh,jkjptr1,z_endflgk)
            {
               if ((jkj.kj[0] & 0x80) == 0x00 )
                  z_endflgk = ON;       /* set flag on if end of hyouki */
               jkjptr2  = jkjptr1;
               CMOVB(kcb.jkhchh,jkjptr1);/* get last jiritu hyouki ptr   */
                                        /* free jiritu-hyoki pointer    */
               _Kccfree(&kcb.jkhchh,jkjptr2);

            }
         }

         jteptr2 = jteptr1;             /* free unused JTE              */
         CMOVB(kcb.jthchh,jteptr1);
         _Kccfree(&kcb.jthchh,jteptr2);

         continue;
      }
      if (jte.stap + jte.len < z_minstp)/* for performace               */
         continue;

      /*----------------------------------------------------------------*
       *     LOOP OF FTE
       *----------------------------------------------------------------*/
      FOR_FWD(kcb.fthchh,fteptr1,z_endflgf)
      {
         LST_FWD(fteptr1,z_endflgf);
         /*-------------------------------------------------------------*
          *     MAKE BTE, WHEN THE END OF JIRITSU-GO
          *             AND START OF ADJCTIVE WORD CAN CONNECT
          *-------------------------------------------------------------*/
         if( ( jteptr1 != NULL ) 
                        && ( (jte.stap + jte.len ) == (fte.stap) ) )
                                        /* if no gap between jiritsugo  */
                                        /* and fuzokugo                 */
         {
            /*----------------------------------------------------------*
             *   DON'T CONNECT FTE WHICH HAS KANJI-HYOUKI
             *       WHEN ENVIRONMENT IS NEXTCNV OR ZENCONV
             *----------------------------------------------------------*/
            if ( ( jte.len != 0 ) && ( fte.fhtx >= 0 ) 
                &&  ( ( kcb.env == ENVNEXT ) || ( kcb.env == ENVZEN ) ) )
               continue;

            /*----------------------------------------------------------*
             *   CHEK HINSHI OF JTE AND HINSHI OF FTE CAN CONNECT
             *----------------------------------------------------------*/
            z_kk = _Kcbchkp(kcbptr1,jte.hinpos, fte.setsu);

            if( z_kk > 0 )
            {
               z_hinl = tp_hinl[jte.dtype];
               if ( z_hinl >= Z_NUM_HIN)
                  z_hinl = hn_hinl[z_kk];
            }
            else                        /* they can't connect           */
               continue;

            /*----------------------------------------------------------*
             *   WHEN BTE COMPOSED BY ONLY JIRITSU-GO ( FTE IS DUMMY )
             *          HINR IS ON THE TYP_HINR TABLE BUT IPPAN_GO
             *      WHEN IPPAN_GO HINR IS ON THE HIN_HINR TABLE
             *   WHEN BTE COMPOSED BY JIRITU-GO AND ADJECTIVE WORD
             *          HINR IS ON THE HIN_HINR TABLE
             *----------------------------------------------------------*/
            if ( fte.hinr == Z_HINR_DMY )/* if FTE is dummy            */
            {
               z_hinr = tp_hinr[jte.dtype];
               if ( z_hinr >= Z_NUM_HIN)/* get right hinshi from JTE    */
                   z_hinr = hn_hinr[z_kk];
            }
            else                        /* get right hinshi from FTE    */
               z_hinr = hn_hinr[fte.hinr];
            /*----------------------------------------------------------*
             *   CALICULATE PENALTY OF BTE
             *----------------------------------------------------------*/
            z_stap = jte.stap;          /*   jte.stap                   */
            z_jpen = _Kcjjpen(jteptr1,z_kk);
                                        /* jteptr                       */
                                        /* reurn code is pen            */
            if ( fte.hinl > NULL )
            {
               if ( z_kk <= 22 )
               {
                  z_pen1 = _Kcfjclc(kcbptr1,z_kk,jteptr1,fteptr1);
                                        /* return code conn pen         */
                  z_pen2 = z_jpen + fte.pen + z_pen1 ;
               }
               else
                  continue;
            }
            else
               z_pen2 = z_jpen + fte.pen ;

            /*----------------------------------------------------------*
             *   CHECK NEW BTE IS AVAILABLE BTE
             *----------------------------------------------------------*/
            if( ( CACTV( kcb.bthchh ) > 0) &&
                ( kcb.env == ENVNONE )       &&
                ( (*gpw.lastbte).hinl == z_hinl ) &&
                ( (*gpw.lastbte).hinr == z_hinr ) &&
                ( (*gpw.lastbte).stap == z_stap ) &&
                ( (*gpw.lastbte).endp == z_bstend ) &&
                ( (*gpw.lastbte).pen < z_pen2 ))
            {
               continue;                /* for jiritsugo                */
            }
            else
            {
               z_rbsbst = _Kcbsbst(z_kcbptr,
                       z_hinl,          /* left  hinsi                  */
                       z_hinr,          /* right hinsi                  */
                       z_stap,          /* mora start position          */
                       z_bstend,        /* mora end   position          */
                       jteptr1,         /* JTE pointer                  */
                       z_pen2,          /* calculated pen               */
                       fteptr1);        /* FTE pointer                  */

               if ( z_rbsbst.rc != SUCCESS )
                  return(z_rbsbst.rc);
                                        /* save BTE pointer in GPW      */
               gpw.lastbte = (struct BTE *)z_rbsbst.bteptr;
               jte.usage ++;
               if( kcb.env == ENVNONE )
               {
                                        /* Create New Path or Add path  */
                  z_rbpath = _Kcbpath(kcbptr1,z_rbsbst.bteptr,
                                      z_bstend,z_minpen);
                  if ( z_rbpath.rc != SUCCESS )
                     return(z_rbpath.rc);
               }
            }                           /* end else                     */
         }                              /* end if                       */
      }                                 /* end for fuzoku-go            */
   }                                    /* end for jiritsu-go           */

   /*-------------------------------------------------------------------*
    *     RETURN
    *-------------------------------------------------------------------*/
   return( SUCCESS );
}
