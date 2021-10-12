/* @(#)50	1.3  src/bos/usr/lib/methods/common/pparms.h, cfgmethods, bos411, 9428A410j 6/15/91 16:50:22 */
#ifndef _H_PPARMS
#define _H_PPARMS
/*********************************************************************
 *
 * COMPONENT_NAME: CFGMETHODS      pparms.h
 *
 * ORIGINS : 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * Unpublished Work
 * All Rights Reserved
 *
 * RESTRICTED RIGHTS LEGEND
 * US Government Users Restricted Rights -  Use, Duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ***********************************************************************/

/* This structure is needed for some functions in cfgtools.c */

struct attr {
	char *attribute;
	char *value;
};

#endif /* _H_PPARMS */
