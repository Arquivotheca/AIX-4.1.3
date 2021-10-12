static char sccsid[] = "@(#)33	1.1  src/bos/usr/ccs/lib/libc/POWER/fp_trapstate.c, libccnv, bos411, 9428A410j 3/31/93 10:22:23";
/*
 *   COMPONENT_NAME: LIBCCNV
 *
 *   FUNCTIONS: fp_trapstate
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

extern int _fp_trapstate_ker(int);

/*
 * NAME: fp_trapstate()
 *
 * FUNCTION:
 *      fp_trapstate(): query or set a process's state regarding the
 *      ability to generate floating point exception traps.
 *
 * EXECUTION ENVIRONMENT:
 *      Problem state library routine.
 *
 *      This routine simply passes its argument to the 
 *      _fp_trapstate_ker() system call, which does the real
 *      work.  The user-visible interface is intentionally
 *      in libc.a, because at a future release it may be 
 *      desirable to redistribute the work between the library
 *      and the system call.
 *
 * NOTES:
 *
 *   INPUT: 
 *
 *   OUTPUT:
 */

int
fp_trapstate( int flag )
  {
  return _fp_trapstate_ker(flag);
  }
