/* @(#)66	1.9  src/bos/usr/bin/gencat/catio.h, cmdmsg, bos411, 9428A410j 5/12/94 14:41:53 */
/*
 * COMPONENT_NAME: CMDMSG
 *
 * FUNCTIONS: catio.h
 *
 * ORIGINS: 27, 18
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 *
 * (c) Copyright 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

/*
 * Note: There is a duplicate (sort of) of this file in libc.
 * 
 * If you change any values here, make sure the other also gets
 * changed.
 */

#ifndef _BLD_OSF	/* bootstrap indicator */
#include <mesg.h>
#endif /* _BLD_OSF */
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

#define ERR 		(-1)
#define TRUE 		1
#define FALSE 		0

#define QTSTR		"$quote"
#define SETSTR		"$set"

#define PATH_FORMAT	"/usr/lib/nls/msg/%L/%N:/usr/lib/nls/msg/%L/%N.cat"
#define DEFAULT_LANG	"C"

#define SETMIN		1
#define SETMAX  	65535

#define die(s)			fputs(s,stderr),fputc('\n',stderr), exit(1)
