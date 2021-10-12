static char sccsid[] = "@(#)94  1.4  src/bos/kernext/aio/aio.c, sysxaio, bos412, 9445C412a 11/1/94 09:34:34";
/*
 * COMPONENT_NAME: (SYSXAIO) Asynchronous I/O
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "aio_private.h"

/*
 * global data
 */

int     aio_state = AIO_UNCONFIGURED;   /* configuration state of aio */
int	minservers = DEFMINSERVERS;	/* starting number of kprocs */
int	maxservers = DEFMAXSERVERS;	/* max kprocs we will create */
queue	qtab[QTABSIZ];			/* array of queue headers */
int	maxreqs = DEFMAXREQUESTS;	/* maximum requests outstanding */
int	requestcount = 0;		/* total requests outstanding */
int	s_priority = DEFSPRIORITY;	/* priority of kprocs */
ulong   aio_generation = 0;             /* generation number for iosuspends */
struct	devtab	*devnop;		/* devid */

s_list	servers = {NULL, 0};		/* list of available servers */
Simple_lock	aio_qlock;		/* global queue list lock */
Simple_lock	devtab_lock;		/* global dev table  lock */
