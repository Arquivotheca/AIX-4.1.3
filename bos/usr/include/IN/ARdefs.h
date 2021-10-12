/* @(#)85	1.11  src/bos/usr/include/IN/ARdefs.h, libIN, bos411, 9428A410j 6/16/90 00:16:45 */
/*
 * COMPONENT_NAME: LIBIN
 *
 * FUNCTIONS:
 *
 * ORIGINS: 9,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* Archive format identifying numbers */
#define NO_AR_ID	0x00
#define	S3_AR_ID	0x03
#define S5_AR_ID	0x05
#define BK_AR_ID	0x2a
#define	AIAF_AR_ID	0x2f

/* Definitions for using ARforeach */
typedef struct {
	FILE *file;			/* positioned FILE pointer */
	long date,uid,gid,mode,size; 	/* facts from the ar header */
	long oldpos;			/* file position of old header */
	char *name;}			/* file name ("" is symbol table ) */
    ARparm;
