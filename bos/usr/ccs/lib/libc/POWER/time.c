static char sccsid[] = "@(#)70	1.2  src/bos/usr/ccs/lib/libc/POWER/time.c, libctime, bos411, 9430C411a 7/13/94 09:07:49";
/*
 *   COMPONENT_NAME: libctime
 *
 *   FUNCTIONS: time
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/systemcfg.h>
#include <time.h>		/* time_t */
#include <sys/time.h>		/* NS_PER_SEC */

/* The following will cause xlc to generate inline code
 * to read some special purpose registers.  This avoids 
 * writing the whole function in assembler.
 */

time_t read_rtcu(void);		/* POWER and 601 real time clock upper */
unsigned int read_tbu(void);	/* PowerPC time base upper */
unsigned int read_tbl(void);	/* PowerPC time base lower */

#pragma mc_func read_rtcu	{ "7c6402a6" }		/* mfspr r3, RTCU */
#pragma mc_func read_tbu	{ "7c6d42e6" }		/* mftbu r3 */
#pragma mc_func read_tbl	{ "7c6c42e6" }		/* mftb r3 */

#pragma reg_killed_by read_rtcu		gr3
#pragma reg_killed_by read_tbu		gr3
#pragma reg_killed_by read_tbl		gr3

/*
 *  NAME: time
 *
 *  FUNCTION: Return time in seconds since EPOCH
 *
 *  EXECUTION ENVIRONMENT:
 *  Standard register usage and linkage convention.
 *
 *  NOTES:
 *  For POWER and 601 processors, simply read the real time clock upper.
 *
 *  For PowerPC, read the time base register (64 bits) and convert
 *  time base to seconds.  The conversion is done inline, rather than
 *  calling a service, to make this routine a speedy as possible.
 *
 *  PERFORMANCE:
 *  Someone, once upon a time, appears to have thought that the performance
 *  of this routine was important enought to write it in assembler.
 *  The alternative would have been to call the gettimer() kernel
 *  service.  This version takes some care to be as quick as possible.
 *  For the POWER case, the only extra cost over an assembler implementation
 *  is that this code will buy a stack frame.  The PowerPC case will be
 *  much slower, since it has to do the 64-bit integer math to convert
 *  time base format to seconds.
 *
 *  RETURN VALUE DESCRIPTION:
 *   unsigned long integer - the time in seconds since the EPOCH
 */

time_t
time(time_t *tp)
  {
  time_t rtc_sec;
  unsigned int flag;
  
  unsigned long tbu_1, tbu_2, tbl;
  union {
	unsigned long long ull;
	struct { unsigned long hi, lo; } ul;
	} ll_sec;

  /* 
   * The scheduler still treats the inline code like function calls, 
   * so this little trick greatly improves the scheduling, especially 
   * for the POWER case.
   */

  flag = (tp != (void *) 0);

  switch (_system_configuration.rtc_type)
      {
    case RTC_POWER:		/* POWER and 601 */
      rtc_sec = read_rtcu();
      /* do this inline here to avoid another branch */
      if (flag)
	*tp = rtc_sec;
      return rtc_sec;
      break;

    case RTC_POWER_PC:		/* PowerPC */
    re_read:
      tbu_1 = read_tbu();
      tbl = read_tbl();
      tbu_2 = read_tbu();
      if (tbu_1 != tbu_2)	/* if we got a carry into upper, start over */
	goto re_read;

      /* move to long long variable */
      ll_sec.ul.hi = tbu_1;
      ll_sec.ul.lo = tbl;

      /* convert time base to nanoseconds
       * Caution:  Divide then multiply to avoid an overflow
       */
      ll_sec.ull /= (unsigned long long) _system_configuration.Xfrac;
      ll_sec.ull *= (unsigned long long) _system_configuration.Xint;
      
      /* convert nanoseconds to seconds */
      ll_sec.ull /= (unsigned long long) NS_PER_SEC;

      rtc_sec = (unsigned long) ll_sec.ull;
      break;

    default:
      /* Unknow rtc_type.  If this was the kernel there would be an
       * assert() here.
       */
      rtc_sec = 0;
      }

  if (flag)
    *tp = rtc_sec;
  return rtc_sec;
  }
