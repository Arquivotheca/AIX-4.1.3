static char sccsid[] = "@(#)46	1.5  src/bos/kernel/proc/times.c, sysproc, bos41J, 9512A_all 3/20/95 19:35:28";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: times
 *		uS_tick
 *		
 *
 *   ORIGINS: 27,3,26,83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/param.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/lockl.h>
#include <sys/trchkid.h>
#include <sys/systm.h>

/*
 * NAME: times()
 *
 * FUNCTION: Returns timing values in quantities of clock ticks (Hertz or HZ)
 *
 * EXECUTION ENVIRONMENT: This routine may only be called by a process
 *                        This routine cannot page fault
 *
 * RETURN VALUES:
 *		Writes the number of clock ticks of cpu usage
 *		since process creation to the user supplied address.
 *		Returns the lbolt value, which is the system wide
 *		timer in clock ticks (HZ) since the system boot up.
 *		A -1 is returned if the address supplied by user
 *		is in error.
 */

#define uS_tick(tvp)	((tvp.tv_sec * HZ) + (tvp.tv_usec / (NS_PER_SEC / HZ)))

clock_t
times(struct tms *timbuf)
{
	struct tms build_tms;
	int lbolt_sav, ipri, ipri2;
	
	/*
	 * Acquire locks upfront to avoid context switches, delays, ...
	 *
	 * U_ru is protected by (INTTIMER, U_timer_lock) (cf. sys_timer) and
	 * U_cru is protected by (INTMAX, proc_base_lock) (cf. kwaitpid).
	 */
	ipri = disable_lock(INTTIMER, &U.U_timer_lock);
#ifdef _POWER_MP
	ipri2 = disable_lock(INTMAX, &proc_base_lock);
#endif

	lbolt_sav = lbolt;
	build_tms.tms_utime = (clock_t) uS_tick(U.U_ru.ru_utime);
	build_tms.tms_stime = (clock_t) uS_tick(U.U_ru.ru_stime);
	build_tms.tms_cutime = (clock_t) uS_tick(U.U_cru.ru_utime);
	build_tms.tms_cstime = (clock_t) uS_tick(U.U_cru.ru_stime);

#ifdef _POWER_MP
	unlock_enable(ipri2, &proc_base_lock);
#endif
	unlock_enable(ipri, &U.U_timer_lock);

	if (copyout((caddr_t)&build_tms, (caddr_t)timbuf, sizeof(struct tms))) {
		u.u_error = EFAULT;
		return(-1);
	}

	TRCGENT(0,HKWD_SYSC_TIMES,0,sizeof(struct tms),&build_tms);

	return(lbolt_sav);
}
