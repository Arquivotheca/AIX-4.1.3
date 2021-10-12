static char sccsid[] = "@(#)78	1.52  src/bos/kernext/mpqp/mpqioctl.c, sysxmpqp, bos411, 9438B411a 9/20/94 15:45:38";

/*
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 * FUNCTIONS: mpqioctl
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


/*******************************************************************
 *    Include Files						   *
 *******************************************************************/

#include <sys/types.h>
#include <sys/adspace.h>
#include <sys/comio.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/device.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/ioctl.h>
#include <sys/intr.h>
#include <sys/listmgr.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mpqp.h>
#include <sys/mpqpdd.h>
#ifdef _POWER_MPQP
#include "mpqpxdec.h"
#endif /* _POWER_MPQP */
#include <sys/mpqpdiag.h>
#include <sys/param.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/sysdma.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/xmem.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>

/*******************************************************************
 *    External Function Declarations				   *
 *******************************************************************/

extern void mpqtimer( t_mpqp_dds * );

/* Que an Adapter Command */
extern int		que_command(t_acb *,
				    unsigned char *,
				    unsigned char *,
				    unsigned int,	
				    unsigned int);	

/* Reload Adapter Software */
extern int		reload_asw(t_acb *,
				   t_mpqp_dds *,
				   int,
				   t_rw_cmd *,
				   unsigned long,
				   unsigned long,
				   unsigned long,
				   unsigned int);	

/* Adapter Query Function */
extern int		adapt_query ( char *,
				      t_acb *,
				      dev_t ,
				      int );
/* change parameters function */
extern int		change_parms ( t_chg_parms *,
				       unsigned char,
				       t_acb *, 
				       t_mpqp_dds *,
				       int,
				       dev_t,
				       unsigned int );
/* Flush Port function */
extern int		flush_port ( unsigned char,
				     t_acb *,
				     t_mpqp_dds *,
				     int,
				     dev_t,
				     unsigned int );

/* Get Status function */
extern int		get_stat ( t_mpqp_dds *,
				   char *,
				   dev_t );

/* Query Statistics */
extern int		query_stats ( t_query_parms *, 
    				      t_mpqp_dds    *,
				      int,
				      dev_t );

/* start auto response */
extern int		start_ar ( t_start_ar *,
				   unsigned char,
				   t_acb *,
				   t_mpqp_dds *,
				   int,
				   dev_t,
				   unsigned int);

/* start device function */
extern int		start_dev ( t_start_dev *,
				    t_acb *,
				    unsigned char,
				    t_mpqp_dds *,
				    int,
				    int,
				    dev_t,
				    unsigned int );

/* stop auto response */
extern int		stop_ar ( unsigned char,
				  t_acb *,
				  t_mpqp_dds *,
				  int,
				  dev_t,
				  unsigned int );

/* Halt Port function */
extern int		stop_port ( unsigned char,
				    t_acb *,
				    t_mpqp_dds *,
				    int,
				    dev_t,
				    unsigned int );

/* adapter trace off */
extern int		trace_off( unsigned char,
				   t_acb *,
				   t_mpqp_dds *,
				   int,
				   dev_t,
				   unsigned int );

/* adapter trace on */
extern int		trace_on ( unsigned char,
				   t_acb *,
				   t_mpqp_dds *,
				   int,
				   dev_t,
				   unsigned int );


/*******************************************************************
 *    External Declarations					   *
 *******************************************************************/

extern t_acb		*acb_dir[];	/* ACB directory */

extern t_mpqp_dds	*dds_dir[];	/* DDS directory */

/*******************************************************************
 *    Stat memory....						   *
 *******************************************************************/

static struct xmem	*dp;		/* cross memory descriptor */

/*******************************************************************
 *    Internal Function Declarations				   *
 *******************************************************************/
int		mpqioctl();
static int	proc_cmd();
void 		halt_pend_timer( t_mpqp_dds * );

/*
 * NAME: mpqioctl
 *								      
 * FUNCTION: Performs a variety of control functions to the MPQP device
 *								      
 * EXECUTION ENVIRONMENT:
 *								     
 *	Preemptable	   : Yes
 *	VMM Critical Region: No
 *	Runs on Fixed Stack: Yes
 *	May Page Fault	   : Yes
 *	May Backtrack	   : Yes
 *								     
 * NOTES: More detailed description of the function, down to
 *	    what bits / data structures, etc it manipulates. 
 *
 * DATA STRUCTURES: 
 *
 * RETURN VALUE DESCRIPTION: 
 */  

int mpqioctl ( dev_t		devno,
	       int		cmd,
	       caddr_t		arg,
	       int		devflag,
	       int		chan,
	       int		ext )

{
    unsigned short		adapt_num;     /* adapter number, zero based */
    volatile unsigned long	bus_sr;	       /* IO Seg Reg number mask */
    int				error;	       /* return value	*/
    unsigned short		port_num;      /* port number, zero based    */
    t_mpqp_dds			*p_dds;	       /* dds pointer */
    t_acb			*p_acb;	       /* pointer to ACB struct */
    unsigned long		iob;	       /* adapter io base addr */
    unsigned long		memb;	       /* adapter bus memory base */
    volatile t_chan_info	*p_tmp_chinfo; /* pointer for devflag */
    unsigned int                sleep_flag;    /* sleep flag for que_command */
    int				sleep_rc;
#ifdef _POWER_MPQP
    int				old_pri;
#endif /* _POWER_MPQP */


    /* log a trace hook */

    DDHKWD5(HKWD_DD_MPQPDD, DD_ENTRY_IOCTL, 0, devno, cmd,
	    devflag, chan, ext);

    MPQTRACE4("IioE", devno, cmd, arg);
#ifdef _POWER_MPQP
    MPQP_SIMPLE_LOCK(&mpqp_mp_lock);
#endif /* _POWER_MPQP */

    MPQTRACE3("Ii1E", devflag, *arg);

    /* if minor number is invalid, return error */
    if (minor(devno) >= (MAX_ADAPTERS*NUM_PORTS))
    {
	    MPQTRACE2("Iie1", devno);
	    DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_IOCTL, EINVAL, devno, 0,0,0,0);
#ifdef _POWER_MPQP
    	    MPQP_SIMPLE_UNLOCK(&mpqp_mp_lock);
#endif /* _POWER_MPQP */
	    return(EINVAL);
    }

    /* if the channel number out of range (only 0 is valid for now) */
    if ( chan != 0 )
    {
	    MPQTRACE3("Iie2", devno, chan);
	    DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_IOCTL, ECHRNG, devno, 0,0,0,0);
