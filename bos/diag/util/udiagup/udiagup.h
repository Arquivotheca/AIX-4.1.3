/* @(#)58	1.4  src/bos/diag/util/udiagup/udiagup.h, dsaudiagup, bos411, 9428A410j 12/10/92 16:04:56 */
/*
 *   COMPONENT_NAME: DSAUDIAGUP
 *
 *   FUNCTIONS: Diagnostic header file.
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _h_udiagup
#define _h_udiagup

#define		T_DA		0x0001	/* Diag. Application */
#define		T_MSG		0x0002	/* Message File */
#define		T_LIB		0x0004	/* library */
#define		T_DD		0x0008  /* device driver */
#define		T_CFG 		0x0010  /* config. method */
#define		T_DB		0x0020  /* database .add */
#define		T_CTRL		0x0040  /* controller */
#define		T_UTIL  	0x0080  /* service aid */
#define		T_MC		0x0100  /* micro code */
#define		T_NEW		0x0200  /* new file   */
#define		T_REP		0x0400  /* replacement */
#define		T_DUPDATE	0x0800  /* replacing the Diagnostic Update SA */

/* record of update instance; one per file */
typedef struct {
	char *file; 	/* full file path */
	char *dscp; 	/* description */
	char *ec; 	/* EC number */
	int version; 	/* version level */
	int release; 	/* release level */
	int mod; 	/* mod level */
	char *ptf; 	/* ptf id if file was fixed by a ptf */
	int update; 	/* decimal digit update number */
	long type; 	/* file type flag */
} DIAGUP_REC;

#endif
