/* @(#)64	1.2  src/bos/usr/lib/nls/loc/jim/jmnt/fsb.h, libKJI, bos411, 9428A410j 6/6/91 14:29:26 */

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
 * MODULE NAME:         fsb.h
 *
 * DESCRIPTIVE NAME:    Field Save Block Structure.
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
#ifndef _kj_FSB
#define _kj_FSB
#include "kjmacros.h"   /* Kanji Project Macros.                          */

typedef struct fsbblk FSB;

typedef struct fsbblk {
uchar  *string; /* Pointer to Input Field Save Area.                      */
                /* Kanji Monitor Sets this.                               */

uchar  *hlatst; /* Pointer to Input Field Attribute Save Area.            */
                /* Kanji Monitor Sets this.                               */

short  strmax;  /* Length of Maximum String Save Area.                    */
                /* Kanji Monitor Sets this.                               */

short  hlatmax; /* Length of Hilighting Attribute Save Area.              */
                /* Kanji Monitor Sets this.                               */

short  length;  /* Length of Saved Input Field(Per Byte).                 */
                /* Kanji Monitor Sets this.                               */

short  curcol;  /* Position of Input Field Cursor Column(s).              */
                /* Kanji Monitor Sets this.                               */

short  currow;  /* Position of Input Field Cursor Row(s).                 */
                /* Kanji Monitor Sets this.                               */

short  lastch;  /* Last Character Position of Input Field.                */
                /* Kanji Monitor Sets this.                               */

short  curleft; /* Cursor Movement Left Margin.                           */
                /* Kanji Monitor Sets this.                               */

short  curright;/* Cursor Movement Right Margin.                          */
                /* Kanji Monitor Sets this.                               */

long   rsv1;    /* **** RESERVED FOR FUTURE USE ****                      */
long   rsv2;    /* **** RESERVED FOR FUTURE USE ****                      */
long   rsv3;    /* **** RESERVED FOR FUTURE USE ****                      */
long   rsv4;    /* **** RESERVED FOR FUTURE USE ****                      */
};
#endif
