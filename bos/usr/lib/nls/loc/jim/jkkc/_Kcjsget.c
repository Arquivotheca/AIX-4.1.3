static char sccsid[] = "@(#)01	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjsget.c, libKJI, bos411, 9428A410j 7/23/92 03:14:26";
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
 * MODULE NAME:       _Kcjsget
 *
 * DESCRIPTIVE NAME:  PICKS UP TYPE HINPOS DFLAG FROM THE SPECIFIED OFFSET.
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS): success
 *                    0x1104 (JTEOVER): JTE table overflow
 *                    0x1204 (JKJOVER): JKJ table overflow
 *                    0x1304 (JLEOVER): JLE table overflow
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
#include   "_Kcrcb.h"                   /* Define Return Code Structure */


/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
short _Kcjsget(z_kcbptr,z_tbtype,z_strd,z_mode,
                        z_endd,z_strpos,z_length,z_strm,z_file)

struct KCB    *z_kcbptr;                /* pointer of KCB               */
short         z_tbtype;                 /* type of table                */
short         z_strd;                   /* offset of top of data buf    */
short         z_mode;                   /* create mode of table         */
short         z_endd;                   /* offset of last of data buf   */
unsigned char z_strpos;                 /* offset of 1st MCE            */
unsigned char z_length;                 /* length of MCE(yomi data)     */
short         z_strm;                   /* offset of mora code in buf   */
short         z_file;                   /* System File Number           */
{
/*----------------------------------------------------------------------*
 *      EXTERNAL REFERENCE
 *----------------------------------------------------------------------*/
   extern struct RETJSGT1 _Kcjsgt1();   /* Look up about One Entry      */

/*----------------------------------------------------------------------*
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"   /* CHain Header map (CHH)                       */
#include   "_Kckcb.h"   /* Kkc Control Block (KCB)                      */

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETJSGT1 z_rjsgt1;    /* Define Area for Return of _Kcjsgt1   */

/*----------------------------------------------------------------------*
*       SET JIRITSU-GO OR LONG WORD TABLE                               *
*-----------------------------------------------------------------------*/

   z_rjsgt1.newoff = z_strd;            /* set initialization offset    */

   while ( z_rjsgt1.newoff < z_endd ) { /* loop for same yomi entry     */
      z_rjsgt1                          /* for 1 entry                  */
         = _Kcjsgt1(z_kcbptr,z_tbtype,z_rjsgt1.newoff,z_mode,
                             z_strpos,z_length,z_strm,z_file);
      if ( z_rjsgt1.rc != SUCCESS )
         return(z_rjsgt1.rc); 
   }
        
   return( SUCCESS );
}
