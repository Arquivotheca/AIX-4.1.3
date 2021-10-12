/* @(#)94       1.4  src/bos/kernel/io/machdd/md_rspc.h, machdd, bos411, 9428A410j 6/20/94 10:54:39 */
#ifndef _H_MDRSPC
#define _H_MDRSPC
/*
 * COMPONENT_NAME: (MACHDD) Machine Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#ifdef _RSPC

#include <sys/dump.h>
#include <sys/erec.h>

/* RSPC NVRAM Map */
#define	NVSIZE		4096
#define	VARSIZE		512
#define	CONFSIZE	1024

/* Temp (Var) Area Usage */
#define ERRLOG_SIZE	256
#define	ERRLOG_ADDR	NVSIZE - CONFSIZE - VARSIZE
#define	DUMP_ADDR	ERRLOG_ADDR + ERRLOG_SIZE
#define NVBUF_SIZE	ERRLOG_SIZE + sizeof(struct short_dump) + CRC_SIZE

/* Vital dump info structure */
struct short_dump {
   char dm_devicename[20];
   off_t dm_size;
   time_t dm_timestamp;
   int dm_status;
   int dm_flags;
   char dm_filehandle[16];
};

#endif	/* _RSPC */

#endif	/* _H_MDRSPC */