#ifdef _POWER_MPQP
            MPQP_SIMPLE_UNLOCK(&mpqp_mp_lock);
#endif /* _POWER_MPQP */

	    return(ECHRNG);
    }

    /* get dds pointer from dds directory */
    p_dds = dds_dir[minor(devno)];

    /* if port not configured, return error */
    if (p_dds == NULL)
    {
	    MPQTRACE2("Iie3", devno);
	    DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_IOCTL, EINVAL, devno, 0,0,0,0);
#ifdef _POWER_MPQP
            MPQP_SIMPLE_UNLOCK(&mpqp_mp_lock);
#endif /* _POWER_MPQP */

	    return(EINVAL);
    }

    adapt_num = HDW.slot_num;
    p_acb = acb_dir[adapt_num];
    port_num = DVC.port_num;
    MPQTRACE2("Ii2E", p_dds);

    /* gain access to bus memory */

    bus_sr = BUSIO_ATT(p_acb->io_segreg_val, 0);

    /* get memory and io base addresses from the acb for the adapter  */

    iob = p_acb->io_base + bus_sr;
    MPQTRACE5("IioZ", p_acb->io_base, iob, p_acb, DVC.port_state);
    memb = p_acb->mem_base + bus_sr;

    p_tmp_chinfo = p_dds->dds_wrk.p_chan_info[chan];

    /* now we will use the cmd parameter to switch for various operations */
    
    error = 0;
    switch ( cmd )
    {
	case CIO_START:		/* Start Device Command */
	    if /* port state is not opened */
	       ( DVC.port_state != OPEN )
	    {
		MPQTRACE4("Iio4", p_dds, DVC.port_state, devno);
		DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_IOCTL, EBUSY, devno,
			0, 0, 0, 0); 
		error = EBUSY;
		break;
	    }
	    /* Call init_recv before calling start_dev.  This will prevent */
	    /* a timing window on the adapter where a receive buffer       */
	    /* indicate was overwriting the start before it was processed  */
	    /* on the adapter.						   */
	    if ( !error )
	    {
	        if /* this is the first start for this port */
		    ( ++WRK.num_starts == 1 )
	        {
		    init_xmit( p_acb, p_dds );	/* enable transmits */
	        }
		else
		{
		    /* number of starts on a port should not be greater
		     * than one.
		     */
		    WRK.num_starts = 1;
		}
	        if /* this is the first start for the adapter */
		    ( ++p_acb->num_starts == 1 )
	        {
		    /* Now determine if any pending HALTS have not completed */
		    /* If so, set a timer for 1 second and sleep.  Continue  */
		    /* until the pending HALT has completed.		     */
	    	    while ( p_acb->halt_complete == FALSE )
		    {
		      timeout((int (*)(void))halt_pend_timer,(int)p_dds,(HZ*1));
		      WRK.halt_sleep_event = EVENT_NULL;
		      MPQTRACE2("isse",WRK.halt_sleep_event);
#ifdef _POWER_MPQP
		      e_sleep_thread((void *)&WRK.halt_sleep_event,&mpqp_mp_lock, EVENT_SHORT);
#else
		      e_sleep((void *)&WRK.halt_sleep_event, EVENT_SHORT);
#endif /* _POWER_MPQP */
		    }
		    if ( !init_recv( p_acb, p_dds ))
		    {
		        error = ENOMEM;		/* no mbufs */
		        free_recv( p_acb, p_dds );
		        p_acb->num_starts = 0;
		        free_xmit( p_acb, p_dds );	/* free xmit map */
		        --WRK.num_starts;
		        break;			/* so quit */
		    } 
	        }
	    	if ( !error )
	    	{
	    	    if (error = start_dev (arg, p_acb, port_num, p_dds, 
		        devflag, chan, devno, sleep_flag))
		    {
			MPQTRACE2("Iio5", error);
		        free_xmit( p_acb, p_dds );	/* free xmit map */
		        --WRK.num_starts;
			if (--p_acb->num_starts == 0)
		            free_recv( p_acb, p_dds );
	 	        DVC.port_state=OPEN;  /* start failed; set port_state */
		    }
		    else {
			/* number of starts shouldn't be greater than 	*/
			/* number of opens 				*/
                	if(p_acb->num_starts > p_acb->n_open_ports)
                   	    p_acb->num_starts = p_acb->n_open_ports;
		    }
	    	}
	    }
	    break;


	case CIO_HALT:		/* Halt Device Command */

	    MPQTRACE4("Iih1", port_num, chan, devno);

	    if (( DVC.port_state == HALTED ) ||
		( DVC.port_state == HALT_REQUESTED ))
	    {
		break;
	    }

	    /*  Since the HALTS are async. to the STARTS a check is     */
	    /*  required to determine if any HALTS are still pending.   */
	    /*  Set a adapter state variable = FALSE to indicate that a */
	    /*  HALT has started but not completed.                     */
	    /*  A check in made in the CIO_START sequence to determine  */
	    /*  if a HALT is pending before the receive buffers are     */
	    /*  re-allocated.                                           */

	    p_acb->halt_complete = FALSE;	/* indicate HALT started */

	    WRK.num_starts = 0;		/* make sure to init. transmit table */

	    /*  If this is the last port to close, disable frame	*/
	    /*  reception.						*/

	    if ( --p_acb->num_starts == 0 )
		disable_recv( p_acb );

	    /*  Issue a stop port command to the adapter then wait	*/
	    /*  for it to complete.					*/

	    /*  Setup timer for completion of HALT sequence.		*/

	    timeout((int (*)(void))mpqtimer, (int)p_dds, (HZ*5));
	    WRK.sleep_on_halt    = TRUE;
	    WRK.halt_sleep_event = EVENT_NULL;

#ifdef _POWER_MPQP
		MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */
	    error = stop_port ( port_num, p_acb, p_dds, chan, devno,
				sleep_flag );
