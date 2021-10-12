/*@(#)49        1.2  src/bos/usr/lib/methods/common/cfgtoolsx.h, cfgmethods, bos411, 9428A410j 3/25/91 14:02:43*/
#ifndef _H_CFGTOOLSX
#define _H_CFGTOOLSX
/*
 *
 * COMPONENT_NAME: (CFGMETH) cfgtoolsx.h (configuration method tools)
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
 */


/* this structure is returned by get_attr_list */
struct	attr_list {
	int	attr_cnt;	/* number of attributes in list */
	struct	CuAt cuat[1];	/* array of CuAt like attr objects */
};

#endif /* _H_CFGTOOLSX */
