static char sccsid[] = "@(#)33	1.3  src/bos/usr/ccs/lib/libc/POWER/time_base_to_time.c, libctime, bos411, 9430C411a 7/14/94 16:40:31";
/*
 *   COMPONENT_NAME: libctime
 *
 *   FUNCTIONS: time_base_to_time
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

#include <sys/types.h>
#include <sys/systemcfg.h>
#include <sys/time.h>

/*
 * FUNCTION:  time_base_to_time
 *
 * DESCRIPTION:
 *
 * Service to convert time base to real time (if necessary).
 * If t->flag == RTC_POWER,
 *		return 0;
 * If t->flag == RTC_POWER_PC,
 *		convert t->tb_high and t->tb_low to
 *			seconds and nanoseconds;
 *		set t->flag to RTC_POWER;
 *		return 0;
 * Else
 *		return -1;
 */

int 
time_base_to_time(timebasestruct_t *t,
		  size_t size_of_timebasestruct_t)
  {
  union {
	unsigned long long ull;
	struct { unsigned long hi, lo; } ul;
	} ll_sec;
  unsigned long secs, nsecs;
  unsigned long long ll_quo, ll_rem;
  unsigned long long Xint, Xfrac;

  switch (t->flag)
      {
    case RTC_POWER:
      return 0;
      
    case RTC_POWER_PC:
      t->flag = RTC_POWER;
      /* move to long long variable */
      ll_sec.ul.hi = t->tb_high;
      ll_sec.ul.lo = t->tb_low;

      /* Convert time base to nanoseconds.  The conversion is
       * (mathematically) = (time_base * Xint) / Xfrac.
       * This is done in 64-bit arithmetic.  If done in the
       * obvious way the intermediate product (time_base * Xint)
       * can overflow 64 bits, so the the calculation is done 
       * as follows:
       *
       * ((TB / Xfrac) * Xint) + (((TB % Xfrac) * Xint) / Xfrac)
       *    TB = time base tic, all integer operations.
       *
       * Note that the 64-bit division routine returns both the
       * quotient and remainder, and in the code we keep those
       * two operations together to make sure only one division
       * is done to get both parts.
       */

      Xint = (unsigned long long) _system_configuration.Xint;
      Xfrac = (unsigned long long) _system_configuration.Xfrac;

      ll_quo = ll_sec.ull / Xfrac;
      ll_rem = ll_sec.ull % Xfrac;
      ll_quo *= Xint;
      ll_rem = (ll_rem * Xint) / Xfrac;
      ll_sec.ull = ll_quo + ll_rem;

      secs = ll_sec.ull / (unsigned long long) NS_PER_SEC;
      nsecs = ll_sec.ull % (unsigned long long) NS_PER_SEC;

      t->tb_high = secs;
      t->tb_low = nsecs;

      return 0;

    default:
      return -1;
      }

  }