#ifdef _POWER_MPQP
		MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */

	    /*  Make sure the adapter hasn't responded to the stop port	*/
	    /*  command.  If it has WRK.sleep_on_halt = FALSE then	*/
	    /*  bypass this step.					*/
	    if ( WRK.sleep_on_halt )
	    {
		MPQTRACE2("Isse",WRK.halt_sleep_event);
#ifdef _POWER_MPQP
	        sleep_rc = e_sleep_thread((void *)&WRK.halt_sleep_event,&mpqp_mp_lock, EVENT_SHORT);
#else
	        sleep_rc = e_sleep((void *)&WRK.halt_sleep_event, EVENT_SHORT);
#endif /* _POWER_MPQP */
	    }

	    MPQTRACE3("Iese",WRK.halt_sleep_event,sleep_rc);
	    /*  If the port did not HALT then queue a stat. element	*/
	    /*  indicating a FORCED halt.				*/
            if ( DVC.port_state == DORMANT_STATE )
            {
                /* queue halt done status block */
#ifdef _POWER_MPQP
		MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */
                que_stat(p_dds, 0, CIO_HALT_DONE, CIO_OK, MP_FORCED_HALT, 0, 0);
#ifdef _POWER_MPQP
		MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */
		DVC.port_state = HALTED;
            }

	    /*  Free up transmit resources for this port.  If frame	*/
	    /*  reception has been disabled, free up receive resources.	*/

	    free_xmit( p_acb, p_dds );
	    if ( !p_acb->recv_enabled )
		free_recv( p_acb, p_dds );

	    p_acb->halt_complete = TRUE;	/* no HALTS pending */

	    break;

	case CIO_QUERY: /* Query Statistics Command */
	    MPQTRACE2("Iiq1", p_dds );
	    error= query_stats( arg, p_dds, p_tmp_chinfo->devflag, devno );
	    break;

	case CIO_GET_STAT: /* Get Status Command */
	    error = get_stat ( p_dds, arg, devno );
	    break;

	case Q_GETSTAT: /* Get Status Command */
	{
	    int			old_pri;	/* interrupt level	*/
	    t_chan_info		*p_ch_info;	/* channel info pointer */
	    t_get_stat		*p_get_stat;	/* overlay for arg */
	    t_sel_que		*p_stat_elem;	/* select queue status elt */
	    unsigned long	tmp_code;	/* code value */
	    unsigned long	tmp_cmd;	/* command value */
	    unsigned long	tmp_rqe;	/* response queue element */
            unsigned int	code;           /* temp. loc. for status code */
            unsigned int	opt0;           /* temp. loc. for option 0 */

	    /* overlay user's arg with get_stat template */

	    p_get_stat = (t_get_stat *)arg;

	    /* get pointer to channel information */

	    p_ch_info = p_dds->dds_wrk.p_chan_info[0];

	    /* disable interrupts until we finish with queue */
#ifdef _POWER_MPQP
	    MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
	    old_pri = i_disable(INTOFFL3);
#endif /* _POWER_MPQP */

	    /* now check if any entries exist on the status queue */

	    if /* status queue pointer is NULL, no entries */
	       (p_ch_info->p_stat_head == (t_sel_que *)NULL)
	    {
		error = ENOMSG;
#ifdef _POWER_MPQP
	    	MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
		i_enable(old_pri);
#endif /* _POWER_MPQP */
	    }
	    else
	    {
		/* now we will deque the status element */
		/* and save the code and rqe values	*/

		/* first we set p_stat_elem to point to the first structure */
		/* the status linked list */

		p_stat_elem = p_ch_info->p_stat_head;	/* point to first elt */

		/* now we set temporary code and rqe values from the status */
		/* entry we just pointed to */

		tmp_cmd = p_stat_elem->stat_block.code; /* save cmd type */

		tmp_code = p_stat_elem->stat_block.option[0]; /* save status */

		tmp_rqe = p_stat_elem->rqe_value;	/* save rqe value */

		/* next we set the status head pointer to the link pointer */
		/* from the status block */

		p_ch_info->p_stat_head = p_stat_elem->p_sel_que; /* deque it */

		if /* status head ptr is now null, make status tail ptr null */
		   (p_ch_info->p_stat_head == (t_sel_que *)NULL)
		{
		    p_ch_info->p_stat_tail = (t_sel_que *)NULL;
		}

		code = p_stat_elem->stat_block.code;
		opt0 = p_stat_elem->stat_block.option[0];

		/* now we zero out the select queue element and add it back  */
                /* to the select queue available chain if the stat. element  */
                /* was not a CIO_LOST_STATUS or CIO_ASYNC_STATUS.            */

		p_stat_elem->rqe_value = 0;
		p_stat_elem->stat_block.code = 0;
		p_stat_elem->stat_block.option[0] = 0;
		p_stat_elem->stat_block.option[1] = 0;
		p_stat_elem->stat_block.option[2] = 0;
		p_stat_elem->stat_block.option[3] = 0;

		switch ( code )
                {
                  case CIO_LOST_STATUS:           /* lost status overflow */
                  {   
                    p_stat_elem->p_sel_que = (t_sel_que *)NULL;
                    p_ch_info->p_lost_stat = p_stat_elem;
                    break;
                  }

                  case CIO_ASYNC_STATUS:          /* lost data from receive */
                  {
                    /* make sure if this is a que element for LOST_DATA only */
	            if ( opt0 == CIO_LOST_DATA ) 
		    { 
                      p_stat_elem->p_sel_que = (t_sel_que *)NULL;
                      p_ch_info->p_lost_rcv = p_stat_elem;
                      break;
		    }
                  }

                  default:                     /* must be one or the other */
		    p_stat_elem->p_sel_que = p_ch_info->p_sel_avail; 
		    p_ch_info->p_sel_avail = p_stat_elem;
                }

#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

		/* finally, move edrr contents if necessary */

		if /* user has set pointer to a local edrr */
		   (p_get_stat->p_edrr != (char *)NULL)
		{
		    xfer_edrr(p_acb, tmp_rqe, p_get_stat->p_edrr);	
		}

		p_get_stat->type = 
		    tmp_code ? TEST_FAIL : TEST_PASS;
		p_get_stat->status = tmp_cmd;

	    }
	    break;
	}

	case MP_START_AR:	/* Start Auto Response */
	    error = start_ar ( arg, port_num, p_acb, p_dds, chan, devno,
			       sleep_flag );
	    break;

	case MP_STOP_AR:	/* Stop Auto Response */
	    error = stop_ar (port_num, p_acb, p_dds, chan, devno,
			     sleep_flag );
	    break;

	case MP_CHG_PARMS:	/* Change Parameters Command */
	    error = change_parms(arg, port_num, p_acb, p_dds, chan, devno,
				 sleep_flag );
	    break;

	case MP_FLUSH_PORT:	/* Flush Queued Commands for a Port */
	    error = flush_port (port_num, p_acb, p_dds, chan, devno, 
				sleep_flag );
	    break;

	case Q_SDELAY:		/* set the NDELAY bit on the device */
	    break;

	case Q_CDELAY:	/* clear the NDELAY bit on the device */
	    break;

	case MP_TRACEON:	/* Turn tracing on for all ports on adapter */
	    error = trace_on ( port_num, p_acb, p_dds, chan, devno,
				sleep_flag );
	    break;

	case MP_TRACEOFF:	/* Turn tracing off for all ports on adapter */
	    error = trace_off ( port_num, p_acb, p_dds, chan, devno,
				sleep_flag );
	    break;

	case MP_AQUERY:		/* Query adapter trace data */
	    error = adapt_query( arg, p_acb, devno, devflag );
	    break;

	case MP_RMEM:	/* read from adapter bus memory	       */
	case MP_WMEM:	/* write to adapter bus memory	      */
	    error = mem_rw(p_acb, cmd, arg, iob, memb);
	    break;

	case MP_RASW:	/* Reload adapter software */

	    /* invoke reload_asw to actually do adapter software */
	    /* reload */
	    sleep_flag = 0;
	    error = reload_asw(p_acb, p_dds, chan, arg, bus_sr, iob, memb, 
                               sleep_flag);

	    break;

	case MP_QPCMD:	/* perform command specified in parameter block */
	{
	    unsigned char	sub_cmd;	/* requested command */
	    t_usr_cmd		*p_usr_cmd;	/* pointer to diagnostic */
						/* control block */
	    t_adap_cmd		tmp_lcl_cmd;	/* pointer to current command */
		
	    /* zero out local command block in stack */

	    bzero(&tmp_lcl_cmd, sizeof(t_adap_cmd));

	    p_usr_cmd = (t_usr_cmd *) arg; /* cast arg to struct DCB ptr */

	    sub_cmd = (unsigned char)p_usr_cmd->command; /* get command */

	    /* Set adapter memory window to view data structures */

	    SET_CPUPAGE( p_acb, bus_sr, p_acb->ds_base_page ); 

	    tmp_lcl_cmd.cmd_typ = sub_cmd;

	    error = proc_cmd(port_num, p_acb, p_dds, chan, devflag, sub_cmd, 
			     &tmp_lcl_cmd, p_usr_cmd, sleep_flag);

	    break;
	}

	case Q_GETIC:	/* get interrupt count since last restart */
	    error = p_acb->c_intr_rcvd;
	    break;
	
	default:
	    error = 0;
	    break;
    }

    MPQTRACE3("IioX", error, devno);
    DDHKWD5(HKWD_DD_MPQPDD, DD_EXIT_IOCTL, error, devno,  0, 0, 0, 0 ); 

    /* free access through segment register in bus_sr */

    BUSIO_DET(bus_sr);

    if (error == -1) 
	error = EIO;

