/* @(#)11	1.6  src/bos/usr/bin/src/include/srcodm.h, cmdsrc, bos411, 9428A410j 6/15/90 23:37:24 */
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controller
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#ifndef _H_SRCODM
#define _H_SRCODM

#include <sys/types.h>
#define SRCSYSTEM	SRCsubsys_CLASS
#define SRCSUBSVR	SRCsubsvr_CLASS
#define SRCNOTIFY	SRCnotify_CLASS
#define SRC_ODM_PATH	"/etc/objrepos"
#define SRC_ODM_LOCK	"/etc/objrepos/SRCodmlock"

extern int odmerrno;

struct fieldview
{
	char *c_addr;
	char *db_addr;
	long size;
};

struct objview
{
	char *db_rec;
	struct fieldview *fieldview;
};

#define whaterr(x,y)	((x == -1) ? (int)y : (int)x)

#endif
