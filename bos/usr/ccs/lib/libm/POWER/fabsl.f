* @(#)02	1.1  src/bos/usr/ccs/lib/libm/POWER/fabsl.f, libm, bos411, 9428A410j 9/21/93 11:55:51
*
*   COMPONENT_NAME: LIBM
*
*   FUNCTIONS: fabsl
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

* 128-bit long double absolute value function
* 
* C language prototype:
* 
* long double fabsl(long double x);
*
* This is written in Fortran because Fortran has an
* in line intrinsic function to accomplish this, which
* is much faster than coding it out in C

@process debug(callby)
      real*16 function fabsl(%val(x))
      real*16 x
      fabsl = qabs(x)
      return
      end
