static char sccsid[] = "@(#)61	1.2  src/bos/usr/ccs/lib/libc_r/__threads_init.c, libcthrd, bos411, 9428A410j 6/8/94 16:31:01";
/*
 *   COMPONENT_NAME: LIBCTHRD
 *
 *   FUNCTIONS: __threads_init
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* prototype for phthread_init routine */
extern void pthread_init(void);

/* typedef for a pointer to a function with
 * void argument and void return type
 */
typedef void (*FPV)(void);

/* Create an uninitialized instance of a function pointer.
 * By default this will be null.  If linked with another object
 * file that has an object of the same name and type, the initialized
 * version will be used.  If libxti.a is linked, it will create
 * an object having an _xti_tli_init_routine, initialized to point
 * to its initialization routine.  This allows the Cute Trick below
 * to execute the libxti.a initialization routine if that library
 * is linked, and do nothing if not linked.
 */
FPV _xti_tli_init_routine;
FPV __dce_compat_init_routine;

/*
 * FUNCTION:  __threads_init
 *
 * DESCRTION:  Called by crt0_r on program initialization.
 *             This routines does all threads-related initialization.
 *
 * RETURNS:  nothing.
 */

void
__threads_init(void)
  {
  pthread_init();		/* libc pthreads initialization */

  /* if _xti_tli_init_routine is not null, it is a function
   * pointer to the libxti.a initialization routine, in which case
   * execute it.  If it is null, libxti.a is not linked.
   */
  if (_xti_tli_init_routine)
    (*_xti_tli_init_routine)();

  /* if __Init_Compatibility_library is not null, it is a function
   * pointer to the DCE compat initialization routine, in which case
   * execute it.  If it is null, the Compatibitilty library is not linked
   */
  if (__dce_compat_init_routine)
    (*__dce_compat_init_routine)();
  }
