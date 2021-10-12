static char sccsid[] = "@(#)79	1.20  src/bos/kernext/mpqp/mpqmpx.c, sysxmpqp, bos411, 9434B411a 8/22/94 16:20:31";
/*
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 * FUNCTIONS: mpqmpx
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
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/intr.h>
#include <sys/mpqp.h>
#include <sys/mpqpdd.h>
#ifdef _POWER_MPQP
#include "mpqpxdec.h"
#endif /* _POWER_MPQP */
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>
 
/*******************************************************************
 *    External Declarations                               	   *
 *******************************************************************/

extern t_acb		*acb_dir[];	/* ACB directory */

extern  t_mpqp_dds	*dds_dir[];	/* DDS directory */


/*******************************************************************
 *    Internal Function Declarations                               *
 *******************************************************************/

/*
 * NAME: mpqmpx
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


int mpqmpx ( dev_t		devno,
	     int		*p_chan,
	     char		*p_channame )

{
    t_acb		*p_acb;		/* pointer to adapter control block */
    t_mpqp_dds		*p_dds;		/* pointer to device data structure */
    int                 tmp_chan;       /* local chan storage */
    
    /* log a trace hook */
    DDHKWD1 (HKWD_DD_MPQPDD, DD_ENTRY_MPX, 0, devno);

    MPQTRACE4("MpxE", devno, *p_chan, *p_channame);
#ifdef _POWER_MPQP
    MPQP_SIMPLE_LOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */

    /* if minor number is invalid, return error */
    if (minor(devno) >= (MAX_ADAPTERS*NUM_PORTS))
    {
#ifdef _POWER_MPQP
    	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
    	MPQTRACE3("Mpe1", devno, EINVAL);
	return(EINVAL);
    }

    /* set up DDS pointer */
    p_dds = dds_dir[minor(devno)];

    /* if dds pointer is null, return error */
    if (p_dds == NULL)
    {
#ifdef _POWER_MPQP
    	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
    	MPQTRACE3("Mpe2", devno, EINVAL);
	return(EINVAL);
    }

    p_acb = acb_dir[p_dds->dds_hdw.slot_num];	/* get the acb pointer */

    /* see if mpqmpx called to deallocate the channel */
    if ( p_channame == (char *)NULL )
    {
	p_dds->dds_wrk.cur_chan_num = 0;        /* Deallocate the channel */

	/* on a deallocate, always set diag flag to 0 */
	p_acb->diag_flag = 0;
    }
    else
    {
	/* get channel allocated indicator */
	tmp_chan = (int)p_dds->dds_wrk.cur_chan_num;

	/* if channel number already allocated, return error */
	if (tmp_chan > 0)
	{
	    MPQTRACE4("Mpx1", p_dds, tmp_chan, devno);
	    DDHKWD5 (HKWD_DD_MPQPDD, DD_EXIT_MPX, EBUSY, devno, 0, 0, 0, 0);
#ifdef _POWER_MPQP
    	    MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
	    return(EBUSY);
	}

	/* if the request is for a diagnostic open */
	if ( *p_channame == 'D' )
	{
	    /* if opens have already occurred on this adapter   */
	    if ( p_acb->n_open_ports > 0 )
	    {
		MPQTRACE4("Mpx2", p_dds, p_acb, p_acb->n_open_ports);
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_MPX, EBUSY, devno, 0, 0, 0, 0);
#ifdef _POWER_MPQP
    	    	MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
		return(EBUSY);
	    }

	    /* here we set the diagnostics flag in the acb to 1 */
	    p_acb->diag_flag = 1;
	}
	else
	    p_acb->diag_flag = 0;		/* not diagnostics open */

	p_dds->dds_wrk.cur_chan_num = 1;        /* allocate channel 0 */
	*p_chan = 0;                            /* channel returned is 0 */
    }

    MPQTRACE4("MpxX", devno, p_channame, *p_chan);
    DDHKWD5 (HKWD_DD_MPQPDD, DD_EXIT_MPX, 0, devno, 0, 0, 0, 0);
#ifdef _POWER_MPQP
    MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */
    return(0);
}
