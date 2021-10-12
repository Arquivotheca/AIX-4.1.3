/*
 * COMPONENT_NAME: CMDAOUT (libld)
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef	lint
static char *lddef_sccsid = "@(#)68  1.3  src/bos/usr/ccs/lib/libld/lddef.h, libld, bos411, 9428A410j 6/16/90 02:02:20";
#endif	lint

#ifndef LDLIST

struct ldlist
{
	LDFILE		ld_item;
	struct ldlist	*ld_next;
};

#define	LDLIST	struct ldlist
#define	LDLSZ	sizeof(LDLIST)

#endif
/* static char lddef_ID[ ] = "lddef.h: 1.1 1/7/82"; */
