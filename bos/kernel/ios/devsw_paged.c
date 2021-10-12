static char sccsid[] = "@(#)67  1.2  devsw_paged.c, sysios, bos410 7/27/93 20:56:12";
/*
 * COMPONENT_NAME: (SYSIOS) Device Switch Table services
 *
 * FUNCTIONS:	devswadd	devswdel
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
#include <sys/sysmacros.h>
#include <sys/device.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/sysconfig.h>
#include <sys/pin.h>
#include <sys/syspest.h>
#include <sys/lockl.h>

extern struct devsw	*devsw;
extern int	devqry();

extern int	hiwater;		/* the highest pinned devsw entry */

extern lock_t	devswlock;
 


/*
 * NAME:  devswadd
 *
 * FUNCTION:  Adds an entry to the Device Switch Table
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by device drivers to add themselves to
 *	the device switch table.
 *      It can page fault.
 *
 * NOTES:  This routine is used to add an entry to the device switch table.
 *
 * DATA STRUCTURES:  devsw
 *
 * RETURN VALUE DESCRIPTION:	CONF_SUCC upon successful completion;
 *				EEXIST if the specified device switch entry
 *				is in use and cannot be replaced;
 *				ENOMEM if unable to pin entry due to
 *				insufficient real memory;
 *				EINVAL if the specified major number exceeds
 *				the maximum allowed by the kernel.
 *
 * EXTERNAL PROCEDURES CALLED:  pin
 */
int
devswadd(
register dev_t	devno,
register struct devsw	*dswptr)
{
	register int	major_number;/* 'major' portion of devno	*/
	register int 	rc;
	register int	lrc;

	major_number = major(devno);

	if (major_number >= DEVCNT)
	{
		return(EINVAL);			/* invalid major number	*/
	}
 
	/* 
	 * serialize access to hiwater
	 */
	lrc = lockl(&devswlock, LOCK_SHORT);
	ASSERT(lrc == LOCK_SUCC);

	/*
	 * DEV_DEFINED means that the device already exists in the devsw tbl.
	 * devqry rc=1 means that the device has at least 1 open.
	 */
	if ((devsw[major_number].d_opts & DEV_DEFINED) && (devqry(devno, -1)))
	{
		unlockl(&devswlock);
		return(EEXIST);
	}

	if (major_number > hiwater)  
	{
		/*
		 *  update hiwater and make every entry with index less than
		 *  or equal to the new hiwater pinned exactly once.
		 */
 
		int i;
		for (i = hiwater + 1; i <= major_number; i++)  
		{
			if (pin(&devsw[i], sizeof(struct devsw)) != PIN_SUCC)
			{
				/*
				 * undo successful pins
				 */
	
				int j;
				for ( j = hiwater + 1; j < i; j++)
				{ 
				   rc = unpin(&devsw[j], sizeof(struct devsw));
				   assert(rc == UNPIN_SUCC);
				}
				unlockl(&devswlock);
				return(ENOMEM);
			}
		}

		hiwater = major_number;
	} 
        
	devsw[major_number] = *dswptr;
	devsw[major_number].d_selptr = NULL;
#ifdef _POWER_MP
	devsw[major_number].d_opts &= DEV_MPSAFE;
	devsw[major_number].d_opts |= DEV_DEFINED;
#else
	devsw[major_number].d_opts = DEV_DEFINED;
#endif /* _POWER_MP */

	unlockl(&devswlock);
	return(CONF_SUCC);

}  /* end devswadd */

/*
 * NAME:  devswdel
 *
 * FUNCTION:  Deletes an entry from the Device Switch Table
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by device drivers to remove themselves
 *	from the device switch table.
 *      It can page fault.
 *
 * NOTES:  This routine is used to delete an entry from the device switch
 *	table.
 *
 * DATA STRUCTURES:  devsw
 *
 * RETURN VALUE DESCRIPTION:	CONF_SUCC upon successful completion;
 *				EEXIST if the specified device switch entry
 *				is in use and cannot be removed;
 *				ENODEV if the specified device switch entry
 *				is not defined;
 *				EINVAL if the specified major number exceeds
 *				the maximum allowed by the kernel.
 *
 * EXTERNAL PROCEDURES CALLED:  unpin
 */
