static char sccsid[] = "@(#)79	1.1  src/bos/usr/ccs/lib/libc/quota_init.c, libcquota, bos411, 9428A410j 2/20/91 16:56:14";
/*
 * COMPONENT_NAME: LIBCQUOTA
 *
 * FUNCTIONS:   
 *
 * ORIGINS: 18, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 */

#include <sys/types.h>
#include <jfs/quota.h>

char *qfname = "quota";
char *qfextension[] = INITQFNAMES;
char *quotagroup = "system";

