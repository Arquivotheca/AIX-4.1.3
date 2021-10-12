static char sccsid[] = "@(#)29	1.23  src/bos/kernext/mpqp/mpqselect.c, sysxmpqp, bos411, 9434B411a 8/22/94 16:21:20";
/*
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 * FUNCTIONS: mpqselect
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <errno.h>
#include <sys/device.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/intr.h>
#include <sys/mpqp.h>
#include <sys/mpqpdd.h>
#ifdef _POWER_MPQP
#include "mpqpxdec.h"
#endif /* _POWER_MPQP */
#include <sys/poll.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/comio.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>

/*******************************************************************
 *    External Declarations                               	   *
 *******************************************************************/

extern t_acb		*acb_dir[];	/* ACB directory */

extern t_mpqp_dds	*dds_dir[];	/* DDS directory */


/*******************************************************************
 *      Global declarations for the MPQP Device Driver             *
 *******************************************************************/

/*
 * NAME: mpqselect
 *                                                                    
 * FUNCTION: High level description of what the procedure does
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	Environment-Specific aspects, such as - 
 *	Preemptable        : Maybe
 *	VMM Critical Region: Yes
 *	Runs on Fixed Stack: Yes
 *	May Page Fault     : Yes
 *      May Backtrack      : Yes
 *                                                                   
 * NOTES: More detailed description of the function, down to
 *	    what bits / data structures, etc it manipulates. 
 *
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.
 *
 * RETURN VALUE DESCRIPTION: What this code returns (NONE, if it has no 
 *			     return value)
 */

int mpqselect ( dev_t		devno,
		unsigned short	events,
		unsigned short	*p_revent,
		int		chan )

{
	int 		adapt_num;	/* adapter number, zero based */
	int		port_num;	/* port number within adapter	     */
	t_acb		*p_acb;		/* pointer to adapter control block */
	t_chan_info 	*p_tmp_chinfo;  /* temporary channel info pointer */
	t_mpqp_dds	*p_dds;		/* pointer to device data structure */
	unsigned char	done;

	/* log a trace hook */
	DDHKWD5 (HKWD_DD_MPQPDD, DD_ENTRY_SELECT, 0, devno, 0, 0, 0, 0);
	MPQTRACE4("SelE", devno, events, p_revent);

#ifdef _POWER_MPQP
	MPQP_SIMPLE_LOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */

	/* if minor number is invalid, return error */
	if (minor(devno) >= (MAX_ADAPTERS*NUM_PORTS))
	{
		MPQTRACE2("Sel1", devno);
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_SELECT, EINVAL, devno, 0,0,0,0);
#ifdef _POWER_MPQP
        	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */

		return(EINVAL);
	}

	/* if the channel number out of range (only 0 is valid for now) */
	if ( chan != 0 )
	{
		MPQTRACE3("Sel2", devno, chan);
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_SELECT, ECHRNG, devno, 0,0,0,0);
#ifdef _POWER_MPQP
        	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return(ECHRNG);
	}

	/* get dds pointer from dds directory */
	p_dds = dds_dir[minor(devno)];

	/* if port not configured, return error */
	if (p_dds == NULL)
	{
		MPQTRACE2("Sel3", devno);
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_SELECT, ENXIO, devno, 0,0,0,0);
#ifdef _POWER_MPQP
        	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return(ENXIO);
	}

	/* this flag is set in mpqoff whenever there is a command queue   */
	/* available and will be cleared here for the purpose of checking */
	/* if there is any incoming command in mpqoffl during the time    */
	/* of execution of this mpqselect module                          */

	p_dds->dds_wrk.cmd_avail_flag = FALSE;


	adapt_num = p_dds->dds_hdw.slot_num;
	p_acb = acb_dir[adapt_num];

	port_num = p_dds->dds_dvc.port_num;

	MPQTRACE4("SelZ", p_dds, port_num, adapt_num);

	/* now we go out and get the channel information data structure */
	/* pointer from the dds for this channel.  The channel informa- */
	/* tion data structure contains select queue pointers and other */
	/* necessary information for our continued operation	        */

	/* get that pointer */
	p_tmp_chinfo = p_dds->dds_wrk.p_chan_info[chan];

	MPQTRACE3("SelB", p_tmp_chinfo, p_tmp_chinfo->devflag);

	if /* a kernel process has possession of this channel, select should */
	/* never receive control....*/
	( p_tmp_chinfo->devflag & DKERNEL )
	{
		MPQTRACE4("Sel5", devno, p_revent, p_tmp_chinfo->devflag);
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_SELECT, EINVAL, devno,0,0,0,0);
#ifdef _POWER_MPQP
        	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return (EINVAL);
	}

	done = TRUE;
	while ( done == TRUE )
	{

		/* now we check for requested selections, one at a time */

		if /* select on receive data available */
		( events & POLLIN )
		{
			if /* there is at least one event on the rcv queue */
			(p_tmp_chinfo->p_rcv_head != NULL)
			{
				*p_revent |= POLLIN;
			}
			else
			{
				if /* selnotify must be called when this 
				      event occurs later */
				( !(events & POLLSYNC) )
				{
					p_tmp_chinfo->sync_flags |= POLLIN;
				}
			}
		} /* end of check for POLLIN flag */

		if /* select on status available */
		( events & POLLPRI )
		{
			if /* there is at least one event on the status queue */
			(p_tmp_chinfo->p_stat_head != NULL)
			{
				*p_revent |= POLLPRI;
			}
			else
			{
				if /* selnotify must be called when 
				      this event occurs later */
				( !(events & POLLSYNC) )
				{
					p_tmp_chinfo->sync_flags |= POLLPRI;
				}
			}
		}  /* end of check for POLLPRI flag */

		/* if there is a new command during 
		   the time of execution of this select we capture it */

		if ( p_dds->dds_wrk.cmd_avail_flag != FALSE )
		{
			done = TRUE;
			p_dds->dds_wrk.cmd_avail_flag = FALSE;
		}
		else
		{
			done = FALSE;
		}
	} /* end while */


	/* return value of zero tells poll/select 
	   logic to sleep if necessary */

	MPQTRACE5("SelX", devno, events, *p_revent, chan);
	DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_SELECT, 0, devno, 0, 0, 0, 0);
#ifdef _POWER_MPQP
        MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
	return(0);
}
