static char sccsid[] = "@(#)66	1.2  src/bos/kernel/ios/devsw_pin.c, sysios, bos411, 9428A410j 10/4/93 09:42:10";
/*
 * COMPONENT_NAME: (SYSIOS) Device Switch Table services
 *
 * FUNCTIONS:	nodev		nulldev		devswqry
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/device.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/sysconfig.h>
#include <sys/pin.h>
#include <sys/syspest.h>
#include <sys/lockl.h>

struct devsw	*devsw;
extern int	devqry();

int	hiwater;		/* the highest pinned devsw entry */

lock_t	devswlock = LOCK_AVAIL;
 


/*
 * NAME:  nodev
 *
 * FUNCTION:  Routine which sets a user error; placed in illegal entries 
 *	      in the devsw table.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: 
 *
 * RETURN VALUE DESCRIPTION:
 *	ENODEV	Indicates that this is not a valid device entry point
 *
 * EXTERNAL PROCEDURES CALLED:
 */
int
nodev()
{

	return(ENODEV);
}

/*
 * NAME:  nulldev
 *
 * FUNCTION: Null routine; placed in insignificant entries in the devsw table.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: 
 *
 * RETURN VALUE DESCRIPTION:
 *	0	Always successful
 *
 * EXTERNAL PROCEDURES CALLED:
 */
int
nulldev()
{
	return(0);
}

/*
 * NAME:  devswqry
 *
 * FUNCTION:  Queries the status of an entry in the Device Switch Table
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by device drivers to check the status
 *	of a particular entry in the device switch table.
 *      It cannot page fault.
 *
 * NOTES:  This routine is used to check the status of an entry in the 
 *	device switch table.
 *
 * DATA STRUCTURES:  devsw
 *
 * RETURN VALUE DESCRIPTION:	CONF_SUCC upon successful completion;
 *				EINVAL if the specified major number exceeds
 *				the maximum allowed by the kernel.
 *
 *				The status input parameter is also a return
 *				value.
 *
 * EXTERNAL PROCEDURES CALLED:  none
 */
int
devswqry(
register dev_t	devno,
register uint	*status,
caddr_t		*dsdptr)
{
	register int	major_number;/* 'major' portion of devno	*/

	major_number = major(devno);

	if (major_number >= DEVCNT)
	{
		return(EINVAL);			/* invalid major number	*/
	}

	if (major_number <= hiwater) {
		if (status != NULL)
		{
			*status = DSW_UNDEFINED;

			if (devsw[major_number].d_opts & DEV_DEFINED)
			{
				*status = DSW_DEFINED;
	
				if (devsw[major_number].d_read != nodev)
				{
					*status |= DSW_CREAD;
				}	

				if (devsw[major_number].d_write != nodev)
				{
					*status |= DSW_CWRITE;
				}

				if (devsw[major_number].d_strategy != nodev)
				{
					*status |= DSW_BLOCK;
				}

				if (devsw[major_number].d_ttys != NULL)
				{
					*status |= DSW_TTY;
				}

				if (devsw[major_number].d_select != nodev)
				{
					*status |= DSW_SELECT;
				}

				if (devsw[major_number].d_config != nodev)
				{
					*status |= DSW_CONFIG;
				}

				if (devsw[major_number].d_dump != nodev)
				{
					*status |= DSW_DUMP;
				}

				if (devsw[major_number].d_mpx != nodev)
				{
					*status |= DSW_MPX;
				}

				if (devsw[major_number].d_revoke != nodev)
				{
					*status |= DSW_TCPATH;
				}

				if (devsw[major_number].d_opts & CONS_DEFINED)
				{
					*status |= DSW_CONSOLE;
				}

				if (devqry(devno, -1))
				{
					*status |= DSW_OPENED;
				}
			}
		}  /* if status != NULL */

		if (dsdptr != NULL)
		{
			*dsdptr = devsw[major_number].d_dsdptr;
		}
	}

	/* 
	 * do not touch any devsw page that is not pinned.
	 */ 
	else {
		if (status != NULL) 
		{
			*status = DSW_UNDEFINED;
		}

		if (dsdptr != NULL)
		{
			*dsdptr = NULL;
		}
	}
  
	return(CONF_SUCC);

}  /* end devswqry */