#ifdef _POWER_MPQP
    MPQP_SIMPLE_UNLOCK( &mpqp_mp_lock );
#endif /* _POWER_MPQP */

    return (error);
}
 
/*
 * proc_cmd - this function will process a command once it has been
 *	      set up in the ioctl code
 */

static int proc_cmd ( int		port_num,
		      t_acb		*p_acb,
		      t_mpqp_dds	*p_dds,
		      int		chan,
		      int		devflag,
		      int		sub_cmd,
		      t_adap_cmd	*p_cmd_blk,
		      t_usr_cmd_ovl	*p_usr_cmd,
		      unsigned int      sleep_flag )
{

    int			error;		/* return value for function */
    unsigned int	memb;		/* memb */
    unsigned char	short_pn;	/* port number into character */
    int			old_pri;


    MPQTRACE5("IpcE", port_num, p_acb, p_dds, chan);
    MPQTRACE5("Ipc1", devflag, sub_cmd, p_cmd_blk, p_usr_cmd);

    sleep_flag = 0;
    memb = p_acb->mem_base;		/* get shared memory base */

    error = 0;			/* assume good things */

    short_pn = (unsigned char)port_num; /* squeeze port number in char */

    PIO_PUTC(&(p_cmd_blk->port_nmbr),short_pn);

    switch (sub_cmd)
    {

	/* Begin Adapter Diagnostics Commands */

	case MEMORY_CKSUM:	/* Memory checksum     */
	case ROS_QUIK_TEST:	/* ROS type quick test */
	case RAM_EXT_TEST:	/* RAM extended test   */
	    {
		PIO_PUTLR(&(p_cmd_blk->u_data_area.c_ovl.tst_addr),
			   p_usr_cmd->u_data_area.c_ovl.tst_addr);
		PIO_PUTSR(&(p_cmd_blk->u_data_area.c_ovl.tst_length),
		    p_usr_cmd->u_data_area.c_ovl.tst_length);
		/* send that command down */
#ifdef _POWER_MPQP
		MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
		old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
		error = que_command( p_acb, p_cmd_blk, NULL, 0, sleep_flag);
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

	    }
	    break;
	case ROS_CPU_AU_TEST:	/* ROS CPU auto test   */
	case ROS_IFACE_TEST:	/* ROS SSTIC4 Auto Test */
	    {
		/* send that command down */
#ifdef _POWER_MPQP
                MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
                old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
		error = que_command( p_acb, p_cmd_blk, NULL, 0, sleep_flag );
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

	    }
	    break;
	case CIO_ROS_AU_TEST:	/* CIO ROS Auto Test  */
	case SCC_ROS_AU_TEST:	/* SCC ROS Auto Test  */
	case PORT_DMA_TST:	/* port dma test */
	    {
		/* set up the control field */
		PIO_PUTC(&(p_cmd_blk->u_data_area.d_ovl.data[0]),
			   p_usr_cmd->u_data_area.d_ovl.data[0]);
		/* send that command down */
#ifdef _POWER_MPQP
                MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
                old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
		error = que_command( p_acb, p_cmd_blk, NULL, 0, sleep_flag);
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

	    }
	    break;
	case DIAG_XMT_LNG:		/* Diagnostic Transmit Long */
	case DIAG_XMT_GTHR:		/* Diagnostic Transmit Gather */
	case BUS_MSTR_DMA:		/* Bus Master DMA Test */
	    {
		register int	i;
		unsigned int	temp_addr1;	/* temporary addr calc	*/
		unsigned int	temp_addr2;	/* temporary addr calc	*/
			
		if /* first time through for this adapter */
		   (p_acb->p_dma_tst_buf == (char *)NULL)
		{
		    if ((p_acb->p_dma_tst_buf = 
			 xmalloc(4096, 12, pinned_heap)) == NULL) {
			    MPQTRACE1("Ipc3");
			    return(ENOMEM);
		    }

		    if /* the descriptor has not been allocated */
		       ( dp == (struct xmem *)NULL )
		    {
			if ((dp = xmalloc(sizeof(struct xmem), 
					  2, pinned_heap)) == NULL) {
				MPQTRACE2("Ipc4", sizeof(struct xmem));
				return(ENOMEM);
			}

			/* set up for d_master */

			dp->aspace_id = XMEM_GLOBAL;
			dp->subspace_id = NULL;
		    }

		}

		/* now we put stuff in our 4K page */
		{
		    register unsigned short *p_quick =
			 (unsigned short *)p_acb->p_dma_tst_buf;
		    for (i = 0 ; i < 2048 ; i++ )
			*p_quick++ = i;
		}

		/*  Make system memory available to the adapter for DMA    */
		/*  operations.                                            */
		d_unmask( p_acb->dma_channel_id );

		/* Map the host memory to bus memory */
		d_master(p_acb->dma_channel_id, DMA_READ,
			 p_acb->p_dma_tst_buf, 4096, dp, p_acb->dma_base);

		switch (sub_cmd)
		{
		    case BUS_MSTR_DMA:
		    case DIAG_XMT_GTHR:

			temp_addr1 = (unsigned int)p_cmd_blk;	/* kludge */
			temp_addr1 += 16;	/* point into command block */
			temp_addr2 = (unsigned int)p_acb->dma_base;
			for(i=0;i<8;i++)
			{
			    PIO_PUTLR((t_adap_cmd *)temp_addr1, temp_addr2);
			    temp_addr1 += 4;
			    temp_addr2 += 512;
			}
			temp_addr1 = (unsigned int)p_cmd_blk;	/* kludge */
			temp_addr1 += 48;
			for(i=0;i<8;i++)
			{
			    PIO_PUTSR((t_adap_cmd *)temp_addr1, 500);
			    temp_addr1 += 2;
			}
			if /* this is transmit gather */
			   ( sub_cmd == DIAG_XMT_GTHR )
			{
			    PIO_PUTC(&(p_cmd_blk->lngth), 8);
			}
			break;

		    case DIAG_XMT_LNG:	

			PIO_PUTLR(&(p_cmd_blk->u_data_area.d_ovl.data[0]),
					p_acb->dma_base);
			PIO_PUTSR(&(p_cmd_blk->u_data_area.d_ovl.data[4]),
					4096);

		} /* end of switch statement */	

#ifdef _POWER_MPQP
                MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
                old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */

		error = que_command(p_acb, p_cmd_blk, NULL, 0, sleep_flag);
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

		d_complete(p_acb->dma_channel_id, 0, 
			   p_acb->p_dma_tst_buf, 4096, dp, p_acb->dma_base);

	    }
	    break;

	/* End of Adapter Diagnostic Commands */

	/* Begin Adapter Setup/Status Commands */

	case RTN_MEM_SIZE:		/* Return Memory Size	     */
	case GET_INTF_ID:		/* Get Interface ID	     */
	case GET_EIB_ID:		/* Get EIB ID		     */
	case INIT_WD_TMR:		/* Initialize Watchdog Timer */
	case GET_ROS_VRSN:		/* Get ROS Version	     */
	    {
		/* send that command down */
#ifdef _POWER_MPQP
                MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
                old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
		error = que_command( p_acb, p_cmd_blk, NULL, 0, sleep_flag);
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

	    }
	    break;

	case CFG_CIO_PORT:		/* Configure CIO Port	     */
	case CFG_SCC_CHNL:		/* Configure SCC Channel     */
	case CFG_DMA_CHNL:		/* Configure DMA Channel     */
	case CFG_HDW_TMR:		/* Configure Hardware Timer  */
	case PRIORITY_SWTCH:		/* Priority Switch	     */
	case CPU_TMR_QRY:		/* CPU Timer Query	*/
	case CPU_DMA_QRY:		/* CPU DMA Query	*/
	    {

		/* set up the control field */

		PIO_PUTC(&(p_cmd_blk->u_data_area.d_ovl.data[0]),
			   p_usr_cmd->u_data_area.d_ovl.data[0]);

		/* send that command down */

#ifdef _POWER_MPQP
                MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
                old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
		error = que_command( p_acb, p_cmd_blk, NULL, 0, sleep_flag);
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

	    }
	    break;

	case DATA_BLASTER:		/* Data Blaster Test */
	    {

		/* Set up prescalar value */

		PIO_PUTSR(&(p_cmd_blk->u_data_area.d_ovl.data[0]), 0x02);

		/* Set up Interrupt mask */

		PIO_PUTC(&(p_cmd_blk->u_data_area.d_ovl.data[2]), 0xff);
		
#ifdef _POWER_MPQP
                MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
                old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
		error = que_command(p_acb, p_cmd_blk, NULL, 0, sleep_flag);
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

	    }
	    break;

	case SCC_DIAG_SETUP:
	    {

		unsigned char		c_len = 7;	/* length of write */
		volatile t_usr_cmd	*p_scc_cmd;	/* pointer to user's
							   command block */
		p_scc_cmd = (t_usr_cmd *) p_usr_cmd;
		bcopy( p_scc_cmd->data, 
			p_cmd_blk->u_data_area.d_ovl.data, c_len);

		/* send that command down */
#ifdef _POWER_MPQP
                MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
                old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
		error = que_command( p_acb, p_cmd_blk, NULL, 0, sleep_flag );
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

	    }
	    break;

	case SER_REG_ACC:		/* Serial Register Access */
	    {
		volatile t_usr_cmd	*p_scc_cmd;	/* pointer to user's
							   command block */

		p_scc_cmd = (t_usr_cmd *) p_usr_cmd;
		bcopy( p_scc_cmd->data, 
			p_cmd_blk->u_data_area.d_ovl.data, 48);
		/* send that command down */
#ifdef _POWER_MPQP
                MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
                old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
		error = que_command(p_acb, p_cmd_blk, NULL, 0, sleep_flag);
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

	    }
	    break;

	case IO_READ:			/* IO Write */
	case IO_WRITE:			/* IO Write */
	{
	    unsigned short	*p_addr;
	    unsigned char	mode;

	    mode = p_usr_cmd->u_data_area.d_ovl.data[0];
	    PIO_PUTC(&(p_cmd_blk->u_data_area.d_ovl.data[0]), mode);
	    p_addr = (unsigned short *)&(p_usr_cmd->u_data_area.d_ovl.data[2]);
	    PIO_PUTSR(&(p_cmd_blk->u_data_area.d_ovl.data[2]), *p_addr++);
	    if /* mode says short */
	    (!mode)
	    {
		PIO_PUTC(&(p_cmd_blk->u_data_area.d_ovl.data[4]),
			    p_usr_cmd->u_data_area.d_ovl.data[4]);
	    }
	    else
	    {
		PIO_PUTSR(&(p_cmd_blk->u_data_area.d_ovl.data[4]), p_addr);
	    }
		
	    /* send that command down */
#ifdef _POWER_MPQP
            MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
            old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */

	    error = que_command( p_acb, p_cmd_blk, NULL, 0, sleep_flag);

#ifdef _POWER_MPQP
            MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
            i_enable(old_pri);
#endif /* _POWER_MPQP */

	    break;
	}

	/* End	of Adapter Setup/Status Commands */

	/* Begin Port Diagnostics Commands */

	case QURY_MDM_INT:	/* Query Modem Interrupts */
	case DUSCC_REG_QRY:	/* DUSCC Register Query */
	case CIO_REG_QRY:	/* CIO Register Query */
	    {
		/* set up the control field */
		PIO_PUTC(&(p_cmd_blk->u_data_area.d_ovl.data[0]),
			   short_pn);
		/* send that command down */
#ifdef _POWER_MPQP
                MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
                old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
		error = que_command( p_acb, p_cmd_blk, NULL, 0, sleep_flag);
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */
	    }
	    break;

	case DMA_REG_QRY:	/* DMA Register Query */
	    {

		/* set up the channel field */
		PIO_PUTC(&(p_cmd_blk->u_data_area.d_ovl.data[0]),
			   p_usr_cmd->u_data_area.d_ovl.data[0]);

		/* send that command down */
#ifdef _POWER_MPQP
                MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
                old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
		error = que_command( p_acb, p_cmd_blk, NULL, 0, sleep_flag);
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

	    }
	    break;

	/* End of Port Diagnostics Commands */

	case SET_PARAM:		/* Set Parameters	*/
	case STRT_AUTO_RSP:	/* Start Auto Response	*/
	case STOP_AUTO_RSP:	/* Stop Auto Response	*/
	    {
		/* send that command down */
#ifdef _POWER_MPQP
                MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
                old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
		error = que_command( p_acb, p_cmd_blk, NULL, 0, sleep_flag);
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

		/* simply get a response */
	    }
	    break;

	case START_PORT:
	    {
		p_cmd_blk->port_nmbr = port_num;
		/* send that command down */
#ifdef _POWER_MPQP
                MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
                old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
		error = que_command( p_acb, p_cmd_blk, NULL, 0, sleep_flag);
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

		/* simply get a response */
	    }	 

	case DIAG_WRITE:
	    {

		unsigned char		c_len;		/* length of write */
		volatile t_write_ioc	*p_usr_wcmd;	/* pointer to user's
							   command block */
		/* first we set up the write by filling the command block */
		/* for a transmit short...including command type, sequence */
		/* number, length, control byte and actual data */

		PIO_PUTC(&(p_cmd_blk->cmd_typ), XMIT_SHORT);

		PIO_PUTSR(&(p_cmd_blk->seq_num), ++p_dds->dds_wrk.cmd_seq_num);

		p_usr_wcmd = (t_write_ioc *) p_usr_cmd;

		c_len = (unsigned char)p_usr_wcmd->len;	/* get length of write*/

		PIO_PUTC(&(p_cmd_blk->lngth), c_len);

		bcopy( p_usr_wcmd->data, 
			p_cmd_blk->u_data_area.d_ovl.data, c_len );

		/* send that command down */

#ifdef _POWER_MPQP
                MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
                old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
		error = que_command(p_acb, p_cmd_blk, NULL, 0, sleep_flag);
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

	    }
	    break;
	default:
	    MPQTRACE1("Ipc6");
	    return(EINVAL);
	    break;
    }	/* end of switch based on diagnostic command type */

    MPQTRACE2("IpcX", error);

    if ( devflag & DNDELAY )
    {
	/* if out of command blocks */
	if ( error )
		return( EIO );
    }
    else
    { 
	/* if out of command blocks */
	if ( error )
		return( EAGAIN );
    }
    return(error);
}
 
