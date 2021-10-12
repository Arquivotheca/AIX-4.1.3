/* @(#)73	1.3  src/bos/usr/lib/nls/loc/jim/jmnt/kjmacros.h, libKJI, bos411, 9428A410j 6/6/91 14:31:50 */
/*
 * COMPONENT_NAME :	(LIBKJI) Japanese Input Method (JIM)
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *      Kanji Project. Macros Define.
 */
#ifndef _kj_MAC
#define _kj_MAC
/*
 *      Types Define.
 */
#include <sys/types.h>
#ifdef	UCHARNEEDED
typedef unsigned char uchar;
#endif	/* UCHARNEEDED */

/*
 *      Macros. For Programming.
 */
/*
 *      Check Warning Level Error Code for Kanji Routines.
 */
#define ISWARN(rc)  (((int)(rc)<=(-100)) && ((int)(rc)>(-1000)))
/*
 *      Check Fatal Level Error Code for Kanji Routine.
 */
#define ISFATAL(rc) ((int)(rc)<=(-1000))
/*
 *      Generate Events Control Block Idetifier.
 */
#define ECBID(id,ver,rel,mai)\
( ((unsigned char)id )<<24      /* System ID.                           */\
 +((unsigned char)ver)<<16      /* Version No.                          */\
 +((unsigned char)rel)<<8       /* Release No.                          */\
 +((unsigned char)mai)          /* Maintenance No.                      */\
)
/*
 *      Get from LSB to direction MSB 8bit.
 */
#define LOW(x)     ((unsigned char)(((unsigned short)x) & 0xff))
/*
 *      Get from LSB 8bit Position to Direction MSB 8Bit.
 */
#define HIGH(x)    ((unsigned char)(((unsigned short)x) >> 8))
/*
 *      Get DBCS Char from Character Pointed Area.
 */
#define CHTODBCS(x)  CHPTTOSH(x)
/*
 *      Get Short From Character Pointed 2byte.
 */
#define CHPTTOSH(adr) \
( (((unsigned char *)(adr))[0]<<8) +((unsigned char *)(adr))[1])
/*
 *      Set DBCS Char to Character Pointed Area.
 */
#define DBCSTOCH(adr,dat) SHTOCHPT(adr,dat)
/*
 *      Set Short Data to Character Pointed Area.
 */
#define SHTOCHPT(adr,dat)\
(((unsigned char *)adr)[0]=HIGH(dat),((unsigned char *)adr)[1]=LOW(dat))
/*
 *      Which is Miminum Number Get.
 */
#define MIN( a,b )  (( (a)<=(b) ) ? a:b)
/*
 *      Which is Maximum Number Get.
 */
#define MAX( a,b )  (( (a)> (b) ) ? a:b)
/*
 *      Allways Fail Condition.
 */
#define NILCOND     ( 0==1 )
/*
 *      Alignment Specified Number(Aligment Round).
 */
#define ALIGN( x,siz ) ((x) + ( ((x) % siz ) ? siz - ((x) % siz) : 0 ))
/*
 *      Alginment Specified Number(Alignment Truncate).
 */
#define ALIGNT( x,siz) ((x) - ( ((x) % siz) ? (x) % siz:0 ))
#ifdef  lint
#define IDENTIFY(func)
#else
#define IDENTIFY(func) static char *mod_name="@(#)func"
#endif

#endif
