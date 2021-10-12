static char sccsid[] = "@(#)15	1.3 src/bos/usr/lib/nls/loc/jim/jkkc/_Kcjuhsh.c, libKJI, bos411, 9428A410j 6/8/94 23:38:47";
/*
 * COMPONENT_NAME: (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS: Kana-Kanji-Conversion (KKC) Library
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************** START OF MODULE SPECIFICATIONS ********************
 *
 * MODULE NAME:       _Kcjuhsh
 *
 * DESCRIPTIVE NAME:
 *
 * INPUT:
 *
 * OUTPUT:
 *
 * RETURN CODE:       0x0000 (SUCCESS):   success
 *                    0x08ff (Z_NOTFND): no candidate exists in user dict
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

#define    YOMI_CODE		0x00
#define    NOT_YOMI_CODE	0x01
#define    NOT_ESC_ALPHA	ESC_ALPHA+1

static  uschar  yomi2pri[256] = {
/*      0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
/*0*/ 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
/*1*/ 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,
/*2*/ 0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
/*3*/ 0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
/*4*/ 0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
/*5*/ 0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
/*6*/ 0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
/*7*/ 0x90,0x91,0x00,0x00,0x00,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x00,

/*8*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*9*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*a*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*b*/ 0x00,0x00,0x00,0x00,0x00,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,
/*c*/ 0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x00,
/*d*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*e*/ 0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,
/*f*/ 0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x00,0x00,0x00,0x00,0x00,0x00
};

/************************************************************************
 *      START OF FUNCTION
 ************************************************************************/
struct RETJUHSH _Kcjuhsh(z_kcbptr,z_strpos,z_length)
struct KCB      *z_kcbptr;              /* pointer of KCB               */
unsigned char   z_strpos;               /* offset of 1st MCE            */
unsigned char   z_length;               /* length of MCE(yomi data)     */
{
/*----------------------------------------------------------------------* 
 *      INCLUDE FILES
 *----------------------------------------------------------------------*/
#include   "_Kcchh.h"                   /* CHain Header map (CHH)       */
#include   "_Kckcb.h"                   /* Kkc Control Block (KCB)      */
#include   "_Kcmce.h"
#include   "_Kcyce.h"
#include   "_Kcuxe.h"

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL CONSTANTS
 *----------------------------------------------------------------------*/
#define   Z_NOTFND   0x08ff

/*----------------------------------------------------------------------*
 *      DEFINITION OF LOCAL VARIABLES
 *----------------------------------------------------------------------*/
   struct RETJUHSH   z_ret;             /* return code save area        */
   short             z_i;               /* counter                      */
   short             z_j;               /* counter                      */
   short             z_k;               /* counter                      */
   short             z_pos;             /* yomi position                */
   short             z_yl;              /* length of yomi               */
   short             z_dl;              /* length of yomi               */
   short             z_il;              /* length of yomi               */
   uschar            *z_ds;

/*----------------------------------------------------------------------*
 *      SET BASE POINTERS
 *----------------------------------------------------------------------*/
   kcbptr1 = z_kcbptr;                  /* set pointer of KCB           */
   uxeptr1 = kcb.uxeuxe;                /* set pointer of UXE           */

/*----------------------------------------------------------------------*
 *       SERACH INDEX TO RRN (Rerative Recoed Number) 
 *----------------------------------------------------------------------*/
   z_il = uxe.il[1] * 256 + uxe.il[0];
   z_k = 0;
   z_pos = 0;

   for ( z_ds = &uxe.dptr; z_ds < ((uschar *)uxeptr1 + z_il); z_ds += z_yl ) {
      z_yl = (short)*z_ds++;
      if ( z_yl == 0 )
         break;

      /*---------------   compare mora code with index   ---------------*/
      if ( z_strpos != 0 ) {            /* if 1st MCE is the top of tb  */
         mceptr1 = kcb.mchmce + z_strpos - 1;
         yceptr1 = mce.yceaddr + 1;             
      }
      else                              /* if 1st MCE is middle entry   */
         yceptr1 = kcb.ychyce;

      mceptr2 = kcb.mchmce + z_strpos + z_length - 1;

      z_dl = mce2.yceaddr - yceptr1 + 1;

      for ( z_i = 0; ( z_i < ( z_yl - 1 ) ) && ( z_i < z_dl ); z_i++ ) {
         yceptr2 = yceptr1 + z_i;
         if ( (uschar)ds2pri( z_ds + z_i ) != yomi2pri[yce2.yomi])
               break;
      }
      if ( ( z_i >= z_pos ) || 
         ( (uschar)ds2pri( z_ds + z_i ) >= yomi2pri[yce2.yomi])) {
          if ( z_k >4 ) {
              for(z_k=0;z_k < 3 ; z_k++) {
                  z_ret.rrn[ z_k ] = z_ret.rrn[ z_k + 1 ];
              }
	  }
          z_ret.rrn[z_k++] = *( z_ds + z_yl - 1 );
          z_pos++;
      }
      if ( (uschar)ds2pri( z_ds + z_i ) > yomi2pri[yce2.yomi] )
              break;
   }

   if ( z_k < 5 )
      z_ret.rrn[z_k] = -1;
   if ( z_ret.rrn[0] == -1 )
      z_ret.rc = (short)Z_NOTFND;
   else
      z_ret.rc = (short)SUCCESS;

/*----------------------------------------------------------------------*
 *      RETURN
 *----------------------------------------------------------------------*/
   return(z_ret);
}

static  uschar  bit2pri[128] = {
/*      0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
/*0*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*1*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*2*/ 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
/*3*/ 0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x10,0x11,0x12,0x13,0x14,0x15,
/*4*/ 0x16,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
/*5*/ 0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x00,0x00,0x00,0x00,0x00,
/*6*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*7*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

/*************************************************************************
 *          Pri Code         Yomi Code       ds_code
 * Kigou   0x00 - 0x16      0x01 - 0x16     0x21 - 0x2F, 0x3a - 0x40
 * Number  0x17 - 0x20      0x75 - 0x7e     0x30 - 0x39
 * Alph(a) 0x21 - 0x3a      0xb5 - 0xce     0x41 - 0x5a
 * Alph(A) 0x21 - 0x3a      0xe0 - 0xf9     0x41 - 0x5a
 * Kanji   0x3f - 0x9f      0x1f - 0x71     0x1f - 0x71
 *
 *************************************************************************/

ds2pri( code )
uschar	*code;
{
	if( *code == ESC_ALPHA )
	    return( bit2pri[*( code + 1 )] );
	else 
	    return( *code + 0x20 );
}