/*
 * mem_rw - this function is called from the MP_WMEM and MP_RMEM ioctl
 *	    cmd cases.	mem_rw will transfer information between the
 *	    user-supplied buffer and adapter memory based on information
 *	    contained in the rw_cmd structure pointed to by p_rw_cmd.
 */

int mem_rw( t_acb		*p_acb,
	    int			cmd,
	    t_rw_cmd		*p_rw_cmd,
	    unsigned int	iob,
	    unsigned int	memb )

{
    register char	*lcl_src;	/* local source pointer	     */
    register int	rc = 0;		/* local return code */
#ifdef _POWER_MPQP
    int                 old_pri;
#endif /* _POWER_MPQP */


    MPQTRACE5("ImrE", cmd, p_rw_cmd, iob, memb);

    if	/* for mem_write, only one port may be open */
       ((cmd == MP_WMEM) && (p_acb->n_open_ports > 1))
    {
	MPQTRACE3("ImrS", p_acb, p_acb->n_open_ports);
	return(EINVAL);
    }

    /* allocate and pin a kernel memory buffer */
    MPQTRACE3("ImrM", p_rw_cmd->length, p_rw_cmd);

    if ((lcl_src = xmalloc(p_rw_cmd->length,2,pinned_heap)) == NULL) {
	    MPQTRACE1("ImrU");
	    return(ENOMEM);
    }

    /* prepare for cross memory operations */


		
    if /* operation is write from user memory */
       ( cmd == MP_WMEM)
    {
	rc = copyin(p_rw_cmd->usr_buf, lcl_src, p_rw_cmd->length);
	if /* memory read failed */
	   ( rc != 0)
	{
	    xmfree(lcl_src, pinned_heap);
	    MPQTRACE1("ENO5");
	    return ( ENOMEM );
	}
#ifdef _POWER_MPQP
	MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */
	bus_copyout( p_acb, lcl_src, p_rw_cmd->mem_off >> 16, 
			p_rw_cmd->mem_off, p_rw_cmd->length );
#ifdef _POWER_MPQP
	MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */
    }
    else  /* operation is a read from adapter to user memory */
    {
#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */
	bus_copyin( p_acb, p_rw_cmd->mem_off >> 16, 
			p_rw_cmd->mem_off, lcl_src, p_rw_cmd->length );
#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */

	rc = copyout(lcl_src, p_rw_cmd->usr_buf, p_rw_cmd->length);
	if /* memory read failed */
	   ( rc != 0 )
	{
	    xmfree(lcl_src, pinned_heap);
	    MPQTRACE1("ENO6");
	    return ( ENOMEM );
	}
    }
    xmfree(lcl_src, pinned_heap);
    return (rc);
}
 
