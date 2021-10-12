static char sccsid[] = "@(#)25	1.5  src/bos/kernel/ios/iost.c, sysios, bos411, 9428A410j 6/16/90 03:27:11";
/*
 * COMPONENT_NAME: (SYSIOS) I/O Subsystem
 *
 * FUNCTIONS: iostadd,	iostdel
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
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/iostat.h>
#include <sys/devinfo.h>
#include <sys/sysinfo.h>
#include <sys/sysconfig.h>
#include <sys/var.h>
#include <sys/intr.h>
#include <sys/pin.h>
#include <sys/syspest.h>

struct 	iostat	iostat;		
struct cfgncb	iostat_cfgncb;	/*  config control block */
extern	int	disks_active;	/* global disks active flag */

/*
 * NAME:  iostadd
 *
 * FUNCTION:  Registers  an entry in the iostat table for devices.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by device drivers to register an iostat
 *	table entry to keep statistics for the iostat command to display. 
 *
 * NOTES:  This routine is used to add an entry to the iostat table.
 *	   It can only be called in the process environment. 
 *
 * DATA STRUCTURES:  iostat, dkstat ttystat
 *
 * RETURN VALUE DESCRIPTION:	0 for success, EINVAL for invalid devtype
 *
 * EXTERNAL PROCEDURES CALLED:  pin
 */
int
iostadd(
register int	devtype,
union
{
	struct	ttystat	*ttystp;
	struct	dkstat	*dkstp;
} devstatp)
{
	register int		rc, ipri;
	register struct dkstat	*chainptr;

	/*
	 * Check the device type
	 */
	rc = 0;
	switch (devtype)
	{
		case DD_TTY:
			/*
			 * TTY - return pointer to tty structure
			 */
			devstatp.ttystp = &(sysinfo.ttystat);
			break;	/* exit with good status */

		case DD_DISK:
		case DD_CDROM:
			/*
			 * Device is Disk, Chain caller provided 
			 * structure to dkstat chain and mark used
			 */
			devstatp.dkstp->dknextp = NULL;		
			ipri = i_disable(INTTIMER);	/* disable to timer */
			if (iostat.dkstatp == NULL) 
			{
				iostat.dkstatp = devstatp.dkstp;
			}
			else 
			{
				/*
				 * Find end of chain and add new entry
				 */
				for (chainptr = iostat.dkstatp;
					 chainptr->dknextp != NULL;
					 chainptr = chainptr->dknextp);
				chainptr->dknextp = devstatp.dkstp;
			}
			i_enable(ipri);		/* re-enable interrupts */
			iostat.dk_cnt++;	/* increment disk count */
			break;

		default:
			rc = EINVAL;

	}	/* end switch */

	return(rc);

}  /* end iostadd */

/*
 * NAME:  iostdel
 *
 * FUNCTION:  Deletes an entry from the iostat table.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called by device drivers to remove their
 *	use of an iostat entry.
 *
 * NOTES:  This routine is used to delete an entry from the iostat table
 *
 * DATA STRUCTURES:  iostat
 *
 * RETURN VALUE DESCRIPTION:	none	
 *
 * EXTERNAL PROCEDURES CALLED:  unpin
 */
void
iostdel(
union
{
	struct	ttystat	*ttystp;
	struct	dkstat	*dkstp;
} devstatp)
{
	register int	ipri;	
	struct	dkstat	*chainptr;

	if (devstatp.ttystp != &(sysinfo.ttystat)) 
	{
		/*
		 * Search for disk entry to remove
		 */
		ipri = i_disable(INTTIMER);	/* disable to timer priority */
		if (devstatp.dkstp == iostat.dkstatp)
		{
			iostat.dkstatp = devstatp.dkstp->dknextp;
			iostat.dk_cnt--;	/* decrement disk count */
		}
		else
		{
			/*
			 * Find element to delete or end of chain
			 */
			for (chainptr = iostat.dkstatp;
				(chainptr->dknextp != devstatp.dkstp) &&
				(chainptr->dknextp != NULL);
				chainptr = chainptr->dknextp);
	
			/*
			 * If not end of chain then unchain element
			 */
			if (chainptr->dknextp !=NULL)
			{
				chainptr->dknextp = chainptr->dknextp->dknextp;
				iostat.dk_cnt--;
			}
		}
		i_enable(ipri);
	}

	return;

}  /* end iostdel */

/*
 * NAME:  update_iostats
 *                                                                    
 * FUNCTION:  Update the time statistics for iostat command support.
 *	      This routine scans the disk io statistics dkstat list looking
 *	      for disks that are currently busy. If a busy disk is found the
 *	      dk_time field in the structure is incremented to indicate total
 *	      time busy. Unfortunately this is a fairly poor approximation 
 *	      due to the granularity of the timer tick, but is the most
 *	      portable.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine is called by the timer interrupt handler.
 *                                                                   
 *
 * DATA STRUCTURES: iostat, dkstat
 *
 * RETURN VALUE DESCRIPTION:  None
 *
 * EXTERNAL PROCEDURES CALLED:	None
 */  
void
update_iostats()
{
	struct  dkstat *chainptr;

	disks_active = FALSE; /* assume no disks active on this tick */
	if ((v.v_iostrun == TRUE) && (iostat.dkstatp != NULL))
	{
		/* run the list of registered disks */
		for (chainptr = iostat.dkstatp; chainptr != NULL;
				 chainptr = chainptr->dknextp) 
		{
			if (chainptr->dk_status	& IOST_DK_BUSY)
			{
			/* if disk busy then increment busy count and set
			   global busy flag to indicate at least one disk
			   was busy during this timer tick
			 */
				chainptr->dk_time++;
				disks_active = TRUE; 
			}
		}	/* end of for loop */
	}

	return;

}  /* end update_iostats */

/*
 * NAME:  iostrun_mon
 *                                                                    
 * FUNCTION:  Monitor for the iostrun flag in var.h being changed from a non-run
 *	      to a run condition (from FALSE to TRUE) and clear all counters
 *	      in the dkstat structure since the statistics would be invalid
 *	      if not left relative to the time busy.	      
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine is called by the sysconfig SET_SYSPARMS routine.
 *                                                                   
 *
 * DATA STRUCTURES: iostat, dkstat
 *
 * RETURN VALUE DESCRIPTION:  0 for no error
 *
 * EXTERNAL PROCEDURES CALLED:	i_disable() i_enable()
 */  
int
iostrun_mon(cmd, cur, new)

int		cmd;
struct var	*cur;
struct var	*new;
{
	struct  dkstat *chainptr;
	int	oldpri;
	switch (cmd)
	{
		case CFGV_PREPARE:
			break;

		case CFGV_COMMIT:
			if ((new->v_iostrun == TRUE) &&
			 (cur->v_iostrun == FALSE) && (iostat.dkstatp !=NULL))
			{
				oldpri = i_disable(INTMAX);
				for (chainptr = iostat.dkstatp;chainptr != NULL;
				 chainptr = chainptr->dknextp) 
				{
					chainptr->dk_time = 0;
					chainptr->dk_xfers = 0;
					chainptr->dk_rblks = 0;
					chainptr->dk_wblks = 0;
					chainptr->dk_seek = 0;
				}	/* end of for loop */
				i_enable(oldpri);
			}

	}  /* end switch */
	return(0);
}  /* end iostrun_mon */
