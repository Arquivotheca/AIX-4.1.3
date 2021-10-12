static char sccsid[] = "@(#)44	1.2.1.2  src/bos/usr/ccs/lib/libc/__regfree_std.c, libcpat, bos412, 9446B 11/15/94 14:50:44";
/*
 *   COMPONENT_NAME: libcpat
 *
 *   FUNCTIONS: __regfree_std
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/localedef.h>
#include <regex.h>
#include <unistd.h>

/************************************************************************/
/* __regfree_std() - Free Compiled RE Pattern Dynamic Memory		*/
/************************************************************************/

void
__regfree_std(_LC_collate_objhdl_t hdl, regex_t *preg)
{
	if (preg->re_comp != NULL)
		{
		free(preg->re_comp);
		preg->re_comp = NULL;
		}
	if (preg->re_map != NULL)
		{
		free(preg->re_map);
		preg->re_map = NULL;
		}
	return;
}