/*
 * xfer_edrr - this function will check the response queue element
 *	       to get the port number and then transfer the contents of
 *	       the edrr for that port into the user's edrr area.
 */

int xfer_edrr ( t_acb		*p_acb,
		unsigned long	rqe_elt,
		char		*p_usr_edrr )

{
    volatile unsigned long	bus_sr;
    unsigned char	*p_adrr;	/* Adapter Response Region */
    unsigned int	rqe_port;	/* Response Queue Element Port Num */
    unsigned char	t_char;		/* Temporary Buffer */


    MPQTRACE4("IxeE", p_acb, rqe_elt, p_usr_edrr);

    /* go get the returned response queue element */
    /* and break out its constituant bits */

    rqe_port = (rqe_elt >> 24) & 0x0f;
    bus_sr = BUSIO_ATT(p_acb->io_segreg_val, 0);
    p_adrr = p_acb->p_edrr[rqe_port];
    p_adrr = (unsigned long)p_adrr | bus_sr;
    SET_CPUPAGE( p_acb, bus_sr, p_acb->ds_base_page);
    t_char = PIO_GETC(p_adrr++);
    *p_usr_edrr = t_char;
    p_adrr++;
    p_usr_edrr++;

    if /* any results in the edrr */
       ( t_char )
    {
	PIO_GETSTR(p_usr_edrr, p_adrr, t_char);
    }
    BUSIO_DET(bus_sr);
    return;
}
 
