/* @(#)63	1.2.1.1  src/bos/usr/lib/nls/loc/jim/jmnt/ext.h, libKJI, bos411, 9428A410j 7/23/92 03:25:45 */
/*
 * COMPONENT_NAME :	Japanese Input Method - Kanji Monitor
 *
 * ORIGINS :		27 (IBM)
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/********************* START OF MODULE SPECIFICATIONS **********************
 *
 * MODULE NAME:         ext.h
 *
 * DESCRIPTIVE NAME:    Extended Information Block Structure.
 *
 * COPYRIGHT:           5601-061 COPYRIGHT IBM CORP 1988
 *                      LICENSED MATERIAL-PROGRAM PROPERTY OF IBM
 *                      REFER TO COPYRIGHT INSTRUCTIONS FORM NO.G120-2083
 *
 * STATUS:              DBCS  Editor  V1.0
 *                      DBCS  Monitor V1.0
 *                      Kanji Monitor V1.0
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
 * 7/05/89 add dict name structure
 * 6/09/92 change dict name structure and extinf structure
 ********************* END OF MODULE SPECIFICATIONS ************************
 */
#ifndef _kj_EXT
#define _kj_EXT
#include "kjmacros.h"   /* Kanji Project Macros.                          */
#include "kmpf.h"       /* KMPF structure	                          */

#define SDICT_NUM 16  		/* Max counts of system dictionaries    */
#define SDICT_LEN 80  		/* Max length of system dictionary name */

/* dict name structure 7/05/89 */
typedef struct {
    char *sys[SDICT_NUM+1];     /* system dict. file name       	*/
    char *user;         	/* user dict. file name   		*/
    char *adj;          	/* adjunct dict. file name		*/
    } MONDICTS;

typedef struct extinf EXT;
struct extinf {
KMPF   *prokmpf;/* ptr to KMPF structure		                  */
                /* DBCS Editor Sets this.                                 */

short  maxstc;  /* Max Number of Input Field Column                       */
                /* DBCS Editor Sets this.                                 */

short  maxstr;  /* Max Number of Input Field Row                          */
                /* DBCS Editor Sets this.                                 */

short  maxa1c;  /* Max Number of Auxiliary Area No.1 Column               */
                /* DBCS Editor Sets this.                                 */

short  maxa1r;  /* Max Number of Auxiliary Area No.1 Row                  */
                /* DBCS Editor Sets this.                                 */

short  maxa2c;  /* **** RESERVED FOR FUTURE USE ****                      */
                /* DBCS Editor Sets this.                                 */

short  maxa2r;  /* **** RESERVED FOR FUTURE USE ****                      */
                /* DBCS Editor Sets this.                                 */

short  maxa3c;  /* **** RESERVED FOR FUTURE USE ****                      */
                /* DBCS Editor Sets this.                                 */

short  maxa3r;  /* **** RESERVED FOR FUTURE USE ****                      */
                /* DBCS Editor Sets this.                                 */

short  maxa4c;  /* **** RESERVED FOR FUTURE USE ****                      */
                /* DBCS Editor Sets this.                                 */

short  maxa4r;  /* **** RESERVED FOR FUTURE USE ****                      */
                /* DBCS Editor Sets this.                                 */

MONDICTS *dicts; /* dict name structure address       7/05/89              */
long   rsv2;    /* **** RESERVED FOR FUTURE USE ****                      */
long   rsv3;    /* **** RESERVED FOR FUTURE USE ****                      */
long   rsv4;    /* **** RESERVED FOR FUTURE USE ****                      */
};
#endif
