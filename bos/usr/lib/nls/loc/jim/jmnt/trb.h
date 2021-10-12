/* @(#)85	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/trb.h, libKJI, bos411, 9428A410j 6/6/91 14:34:21 */

/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         trb.h
 *
 * DESCRIPTIVE NAME:    Trace Control Block Structure Define.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              Kanji Monitor V1.0
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
#ifndef _kj_TRB
#define _kj_TRB

#include "kjmacros.h"   /* Kanji Projcet Macros.                          */
#include "trbdef.h"     /* Trace Control  Block Define.                   */
/*
 *      Create Trace Block Nickname.
 */
typedef struct _trbblk TRB;
typedef struct _trbroot TRBROOT;
struct _trbblk {
struct _trbroot {
char   trblch[12];
                /* Trace Block Inidicates Id. "trace block"               */

long   trblklen;/* Length of Trace Block.                                 */

long   troutadr;/* Write Point offset from TRB.                           */

ulong  troutsno;/* Trace Data Sequential Number.                          */

long   filds;   /* Trace Data Output file desctiptor.                     */

long   trflag;  /* Trace Flag.                                            */
                /* =0 ... Trace Output Memory.                            */
                /* =1 ... Trace Output Disk File.                         */

long   trendadr;/* Last Output Header.                                    */

long   rsv1;    /* **** RESERVED FOR FUTURE USE ****                      */
long   rsv2;    /* **** RESERVED FOR FUTURE USE ****                      */
long   rsv3;    /* **** RESERVED FOR FUTURE USE ****                      */
long   rsv4;    /* **** RESERVED FOR FUTURE USE ****                      */
} trbblk;
uchar  trace[K_TRBSIZ];
                /* Trace Structure.                                       */
};
#endif