/*-------------------------  I N I T _ R E C V   -----------------------*/
/*                                                                      */
/*  NAME: init_recv                                                     */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Sets up the receive map table with d_mastered mbufs that	*/
/*	are registered with the adapter.				*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	This function runs only at process level.			*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters the adapter receive map table.				*/
/*                                                                      */
/*  RETURNS: FALSE if an error occurs.                                   */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  m_reg, load_recv_map			*/
/*                                                                      */
/*----------------------------------------------------------------------*/

init_recv ( 
	t_acb		*p_acb, 
	t_mpqp_dds 	*p_dds ) 
{
    	volatile char	*p_bus_mem;	/* bus memory address */
    	register int	i;		/* receive map index */
	adap_cmd_t	adapcmd;	/* adapter command */
	int		old_pri;


    	MPQTRACE3( "Irec", p_acb, p_dds );

    	/*  Calculate the beginning bus memory address for receive 	*/
    	/*  tcws; start after the last transmit TCW.			*/

    	p_bus_mem = (unsigned int)p_acb->dma_base +
		                (NUM_XMIT_TCWS * PAGESIZE);

	/*  Register the amount of mbufs needed and expected:		*/

	p_acb->mbuf_req.initial_mbuf 	= NUM_RECV_TCWS;
	p_acb->mbuf_req.initial_clust	= NUM_RECV_TCWS;
	p_acb->mbuf_req.low_mbuf	= 1;
	p_acb->mbuf_req.low_clust	= 1;
	m_reg( &p_acb->mbuf_req );

	/*  Make system memory available to the adapter for DMA 	*/
	/*  operations.							*/

	d_unmask( p_acb->dma_channel_id );

	/*  Allocate an mbuf and d_mastered TCW for each receive map	*/
	/*  element.  Sleep for resources if necessary.			*/

    	for ( i = 0; i < NUM_RECV_TCWS; i++ )
    	{
	    if (RECVMAP[ i ].p_mbuf) {
		m_freem(RECVMAP[ i ].p_mbuf);
		RECVMAP[ i ].p_mbuf = 0;
	    }
	    RECVMAP[ i ].bus_addr = p_bus_mem;
	    if ( !load_recv_map( p_acb, p_dds, i, FALSE ))
		return( FALSE );		/* quit if error */
	    p_bus_mem += (int) PAGESIZE;
        }

	/*  Issue command to the adapter to begin frame reception.	*/

    	MPQTRACE2( "Srec", p_acb );
	p_acb->recv_enabled = TRUE;		/* enable receive */

	/*  Issue a command to the adapter to start frame reception.	*/

	bzero( &adapcmd, sizeof( adap_cmd_t ));
	adapcmd.type = START_RECV;
#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
        old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
	que_command( p_acb, &adapcmd, NULL, 0, FALSE );
#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
        i_enable(old_pri);
#endif /* _POWER_MPQP */

        return ( TRUE );
}


/*-----------------------  F R E E _ R E C V  --------------------------*/
/*                                                                      */
/*  NAME: free_recv                                                     */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Frees up the receive map table resources (mbufs, recv tcws).	*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	This function runs only at process level.			*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters the adapter receive map table.				*/
/*                                                                      */
/*  RETURNS: FALSE if error occurs.                                     */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  m_dereg, d_complete, m_free		*/
/*                                                                      */
/*----------------------------------------------------------------------*/

free_recv ( 
	t_acb		*p_acb,
 	t_mpqp_dds 	*p_dds )
{
	register int	i;
	struct mbuf	*m;


    	MPQTRACE3( "Frec", p_acb, p_dds );

    	for ( i = 0; i < NUM_RECV_TCWS; i++ )
    	{
	    /*  D_complete the mbuf associated with this receive map 	*/
	    /*  and clear it's dma address from the map.		*/
	
	    if ( RECVMAP[ i ].p_data )
	    {
	    	d_complete( p_acb->dma_channel_id, DMA_READ,
			RECVMAP[ i ].p_data, CLBYTES, 
			M_XMEMD( RECVMAP[ i ].p_mbuf ),
			RECVMAP[ i ].bus_addr );

		RECVMAP[ i ].p_data = NULL;
	    }
        }
	/*  Prevent the adapter from completing spurious DMA transfers	*/
	/*  to mbufs that have been freed back to the system.		*/

	d_mask( p_acb->dma_channel_id );

    	for ( i = 0; i < NUM_RECV_TCWS; i++ )
    	{
	    /*  Clear the mbuf from the map before freeing it back	*/
	    /*  to the system.						*/
	
	    if ( RECVMAP[ i ].p_mbuf )
	    {
		m = RECVMAP[ i ].p_mbuf;
		RECVMAP[ i ].p_mbuf = NULL;
		MBUFTRACE2( "Frmb", m );	/* no mbuf available */
	    	m_freem( m );
	    }
        }
	/*  If not already deregistered, deregister mbuf usage:	*/

	if ( p_acb->mbuf_req.initial_mbuf )
	{
	    m_dereg( &p_acb->mbuf_req );
	    p_acb->mbuf_req.initial_mbuf = 0;
	}

        /* free up pending read list of mbufs */
        free_rcv_list( p_dds );

        return(TRUE);
}

