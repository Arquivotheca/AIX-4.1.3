static char sccsid[] = "@(#)60	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcffdct.c, libKJI, bos411, 9428A410j 7/23/92 03:05:27";
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
 * MODULE NAME:       _Kcffdct
 *
 * DESCRIPTIVE NAME:  LOOK UP FUZOKU-GO TABLES & CONSTRACT FTE
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000(SUCCESS)    : Success
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
short _Kcffdct(z_kcbptr,z_morapos)

struct KCB      *z_kcbptr;              /* get address of KCB           */
short  z_morapos;                       /* specified mora position      */
{ 
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETFFCON _Kcffcon();   /* fuzoku-go construction       */
   extern struct RETFSTMP _Kcfstmp();   /* set fzk-go temporaly table   */
   extern short           _Kcfjcon();   /* check conection F & J        */
   extern short           _Kcfsfzk();   /* set fzk-go table             */
   extern short           _Kcfendm();   /* Check to be Last Mora Code   */

/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */
#include   "_Kcmce.h"   /* Mora Code table Entry (MCE)                  */
#include   "_Kcfae.h"   /* Fuzokugo Attribute table Entry (FAE)         */
#include   "_Kcfax.h"   /* Fuzokugo Attribute table indeX (FAX)         */
#include   "_Kcfwe.h"   /* Fuzokugo Work table Entry (FWE)              */
#include   "_Kcfry.h"   /* Fuzokugo Reversed Yomi table (FRY)           */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   short    z_i;                        /* define loop counter          */
   short    z_pen;                      /* define penalty               */
   short    z_fzkno;                    /* define fuzokugo number       */
   struct   FWE        *z_fwestp;       /* return of start of fweptr    */
   struct   FWE        *z_fweedp;       /* return of end   of fweptr    */
   uschar               z_endflg;       /* end flag                     */

   struct   RETFFCON    z_rffcon;       /* return of _Kcffcon           */
   struct   RETFSTMP    z_rfstmp;       /* return of _Kcfstmp           */
   short                z_rfjcon;       /* return of _Kcfjcon           */
   short                z_rfsfzk;       /* return of _Kcfsfzk           */

/*----------------------------------------------------------------------*
 *      START OF PROCESS
 *----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*  
 *      LOOK UP FUZOKU-GO DICTIONARY & CONSTRACT FTE
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set base address of KCB      */ 

/*----------------------------------------------------------------------*  
 *      FUZOKUGO DAI 1 SEDAI (bunsetsu-end)
 *----------------------------------------------------------------------*/
   mceptr1 = kcb.mchmce+z_morapos;      /* set address of MCE           */
   if( _Kcfendm(mce.code) == 0)         /* test mora code which is end  */
      return(SUCCESS);                  /* mora                         */
   faxptr1 = kcb.faxfax + _Kcfmtom(mce.code);  /* set address of FAX    */
   faxptr2 = faxptr1 + 1;               /* set address of FAX           */
   faeptr1 = kcb.faefae + fax.faendx;   /* set start address of FAE     */
   faeptr2 = kcb.faefae + fax2.faendx;  /* set end  address of FAE      */

   z_rfstmp=_Kcfstmp(z_kcbptr,(struct FWE *)NULL,(short)(z_morapos+1),
                     (short)0,(short)NULL,(short)0);
                                        /* make dummy fuzokugo          */
   if ( z_rfstmp.rc == FWEOVER )        /* error handling               */
      return(SUCCESS);                  /* return                       */
   else if ( z_rfstmp.rc != SUCCESS )   /* error handling               */
      return(z_rfstmp.rc);              /* return with fatal error code */

   z_fwestp = z_rfstmp.fweptr;          /* set start fwe pointer        */
   z_fweedp = z_rfstmp.fweptr;          /* set end   fwe pointer        */

   for(z_fzkno=fax.faendx;
       faeptr1 < faeptr2;z_fzkno++,faeptr1++) 
                                        /* loop all fuzokugo which has  */
                                        /* a mora of z_moarpos at last  */
                                        /* charcter                     */
   {                            /* z_fzkno:fuzokugo number of analizing */
      fryptr1 = kcb.fryfry + fae.frypos;/* set reverse yomi pointer     */

      for(z_i=0,mceptr2=mceptr1-1;z_i<(short)fae.len;
          mceptr2--,z_i++,fryptr1++)
      {                                 /* analize on backward mora     */
         if(fry.kana!=_Kcfmtom(mce2.code)) /* compare input yomi and FRY*/
         {
            z_i=(-1);                   /* set unmatching flag          */
            break;
         }
      }
      if (z_i >= 0)                     /* if it is fuzokugo            */
      {
         z_rffcon=_Kcffcon(z_kcbptr,z_fzkno,(short)0);
                                        /* fzkgo & dummy conection      */
         if(z_rffcon.rc >= 0)           /* if it has penalty            */
         { 
            z_pen = z_rffcon.rc + fae.pen;
                                        /* add own PENALTY to pen       */ 
            z_rfstmp =_Kcfstmp(z_kcbptr,z_fwestp,z_morapos-z_i,
                               z_fzkno,fae.fkjpos,z_pen); 
                                        /* set fzkgo temporary table    */
            if ( z_rfstmp.rc == FWEOVER )/* error handling              */
               return(SUCCESS);         /* return                       */
            else if ( z_rfstmp.rc != SUCCESS )/* error handling         */
               return(z_rfstmp.rc);     /* return with fatal error code */

            z_fweedp = z_rfstmp.fweptr; /* set end fwe pointer          */

            z_rfjcon = _Kcfjcon(z_kcbptr,z_fzkno);
                                        /* check possibility of conec- */
                                        /* tion                        */
            if (z_rfjcon == YES)        /* if conect to jiritsugo      */
            {
               z_rfsfzk=_Kcfsfzk(z_kcbptr,z_rfstmp.fweptr); 
               if ( ( z_rfsfzk == FTEOVER ) || ( z_rfsfzk == FKXOVER ) )
                  return(SUCCESS);      /*    return normal            */
               else if ( z_rfsfzk != SUCCESS )
                  return(z_rfsfzk);     /*return with fatal error code  */
            }
         }                              /* endif it has penalty        */
      }                                 /* endif of fuzokugo found     */
   }

/*----------------------------------------------------------------------*  
 *       FUZOKUGO AFTER 2ND GENERATION
 *----------------------------------------------------------------------*/
   while(z_fwestp != z_fweedp)          /* test new generation is made  */
   { 
      fweptr1 = z_fwestp;               /* set start FWE pointer        */
      z_fwestp=z_fweedp;                /* set new start FWE pointer    */

      CMOVF(kcb.fwhchh,fweptr1);        /* loop of all parent           */
      FOR_FWD_MID(kcb.fwhchh,fweptr1,z_endflg)
      { 
         if(z_fwestp == fweptr1)
            z_endflg = ON;
                                        /* set FWE pointer              */
         mceptr1 = kcb.mchmce + fwe.stap-1;
                                        /* set address of MCE           */
         faxptr1 = kcb.faxfax + _Kcfmtom (mce.code );
                                        /* set start address of FAX     */
         faxptr2 = faxptr1 + 1;
                                        /* set last address of  FAX     */
         faeptr1 = kcb.faefae + fax.faendx;
                                        /* set start address of FAE     */
         faeptr2 = kcb.faefae + fax2.faendx;
                                        /* set end  address of FAE      */

         for(z_fzkno=fax.faendx;
             faeptr1 < faeptr2;z_fzkno++,faeptr1++) 
                                        /* loop all fuzokugo which has  */
                                        /* a mora of z_moarpos at last  */
                                        /* charcter                     */
         {
            fryptr1 = kcb.fryfry + fae.frypos;
            for(z_i=0,mceptr2=mceptr1-1;z_i<fae.len;
                mceptr2--,z_i++,fryptr1++)
            {                           /* analize on backward mora     */
               if(fry.kana != _Kcfmtom(mce2.code)) /* com yomi and FRY  */
               {
                  z_i=(-1);             /* set unmatching flag          */
                  break;
               }
            }
            if (z_i >= 0)               /* if fuzokugo was found        */
            {
               z_rffcon=_Kcffcon(z_kcbptr,z_fzkno,fwe.fno);
                                        /* fzkgo & fzkgo conection      */
               if(z_rffcon.rc >= 0 )    /* if it has penalty            */
               { 
                  z_pen = z_rffcon.rc + fae.pen; /* + fwe.pen;          */
                                        /* add penalty                  */ 

                  z_rfstmp = _Kcfstmp(z_kcbptr,fweptr1,fwe.stap-fae.len-1,
                                z_fzkno,fae.fkjpos,z_pen); 
                                        /* set fzkgo temporary table    */
                  if ( z_rfstmp.rc == FWEOVER )
                     return(SUCCESS);   /* return                       */
                  else if ( z_rfstmp.rc != SUCCESS )
                     return(z_rfstmp.rc);
                                       /* return with fatal error code */
                  z_fweedp = z_rfstmp.fweptr;
                                        /* set end fwe pointer          */

                  z_rfjcon=_Kcfjcon(z_kcbptr,z_fzkno);
                                        /* check possibility of conec- */
                                        /* tion                        */
                  if (z_rfjcon == YES)  /* if conect to jiritsugo      */
                  {
                     z_rfsfzk=_Kcfsfzk(z_kcbptr,z_rfstmp.fweptr); 
                                        /* set one of  FTE entry       */
                     if ( z_rfsfzk == FTEOVER )
                        return(SUCCESS);/* return normal            */
                     else if ( z_rfsfzk != SUCCESS )
                        return(z_rfsfzk);/*return with fatal error code*/

                  }                     /* endif of conection with jrtsu*/
               }                        /* endif of conection with parent*/
            }                           /* endif of fuzokugo found      */
         }                              /* end of for loop fzk no.      */
      }                                 /* end of for loop of all parent*/
   }                                    /* end of while                 */
   return(SUCCESS);                     /* return with number of fzk.   */
}                                       /* end of program               */
