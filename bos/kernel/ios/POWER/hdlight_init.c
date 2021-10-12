static char sccsid[] = "@(#)96	1.1  src/bos/kernel/ios/POWER/hdlight_init.c, sysios, bos41J, 9515A_all 4/3/95 09:40:07";
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystem
 *
 * FUNCTIONS: 
 *	hdlight_init
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
#include <sys/inline.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>

extern int light_flag;
extern int light_count;
extern Simple_lock disk_light_lock;
extern struct io_map light_map;

#define LIGHT_PORT 0x808

/*
 * NAME: hdlight_init
 *
 * FUNCTION: initialize state of the disk light.  This function
 *	will be rewritten in a future release to be residual data
 *	based.
 *
 * EXECTION ENVIRONMENT:
 *	Called only once from process environment at system
 *	initialization time
 *
 * RETURNS:
 * 	None
 */
void
hdlight_init()
{
	if (__rspc())
	{
		light_flag = 1;
		light_count = 0;
		light_map.busaddr = LIGHT_PORT;
	}
	else
	{
		light_flag = 0;
	}

	if (light_flag)
	{
		lock_alloc(&disk_light_lock, LOCK_ALLOC_PIN, IOS_LOCK_CLASS, 1);
		simple_lock_init(&disk_light_lock);
	}
}