/*---------------------  D I S A B L E _ R E C V  ----------------------*/
/*                                                                      */
/*  NAME: disable_recv                                                  */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Prevents receive interrupt processing and deregisters		*/
/*	receive mbuf usage.						*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	This function runs only at process level.			*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters recv_enabled.						*/
/*                                                                      */
/*  RETURNS: FALSE if error occurs.                                     */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  bzero, que_command 			*/
/*                                                                      */
/*----------------------------------------------------------------------*/

disable_recv ( 
	t_acb		*p_acb )
{
	register int	i;
	struct mbuf	*m;
	adap_cmd_t	adapcmd;		/* adapter command */
	int		old_pri;


    	MPQTRACE2( "Drec", p_acb );
	p_acb->recv_enabled = FALSE;		/* disable receive */

	/*  Issue a command to the adapter to halt frame reception;	*/
	/*  this causes the adapter to release all recv TCW's.		*/

	bzero( &adapcmd, sizeof( adap_cmd_t ));
	adapcmd.type = HALT_RECV;
#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
        old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */
	que_command( p_acb, &adapcmd, NULL, 0, FALSE );
#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
        i_enable(old_pri);
#endif /* _POWER_MPQP */

        return(TRUE);
}


/*-------------------------  I N I T _ X M I T   -----------------------*/
/*                                                                      */
/*  NAME: init_xmit                                                     */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Sets up the transmit map table with bus addresses for use in	*/
/*	transmit DMA operations to the adapter.				*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	This function runs only at process level.			*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters the port transmit map table.				*/
/*                                                                      */
/*  RETURNS: Nothing        			                        */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  None					*/
/*                                                                      */
/*----------------------------------------------------------------------*/

init_xmit ( 
	t_acb		*p_acb, 
	t_mpqp_dds 	*p_dds ) 
{
    	volatile char	*p_bus_mem;	/* bus memory address */
    	register int	i;		/* transmit map index */
#ifdef _POWER_MPQP
    	int             old_pri;
#endif /* _POWER_MPQP */


    	MPQTRACE3( "Ixmi", p_acb, p_dds );

    	/*  Calculate the beginning bus memory address for transmit. 	*/

    	p_bus_mem = (unsigned int)p_acb->dma_base +
		                (NUM_PORT_TCWS * DVC.port_num * PAGESIZE);

	/*  Allocate a TCW for each transmit map in the table, clear	*/
	/*  all other fields of the transmit map.			*/

    	for ( i = 0; i < NUM_PORT_TCWS; i++ )
    	{
	    XMITMAP[ i ].map_free = TRUE;
	    XMITMAP[ i ].m        = NULL;
	    XMITMAP[ i ].dma_addr = NULL;
	    XMITMAP[ i ].flags    = NULL;
	    XMITMAP[ i ].bus_addr = p_bus_mem;
	    p_bus_mem += (int) PAGESIZE;
        }
	WRK.xmit_enabled = TRUE;		/* enable transmits */
        return;
}

/*-----------------------  F R E E _ X M I T  --------------------------*/
/*                                                                      */
/*  NAME: free_xmit                                                     */
/*                                                                      */
/*  FUNCTION:                                                           */
/*	Frees up the transmit map table resources.			*/
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*	This function runs only at process level.			*/
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*	Alters the port transmit map table.				*/
/*                                                                      */
/*  RETURNS: Nothing.   		                                */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  free_xmit_map				*/
/*                                                                      */
/*----------------------------------------------------------------------*/

free_xmit ( 
	t_acb		*p_acb,
 	t_mpqp_dds 	*p_dds )
{
	register int	i;

    	MPQTRACE3( "Fxmi", p_acb, p_dds );

	WRK.xmit_enabled = FALSE;		/* disable transmits */
    	for ( i = 0; i < NUM_PORT_TCWS; i++ )	/* free resources */
	    free_xmit_map( p_acb, p_dds, i );

	return;
}

/*--------------------  F R E E _ R C V _ L I S T  ---------------------*/
/*                                                                      */
/*  NAME: free_rcv_list                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*        Free up the pending read list of mbufs.                       */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      This function runs only at process level.                       */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*      Free ups the mbufs associated with each seq_que element.        */
/*                                                                      */
/*  RETURNS: none.                                                      */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:  none.                                  */
/*                                                                      */
/*----------------------------------------------------------------------*/

free_rcv_list (
        t_mpqp_dds      *p_dds )

{
	t_chan_info      *p_tmp_chinfo;  /* pointer to channel information ds */
	t_sel_que        *p_tmp_sel_que; /* pointer to a select queue element */
	int              old_pri;        /* interrupt level	*/
        struct mbuf      *mbuf;

    	MPQTRACE2( "FrqE", p_dds );

	/* find the channel info pointer...right now */
	/* there is only one channel per port allowed */
	/* so the channel structure index is always zero */

	p_tmp_chinfo = p_dds->dds_wrk.p_chan_info[0];

        if /* device was opened by a kernel llc, then return */
        ( p_tmp_chinfo->devflag & DKERNEL )
            return; 

	/* disable interrupts until we finish with queue */
#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
	old_pri = i_disable(INTOFFL3);
#endif /* _POWER_MPQP */


	/* get pointer to first available block */
	p_tmp_sel_que = p_tmp_chinfo->p_rcv_head;

    	MPQTRACE2( "Frq1", p_tmp_sel_que);

	while (p_tmp_sel_que != (t_sel_que *)NULL)
	{
		mbuf = (struct mbuf *) p_tmp_sel_que->stat_block.option[0];

		if (mbuf != (struct mbuf *)NULL)
		{

    	          MBUFTRACE2( "Frq2", mbuf);

		  /* free mbuf associated with that link */
		  m_freem( (struct mbuf *) mbuf );

		  /* null out the free-upped mbuf */
		  p_tmp_sel_que->stat_block.option[0] = (struct mbuf *) NULL;
		}

		/* get next link */
		p_tmp_sel_que = p_tmp_sel_que->p_sel_que;
	}

	/* enable interrupts */
#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
        i_enable(old_pri);
#endif /* _POWER_MPQP */


    	MPQTRACE2( "FrqX", p_tmp_sel_que);
	return;
}
 
/* This routine will handle a timer that expires when a pending HALT has  */
/* not completed.							  */

void halt_pend_timer(
        t_mpqp_dds *p_dds)
{
	MPQTRACE2("TMR1",WRK.halt_sleep_event);
	e_wakeup((void *)&p_dds->dds_wrk.halt_sleep_event);

	return;
}
