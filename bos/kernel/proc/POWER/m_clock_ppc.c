static char sccsid[] = "@(#)50        1.2  src/bos/kernel/proc/POWER/m_clock_ppc.c, sysproc, bos411, 9428A410j 4/29/94 03:03:45";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: update_system_time_ppc
 *
 *   ORIGINS: 83
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/systemcfg.h>
#include <sys/time.h>

/*
 * NAME:  update_system_time_ppc
 *
 * FUNCTION:  Update the processor clock on Power_PC.
 *		Convert timestruc_t date to Time Base ticks and
 *		load into PowerPC Time Base register.
 *
 * EXECUTION ENVIRONMENT:
 *
 * EXTERNAL PROCEDURES CALLED: mt_tb()
 *
 */

void
update_system_time_ppc(struct timestruc_t new_time)
{
	unsigned long long int tb;

	tb = (((unsigned long long int)new_time.tv_sec * NS_PER_SEC 
	       + new_time.tv_nsec) / _system_configuration.Xint)
		* _system_configuration.Xfrac;
	mttb(tb);
}
