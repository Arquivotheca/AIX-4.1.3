static char sccsid[] = "@(#)83  1.1  src/bos/usr/ccs/lib/libiconv/hash.c, libiconv, bos411, 9428A410j 12/7/93 14:13:11";
/*
 *   COMPONENT_NAME:    LIBICONV
 *
 *   FUNCTIONS:         ccsid_hash
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include<sys/types.h>

uint_t	ccsid_hash(char *name, uint_t tbl_siz)
{
	int	g, h;

	for (h = 0; *name;) {
		h = (h << 4) + *name++;
		if (g = h & 0xf0000000) {
			h ^= g >> 24;
			h ^= g;
		}
	}
	return(h % tbl_siz);
}
