/* @(#)51        1.5 8/27/91 12:26:06  */
/*
 * COMPONENT_NAME: User Dictionary Utility
 *
 * FUNCTIONS: header file
 *
 * ORIGINS: IBM
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         kumacros.h
 *
 * DESCRIPTIVE NAME:    Defines User Dictionary Maintenance Macros.
 *
 * COPYRIGHT:           5756-030 COPYRIGHT IBM CORP 1991
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              User Dictionary Maintenance for AIX 3.2
 *
 * CLASSIFICATION:      OCO Source Material - IBM Confidential.
 *                      (IBM Confidential-Restricted when aggregated)
 *
 * FUNCTION:            NA.
 *
 * NOTES:               NA.
 *
 * MODULE TYPE:         Macro.
 *
 *  PROCESSOR:          C
 *
 *  MODULE SIZE:        NA.
 *
 * ENTRY POINT:         NA.
 *
 * EXIT-NORMAL:         NA.
 *
 * EXIT-ERROR:          NA.
 *
 * EXTERNAL REFERENCES: NA.
 *
 * TABLES:              NA.
 *
 * MACROS:              Kanji Project Macro Library.
 *                              NA.
 *                      Standard Macro Library.
 *                              NA.
 *
 * CHANGE ACTIVITY:     NA.
 *
 ********************* END OF MODULE SPECIFICATIONS ************************
 */
#ifndef _kj_kuMAC
#define _kj_kuMAC
/*
 *      Types Define.
 */
#include <sys/types.h>
#if !defined(RS6000)
#ifndef uchar
typedef unsigned char uchar;
#endif
#endif
/*
 *      MRU Length Operation Macro.
 */
#define GMRULEN(adr)     GETSHORT(adr)
#define SMRULEN(adr,dat) SETSHORT(adr,dat)
/*
 *      Dictionary Short Value Get.
 */
#define GETSHORT(adr) ( (((uchar *)(adr))[1]<<8) + ((uchar *)(adr))[0])
#define SETSHORT(adr,dat) \
(((uchar *)adr)[0]=LOW(dat),((uchar *)adr)[1]=HIGH(dat))
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
 *      Cursor Mocve.
 */
#if defined(EXTCUR)
#define CURSOR_MOVE(lin,col) printf("%c[%d;%dH",27,(lin+1),col)
#else
#define CURSOR_MOVE(lin,col) putp(tparm(cursor_address,lin,col))
#endif

/*
 *      Allways Fail Condition.
 */
#define NILCOND     ( 0 == 1 )
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

