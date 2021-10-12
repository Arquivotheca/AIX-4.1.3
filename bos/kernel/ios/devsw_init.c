static char sccsid[] = "@(#)13  1.8  src/bos/kernel/ios/devsw_init.c, sysios, bos411, 9428A410j 7/27/93 20:56:10";
/*
 * COMPONENT_NAME: (SYSIOS) Device Switch Table initialization
 *
 * FUNCTIONS: devsw_init
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/pin.h>
#include <sys/conf.h>
#include <sys/syspest.h>

extern int	hiwater;

extern struct devsw	*devsw;

/*
 * NAME:  devsw_init
 *
 * FUNCTION:  Creates and initializes the Device Switch Table
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called at system initialization time.
 *      It cannot page fault.
 *
 * NOTES:  This routine is used to create and initialize the device
 *	switch table.
 *
 * DATA STRUCTURES:  devsw
 *
 * RETURN VALUE DESCRIPTION:	none
 *
 * EXTERNAL PROCEDURES CALLED:  pin
 */
void
devsw_init()

{
	register int		devsw_size;
	register int		new_entry;
	extern			nodev();
	extern int		devcnt;
	extern struct devsw	static_devsw[];

	devsw_size = sizeof(struct devsw) * DEVCNT;
	devsw = (struct devsw *)(xmalloc(devsw_size, PGSHIFT, kernel_heap));
	assert(devsw != NULL);

	/* 
	 * pin the first STATIC_DEVCNT entries individally.
	 * we need to pin each one since otherwise devswdel may cause
	 * the page with these entries to be unpinned.
	 */ 

	for (new_entry = 0; new_entry < STATIC_DEVCNT; new_entry++)
       		assert(pin(devsw + new_entry, sizeof(struct devsw)) 
			== PIN_SUCC);

	/*
	 *
	 * Copy the static devsw table into the dynamic devsw table.
	 */

	if (devcnt != 0 )
		for (new_entry = 0; new_entry < devcnt; new_entry++)
		{
			bcopy (&static_devsw[new_entry], &devsw[new_entry],
				 sizeof(struct devsw));
			devsw[new_entry].d_opts |=  DEV_DEFINED;
		}

	for (new_entry = devcnt; new_entry < DEVCNT; new_entry++)
	{
		devsw[new_entry].d_open = nodev;
		devsw[new_entry].d_close = nodev;
		devsw[new_entry].d_read = nodev;
		devsw[new_entry].d_write = nodev;
		devsw[new_entry].d_ioctl = nodev;
		devsw[new_entry].d_strategy = nodev;
		devsw[new_entry].d_ttys = NULL;
		devsw[new_entry].d_select = nodev;
		devsw[new_entry].d_config = nodev;
		devsw[new_entry].d_print = nodev;
		devsw[new_entry].d_dump = nodev;
		devsw[new_entry].d_mpx = nodev;
		devsw[new_entry].d_revoke = nodev;
		devsw[new_entry].d_dsdptr = NULL;
		devsw[new_entry].d_selptr = NULL;
		devsw[new_entry].d_opts = DEV_NOT_DEFINED;
	}

	devcnt = DEVCNT; 
	
	hiwater = STATIC_DEVCNT - 1;

}  /* end devsw_init */



