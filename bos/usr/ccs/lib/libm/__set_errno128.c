static char sccsid[] = "@(#)00	1.2  src/bos/usr/ccs/lib/libm/__set_errno128.c, libm, bos411, 9428A410j 10/1/93 17:07:33";
/*
 *   COMPONENT_NAME: LIBM
 *
 *   FUNCTIONS: __set_errno128
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <errno.h>		/* declaration of errno */

/*
 * FUNCTION:  __set_errno128
 *
 * DESCRIPTION:  sets errno to the value of the function's argument.
 *
 * In a threaded environment we have c-language source-
 * level macro that redefines "errno" to call a function
 * to update the per-thread errno value.  There is no
 * corresponding macro for Fortran, so Fortran code which
 * must set errno will call this routine.  This avoids 
 * having to make explicit source-level changes to keep
 * the Fortran code thread safe with regard to errno.
 *
 * RETURNS: Nothing.
 */

void 
__set_errno128(int i)
  {
  errno = i;
  }
