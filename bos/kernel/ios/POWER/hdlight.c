static char sccsid[] = "@(#)95	1.2  src/bos/kernel/ios/POWER/hdlight.c, sysios, bos41J, 9519A_all 5/2/95 15:01:25";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: 
 *	write_operator_panel_light, operator_panel_light_off
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/types.h>
#include <sys/ioacc.h>
#include <sys/adspace.h>
#include <sys/intr.h>
#include <sys/errno.h>
#include <sys/syspest.h>

struct io_map light_map =
	{ IO_MEM_MAP, 0, SEGSIZE, BID_VAL(IO_ISA, ISA_IOMEM, 0), 0};
int light_flag = 0;
int light_count = 0;
Simple_lock disk_light_lock;

/*
 * NAME: write_operator_panel_light
 *
 * FUNCTION: set state of disk activity light.
 *
 * EXECUTION ENVIRONMENT:
 *	Called from process or interrupt environment
 *
 *	Callable from INTMAX interrupt handlers
 *
 *	Called only on RSPC platforms
 *
 * NOTES:
 *	This is a documented kernel service
 *
 * RETURNS:
 *	0 - success
 *	ENODEV - Disk light not present
 */

int
write_operator_panel_light(
	int state)
{
	int ipri;
	volatile char *light_addr;

	/*
	 * If disk light does not exist fail
	 */
	if (light_flag == 0)
	{
		return(ENODEV);
	}

	light_addr = iomem_att(&light_map);
	ipri = disable_lock(INTMAX, &disk_light_lock);

	/*
	 * state of 1 is a on request.  state of 0 is a off request
	 */
	if (state)
	{
		/*
		 * turn on count transition from 0 to 1
		 */
		if (light_count == 0)
		{
			*light_addr = 0x01;
		}
		light_count++;
	}
	else
	{
		/*
		 * turn off disk light on count transition of 1 to 0
		 */
		light_count--;
		if (light_count == 0)
		{
			*light_addr = 0x00;
		}

		/*
		 * check for too many off commands
		 */
		ASSERT(light_count >= 0);
	}

	unlock_enable(ipri, &disk_light_lock);
	iomem_det((void *)light_addr);

	return(0);
}

/*
 * NAME: operator_panel_light_off
 *
 * FUNCTION: Turn the disk activity light off
 *
 * EXECUTION ENVIRONMENT:
 *	Called from function shutdown_io() prior to walking busses
 *	to disable IO in preparation for a warm IPL.
 *
 *	Called at INTMAX
 *
 *	Called only on RSPC platforms
 *
 * RETURNS:
 *	None
 */

void
operator_panel_light_off()
{
	int ipri;
	volatile char *light_addr;

	/*
	 * If disk light does not exist then don't turn it off
	 */
	if (light_flag)
	{

		light_addr = iomem_att(&light_map);
		*light_addr = 0x00;
		iomem_det((void *)light_addr);
	}
	return;
}
