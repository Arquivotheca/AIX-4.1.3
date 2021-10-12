static char sccsid[] = "@(#)08	1.1  src/bos/usr/ccs/lib/libc/wctype.c, libcchr, bos411, 9428A410j 1/12/93 12:41:38";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: wctype
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include <sys/lc_sys.h>
#include <sys/localedef.h>
#include <ctype.h>
#include <string.h>

wctype_t wctype(char *name)
{
	wctype_t rc;
	rc = _CALLMETH(__lc_ctype,__get_wctype)(__lc_ctype, name);

	if (rc == (wctype_t) -1)
		return 0;
	else
		return rc;
}
