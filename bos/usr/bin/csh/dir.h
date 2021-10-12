/* @(#)17	1.5  src/bos/usr/bin/csh/dir.h, cmdcsh, bos411, 9428A410j 11/12/92 13:28:46 */
/*
 * COMPONENT_NAME: CMDCSH c shell (csh) 
 *
 * FUNCTIONS:
 *
 * ORIGINS:  10,26,27,18,71 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.1
 *
 *
 *
 * Structure for entries in directory stack.
 */

struct directory	{
	struct	directory *di_next;	/* next in loop */
	struct	directory *di_prev;	/* prev in loop */
	unsigned int *di_count;		/* refcount of processes */
	uchar_t	*di_name;		/* actual name */
};

struct directory *dcwd;			/* the one we are in now */