int
devswdel(
register dev_t	devno)
{
	register int	rc;	/* return value from unpin routine	*/
	register int	major_number;/* 'major' portion of devno	*/
	register int 	lrc;
	extern		nodev();

	major_number = major(devno);

	if (major_number >= DEVCNT)
	{
		return(EINVAL);			/* invalid major number	*/
	}

	if (!(devsw[major_number].d_opts & DEV_DEFINED))
	{
		return(ENODEV);			/* device not defined	*/
	}

	/*
	 * DEV_DEFINED means that the device exists in the devsw tbl.
	 * devqry rc=1 means that the device has at least 1 open.
	 */
	if ((devsw[major_number].d_opts & DEV_DEFINED) && (devqry(devno, -1)))
	{
		return(EEXIST);
	}

	/* 
	 * serialize access to hiwater.
	 */
        lrc = lockl(&devswlock, LOCK_SHORT);
	ASSERT(lrc == LOCK_SUCC);

	devsw[major_number].d_open = nodev;
	devsw[major_number].d_close = nodev;
	devsw[major_number].d_read = nodev;
	devsw[major_number].d_write = nodev;
	devsw[major_number].d_ioctl = nodev;
	devsw[major_number].d_strategy = nodev;
	devsw[major_number].d_ttys = NULL;
	devsw[major_number].d_select = nodev;
	devsw[major_number].d_config = nodev;
	devsw[major_number].d_print = nodev;
	devsw[major_number].d_dump = nodev;
	devsw[major_number].d_mpx = nodev;
	devsw[major_number].d_revoke = nodev;
	devsw[major_number].d_dsdptr = NULL;
	devsw[major_number].d_selptr = NULL;
	devsw[major_number].d_opts = DEV_NOT_DEFINED;

	
	if (major_number >= hiwater) 
	{
		assert(major_number == hiwater);
		
		/* 
	         * unpin every undefined devsw entry next to hiwater and
		   lower hiwater.
	 	 */
	
		for ( ; devsw[hiwater].d_opts == DEV_NOT_DEFINED; hiwater--)
		{
			rc = unpin(&devsw[hiwater], sizeof(struct devsw));
			assert(rc == UNPIN_SUCC);
		}
	}
			 
	unlockl(&devswlock);
	return(CONF_SUCC);

}  /* end devswdel */

/*
 * NAME:  devswchg
 *
 * FUNCTION:  Change funttion pointer in device switch table
 *
 * EXECUTION ENVIRONMENT:
 *	This can only be called from the process environment
 *
 *
 * DATA STRUCTURES:  devsw
 *
 * RETURN VALUE DESCRIPTION:	0 upon successful completion;
 *				ENODEV if a valid device is not specified
 *				EINVAL if type command is not valid
 *
 * EXTERNAL PROCEDURES CALLED:  none
 */
int
devswchg(
dev_t	devno,			/* device number */
int	type,			/* command */
int	(*newfunc)(),		/* new devswitch intry */
int	(**oldfunc)())		/* previous value is returned here */
{
	int major_number;	/* 'major' portion of devno	*/
	int rc,rv;		/* return code */
	int (**altfunc)();	/* function pointer to change */
	

	major_number = major(devno);
	rc = 0;

	rv = lockl(&devswlock, LOCK_SHORT);
	ASSERT(rv == LOCK_SUCC);

	if (major_number > hiwater ||
			!(devsw[major_number].d_opts & DEV_DEFINED))
	{
		rc = ENODEV;
	}
	else
	{
		/*
		 * find device switch entry to change
		 */
		switch(type)
		{
			case DSW_CREAD:
				altfunc = &devsw[major_number].d_read;
				break;
			case DSW_CWRITE:
				altfunc = &devsw[major_number].d_write;
				break;
			case DSW_BLOCK:
				altfunc = &devsw[major_number].d_strategy;
				break;
			case DSW_SELECT:
				altfunc = &devsw[major_number].d_select;
				break;
			case DSW_DUMP:
				altfunc = &devsw[major_number].d_dump;
				break;
			case DSW_MPX:
				altfunc = &devsw[major_number].d_mpx;
				break;
			case DSW_TCPATH:
				altfunc = &devsw[major_number].d_revoke;
				break;
			case DSW_CONFIG:
				altfunc = &devsw[major_number].d_config;
				break;
			default:
				rc = EINVAL;
				break;
		}

		/*
		 * save the old device switch entry and replace
		 * it with new value
		 */
		if (rc == 0)
		{
			*oldfunc = *altfunc;
			*altfunc = newfunc;
		}
	}

	unlockl(&devswlock);
	return(rc);

}  /* end devswcfg */



