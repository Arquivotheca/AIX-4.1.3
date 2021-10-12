/* @(#)36       1.4  src/bos/kernel/sys/sad.h, sysxpse, bos411, 9428A410j 11/2/93 11:54:02 */
/*
 *   COMPONENT_NAME: SYSXPSE sad.h
 *
 *   ORIGINS: 27 63 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   Copyright (c) 1990  Mentat Inc.
 *   
 *   LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#ifndef _SAD_
#define _SAD_

#define	MAXAPUSH	8

/* Ioctl commands for sad driver */
#define	SAD_BASE	('A' << 8)
#define	SAD_SAP		(SAD_BASE + 1)	/* Set autopush information */
#define	SAD_GAP		(SAD_BASE + 2)	/* Get autopush information */
#define	SAD_VML		(SAD_BASE + 3)	/* Validate a list of modules */

/* Ioctl structure used for SAD_SAP and SAD_GAP commands */
struct strapush {
	uint	sap_cmd;
	long	sap_major;
	long	sap_minor;
	long	sap_lastminor;
	long	sap_npush;
	char	sap_list[MAXAPUSH][FMNAMESZ+1];
};

/* Command values for sap_cmd */
#define	SAP_ONE		1	/* Configure a single minor device */
#define	SAP_RANGE	2	/* Configure a range of minor devices */
#define	SAP_ALL		3	/* Configure all minor devices */
#define	SAP_CLEAR	4	/* Clear autopush information */

#ifdef _KERNEL
extern int sad_get_autopush(long, long, struct strapush *);
#endif /* KERNEL */

#endif /* _SAD_ */
