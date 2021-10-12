/* @(#)72	1.1  src/bos/usr/ccs/lib/libld/takes.h, libld, bos411, 9428A410j 4/16/91 05:18:42 */
#ifndef _H_TAKES
#define _H_TAKES

/*
 * COMPONENT_NAME: CMDAOUT (libld)
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * The TAKES macro is used to write new style ANSI C externs to compile
 * with ANSI C compilers and with pre-ANSI, K&R C compilers (KRC).
 * Usage is:
 *	extern size_t fread TAKES((void *, size_t, size_t, FILE *)); 
 * Libld.a needs to compile on the RT for the RT version of kdbx.
 */
#ifdef KRC
#define TAKES(s)	()
#else
#define	TAKES(s)	s
#endif

#endif
