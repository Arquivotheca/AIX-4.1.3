/* @(#)39	1.3  src/bos/kernel/sys/strinfo.h, sysxpse, bos411, 9428A410j 8/3/91 11:41:43 */
/*
 *   COMPONENT_NAME: SYSXPSE strinfo.h
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_STRINFO_H_
#define	_STRINFO_H_

typedef struct strmdi {
	int heapsize;
	int maxheap;
	int clonemaj;
	int sadmaj;
	int logmaj;
} strmdi_t;

#define	STR_INFO_HEAP	1
#define	STR_INFO_MODS	2
#define	STR_INFO_RUNQ	3

#endif	/* _STRINFO_H_ */
