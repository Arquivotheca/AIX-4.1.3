static char sccsid[] = "@(#)23	1.2.1.2  src/bos/usr/ccs/lib/libc/towupper.c, libcchr, bos411, 9428A410j 1/12/93 11:20:03";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: towupper
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991 , 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include <sys/lc_sys.h>
#include <sys/localedef.h>
#include <ctype.h>

#undef towupper
    
wint_t towupper(wint_t wc)
{
    return _CALLMETH(__lc_ctype,__towupper)(__lc_ctype, wc);
}

