static char sccsid[] = "@(#)77	1.53  src/bos/kernext/mpqp/mpqiocfn.c, sysxmpqp, bos411, 9435D411a 9/2/94 09:21:26";

/*
 * COMPONENT_NAME: (MPQP) Multiprotocol Quad Port Device Driver
 *
 * FUNCTIONS: mpqiocfn
 *            ioctl functions
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
#include <fcntl.h>
#include <sys/adspace.h>
#include <sys/ddtrace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/intr.h>
#include <sys/malloc.h>
#include <sys/mpqp.h>
#include <sys/mpqpdd.h>
#ifdef _POWER_MPQP
#include "mpqpxdec.h"
#endif /* _POWER_MPQP */
#include <sys/mpqpdiag.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/xmem.h>
#include <sys/device.h>
/***************************************************************************
 *	External Function Declarations					   *
 ***************************************************************************/

extern int que_command( t_acb	*,
			unsigned char	*,
			unsigned char	*,
			unsigned int,
			unsigned int );  /* send cmd to adapter */

/*
 * NAME: adapt_query
 *
 * FUNCTION: adapt_query collects port trace information and adapter
 *	   synchronization information for return to the caller
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:) This procedure will collect all the adapter information
 *	  like port traces commands for all ports of the adapter
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *	    If there is no error, 0 will be returned.
 *	    EIO	      :	 cannot allocate memory.
 *	    EFAULT    :	 copyout fails.
 *
 *
 */

int adapt_query ( char	 *p_usr,
		  t_acb	 *p_acb,
		  dev_t	 devno,
		  int	 devflag)


{   /* beginning proc. adapt_query  */

	int			count;		/* size of user buffer */
	int			error;		/* error return code */
	char			*p_ker_buf;	/* kernel buffer */
	char			*p_temp;	/* temporary pointer */
	int			rc;		/* cross memory return code */
	int			old_pri;

	error = rc = 0;				/* init. to zero */
	/* log a trace hook */
	MPQTRACE5("IadE", devno, *p_usr, p_acb, devflag);

	if	/* cannot allocate the memory needed */
		((p_ker_buf = xmalloc ( 1280 , 2, pinned_heap )) == NULL )
	{
		/* log a trace hook */
		MPQTRACE1("Iax1");
		error = EIO;
		return (error);
	}

#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */

	bus_copyin( p_acb, p_acb->ds_base_page, 
				p_acb->p_adap_trc_data - p_acb->mem_base, 
							p_ker_buf, 256 );

	/* move pointer to point at destination for port trace queue */
	p_ker_buf += NUM_TRACE_ELEM;

	bus_copyin( p_acb, p_acb->ds_base_page, 
				p_acb->p_adap_trc_data - p_acb->mem_base, 
							p_ker_buf, 1280 );
#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE( old_pri, &mpqp_intr_lock );
#endif /* _POWER_MPQP */

	p_temp = p_ker_buf;

	count = 1280;	   /* number of bytes to copy from  */

	/* copy from kernel memory to caller's space  */
	if ( devflag & DKERNEL)
	{
		bcopy (p_temp, p_usr, count);
	}
	else
	{
		rc = copyout (p_temp, p_usr, count);
	        if ( rc != 0)			/*  copyout failed  */	
	        {
		   xmfree (p_temp , pinned_heap);
		   MPQTRACE4("IrcX", rc, error, devno);
		   return (EFAULT) ;
	        }

	}
	xmfree (p_temp , pinned_heap);

	MPQTRACE4("Irc1", rc, error, devno);
	return (error) ;

} /* end of procedure adapt_query  */

/*
 * NAME:	change_parms
 *
 * FUNCTION:	The change_parms changes various parameters including:
 *		      1. receive timer.
 *		      2. Select address.
 *		      3. Poll address.
 *	      These parameters are copied into dds and device driver
 *	      will send a setparm command down to the adapter
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 */


int change_parms ( t_chg_parms	  *p_new_parm,
			   unsigned char	port_num,
			   t_acb		*p_acb,
			   t_mpqp_dds		*p_dds,
			   int			chan,
			   dev_t		devno,
			   unsigned int         sleep_flag)

{   /* start of change parameters */

	int			error = 0;	/* return code */
	int			chg_mask;	/* mask for parameters */
	unsigned char		field_select;	/* field select byte */
	int			parm1, parm2, parm3;
	register char		*p_parm_cmd;	/* pointer to parms in acb */
	int			rc;		/* generic return code */
	t_adap_cmd		tmp_lcl_cmd;
	int			old_pri;

	sleep_flag = 0;
	bzero(( char *)&tmp_lcl_cmd, sizeof(t_adap_cmd));

	parm1 = (int) p_new_parm->rcv_timer;
	parm2 = (int) p_new_parm->poll_addr;
	parm3 = (int) p_new_parm->select_addr;

	MPQTRACE5("IchE", devno, parm1, parm2, parm3);

	field_select = FIELD_SELECT_RESET;
	/* Turn off the bit of (receive timer), poll addr and select address  */

	chg_mask = p_new_parm->chg_mask;

	
	if (chg_mask & FS_RCV_TMR) /* receive timer needs to be changed */
	{
		p_dds->dds_wrk.rcv_timeout = p_new_parm->rcv_timer;
		field_select = field_select | FS_RCV_TMR;
	}

	if (chg_mask & FS_POLL_ADDR) /* poll address needs to be changed */
	{
		p_dds->dds_wrk.poll_addr = p_new_parm->poll_addr;
		field_select = field_select | FS_POLL_ADDR;
	}

	if (chg_mask & FS_SEL_ADDR) /* select address needs changed */
	{
		p_dds->dds_wrk.select_addr = p_new_parm->select_addr;
		field_select = field_select | FS_SEL_ADDR;
	}
	p_dds->dds_wrk.field_select = field_select;

	/* begin of set parameters	 */
	/* point to the current adapter command block in the adapter */

	tmp_lcl_cmd.cmd_typ = CHG_PARAM;	/* set the command type */
	tmp_lcl_cmd.port_nmbr = port_num;

	/* set up pointer points to the place where parameters are */

	p_parm_cmd = ( char *) &( tmp_lcl_cmd.u_data_area.d_ovl.data[0]) ;

	/* copy all parameter from LLC to adapter command block */
	/* point to the FIELD SELECT byte   */

	*p_parm_cmd++ = field_select;		/* field select byte	 */

	*p_parm_cmd++ = p_dds->dds_wrk.modem_intr_mask; /* modem intr. mask  */
	*p_parm_cmd++ = p_dds->dds_wrk.phys_link;    /* physical link	     */
	*p_parm_cmd++ = p_dds->dds_wrk.poll_addr;    /* poll address	     */
	*p_parm_cmd++ = p_dds->dds_wrk.select_addr;  /* select address	     */
	*p_parm_cmd++ = p_dds->dds_wrk.baud_rate;    /*	 baud_rate	     */
	*p_parm_cmd++ = p_dds->dds_wrk.dial_proto;   /* auto dial protocol   */
	*p_parm_cmd++ = p_dds->dds_wrk.dial_flags;   /* auto dial flags	     */
	*p_parm_cmd++ = p_dds->dds_wrk.data_proto;   /* data transfer prtocol*/
	*p_parm_cmd++ = p_dds->dds_wrk.data_flags;   /* protocol  flags	     */
	PIO_PUTSR(p_parm_cmd, p_new_parm->rcv_timer);

#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
        old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */

	/* enque command to adapter */
	rc = que_command(p_acb, &tmp_lcl_cmd, NULL, 0, sleep_flag); 

#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
        i_enable(old_pri);
#endif /* _POWER_MPQP */

	if ( rc != 0)
	{
		error = EIO;
	}

	MPQTRACE2("IcpX", error);
	return (error);
	/* end of change parameters	*/

}

/*
 * NAME:	flush_port
 *
 * FUNCTION:
 *	     - All outstanding commands to a port will be
 *	       flushed without dropping the physical links
 *	       This allows the llc software to flush
 *	       pending write commands if a non-permanent
 *	       error is detected
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 */

int flush_port ( unsigned char	port_num,
		 t_acb		*p_acb,
		 t_mpqp_dds	*p_dds,
		 int		chan,
		 dev_t		devno,
		 unsigned int   sleep_flag )

{   /* beginning of flush_port	*/

	t_adap_cmd		tmp_lcl_cmd;	/* command block */
	int			rc;		/* generic return code */
	int			error=0;
	int			old_pri;

	/* log a trace hook */

	MPQTRACE5("IfpE", devno, port_num, p_acb, p_dds);

	sleep_flag = 0;
	bzero(( char *)&tmp_lcl_cmd, sizeof(t_adap_cmd));


	/* pointer to the current adapter command block in the adapter */

	PIO_PUTSR (&(tmp_lcl_cmd.seq_num), ++p_dds->dds_wrk.cmd_seq_num);

	tmp_lcl_cmd.cmd_typ = FLUSH_PORT  ;  /* Q_FLUSH_PORT command */

	tmp_lcl_cmd.port_nmbr = port_num ;   /* set port number */

	/* Issue the flush port command to the adapter	 */

#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
        old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */

	/* enque command to adapter */
	rc = que_command(p_acb, &tmp_lcl_cmd, NULL, 0, sleep_flag); 

#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
        i_enable(old_pri);
#endif /* _POWER_MPQP */

	if ( rc != 0)
	{
		error = EIO;
	}

	MPQTRACE3("IfpX", error, rc);
	return (error) ;	/* good return code */

}  /* end of flush_port module	 */




/*
 * NAME:	query_stats
 *
 * FUNCTION:
 *	      - The query statistics subroutine will fetch statistics
 *		from the adapter and pass them back to the caller of
 *		the get statics ioctl.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *	      EFAULT :	      memory copy fails.
 */

int query_stats ( t_query_parms	*p_arg,
		  t_mpqp_dds	*p_dds,
		  int		devflag,
		  dev_t		devno)

{
	char			*p_ker_buf, *p_temp;
	t_query_parms		*p_query_parms; /* ptr to type query parms   */
	char			*bufptr;	/* local source pointer	     */
	int			error, rc;	/* return code		     */
	boolean			clr_ctrs;	/* clear counter indicator   */

	error = rc = clr_ctrs = 0;		/* initialize to zero */

	/* log a trace hook */
	MPQTRACE5("IqsE", devno, p_arg, p_dds, devflag);

	/* allocate space for the mpqp device dependent query parm structure */
	if ((p_ker_buf = 
	     xmalloc(sizeof(t_query_parms), 2, pinned_heap )) == NULL) {
		MPQTRACE2("Iqs0", sizeof(t_query_parms));
		return(ENOMEM);
	}
		
	/* copy the setting by the application to allocated space */
	p_query_parms = ( t_query_parms *) p_ker_buf;
	if ( devflag & DKERNEL )		/* KERNEL process? */ 
	{
	   bcopy ( p_arg, p_query_parms, sizeof(t_query_parms));
	}
	else 
	{
	   rc = copyin ( p_arg, p_query_parms, sizeof(t_query_parms));
	   if ( rc != 0 ) 				/* copyin failed? */
	   {
	      xmfree (p_ker_buf, pinned_heap);
	      MPQTRACE2("Iqs1", rc);
	      return (EFAULT);
	   }
	}

	/* allocate space for the cio stats structure */
	if ((bufptr = xmalloc(sizeof(t_cio_stats), 2, pinned_heap)) == NULL) {
		MPQTRACE2("Iqs2", sizeof(t_cio_stats));
		xmfree(p_ker_buf, pinned_heap);
		return(ENOMEM);
	}
	p_query_parms->bufptr = (t_cio_stats *) bufptr;
	p_query_parms->buflen  = sizeof(t_cio_stats);

	rc = 0;					/* clear return code */
	/* get the clear counter value */
	if ( devflag & DKERNEL ) 		/* KERNEL process? */
	{
	   bcopy( &(p_arg->bufptr->clear_counters), &clr_ctrs, 
	            sizeof(boolean));
	}
	else 
	{
	   rc = copyin( &(p_arg->bufptr->clear_counters),&clr_ctrs,
	                  sizeof(boolean));
	}

	/* copy the DDS stats to user memory */
	if ( !(rc) ) 
	{
	   if ( devflag & DKERNEL ) 		/* KERNEL process? */
	   {
	      MPQTRACE4("Iqs3",&DDS_STATS, p_arg->bufptr, p_arg);
	      bcopy(&DDS_STATS, p_arg->bufptr, sizeof(t_cio_stats));
	   }
	   else 
	   {
	      MPQTRACE4("Iqs4", &DDS_STATS, p_arg->bufptr, p_arg);
	      rc = copyout(&DDS_STATS, p_arg->bufptr, sizeof(t_cio_stats));
	   }

	   /* clear counters if requested by the application. */
	   if (clr_ctrs)
	   {
	      MPQTRACE1( "Iqcc" );
	      bzero( (char *)&DDS_STATS, sizeof (t_cio_stats) );
	   }
	}

	if ( rc ) 
	   error = EFAULT;

	/* free up allocated space */
	xmfree (p_ker_buf, pinned_heap );
	xmfree (bufptr, pinned_heap );

	MPQTRACE3("IqsX", rc, error);
	return(error);
}

/*
 * NAME:	start_dev
 *
 * FUNCTION:
 *	   - This procedure will set parameters and  start port
 *	     for data ready to transfer
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *	      All parametes are saved in dds.
 *	      If EINVAL is returned, turn trace on to find out
 *	      suberror of EINVAL.
 *
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *	      EINVAL  : Invalid parameter.
 *
 */


int start_dev ( t_start_dev	*p_start_dev,
		t_acb		*p_acb,
		unsigned char	port_num,
		t_mpqp_dds	*p_dds,
		int		devflag,
		int		chan,
		dev_t		devno,
		unsigned int    sleep_flag)

{   /* start of start device  */

	unsigned short		baud_rate;	/* baud rate	    */
	unsigned char		baud_rate_adap; /* baud rate for adapter  */
	unsigned char		field_select;	/* field select */
	int			i, j, k;	/* counter */
	unsigned char		modem_flags;	/* modem flags */
	unsigned char		phys_link;	/* physical link */
	register char		*p_parm_cmd;	/* pointer to parms in acb*/
	unsigned int		*p_parm_cmd_int; /*int pointer to parms in acb*/
	int			rc;		/* generic return code */
	t_adap_cmd		tmp_lcl_cmd;
	adap_cmd_t		lcl_cmd;
	int			baud_rate_val[11]={
				       300, 600, 1050, 1200, 2000,
				       2400, 4800, 9600, 19200, 38400, 39400 };
	unsigned char		baud_rate_conv[11]={
					0x00, 0x06, 0x07, 0x08,
					0x09, 0x0A, 0x0b, 0x0c,
					0x0d, 0x0e, 0x0f};
	char			*p_ker_buf, *s, *t;
	t_adap_dial_data	*p_dial_buf;
	unsigned short		*p_tmp;
	int			num_short;
	char			a, b;
	t_err_threshold	 	applic_thresh;	/* Threshold settings from */
						/* upper layer		   */
	unsigned long		*p_long1; 	/* thresh vals tmp long ptr */
	unsigned long		*p_long2; 	/* thresh vals tmp long ptr */
	int			old_pri;

	bzero((char *)&tmp_lcl_cmd, sizeof(t_adap_cmd));

	/* trace the entry for the routine */

	sleep_flag = 0;

	MPQTRACE4("IstE", (int *)p_start_dev->phys_link, 
                          (int *)p_start_dev->data_flags, 
			  (int *)p_start_dev->baud_rate); 

	field_select = FIELD_SELECT_SET;
	phys_link   = p_start_dev->phys_link;
	modem_flags = p_start_dev->modem_flags;
	baud_rate = p_start_dev->baud_rate;

	if /* physical link parm. is invalid */
		((	phys_link != PHYS_X21) && (phys_link != PHYS_V35)
			&& (phys_link != PHYS_422 ) && (phys_link != PHYS_232)
			&&( phys_link != PHYS_V25)
			&&( phys_link != PHYS_SM))
	{
		MPQTRACE3("IplX", devno, phys_link);
		return (EINVAL);
	}


	if ( port_num != 0 )
	{
		if ( phys_link & PHYS_X21 )	/* physical link is X.21  */
		{
			MPQTRACE3("Ipl1", devno, phys_link);
			return (EINVAL) ;
		}

		else if	 /* physical link is V.35 and port = 2 or 3 */
			(( phys_link & PHYS_V35 ) && ( (port_num == 2)
			   || (port_num == 3)))
		{
			MPQTRACE3("Ipl2", devno, phys_link);
			return (EINVAL) ;
		}

		else if /* physical link is EIA422A and (port =1 or port =3) */
			((	phys_link & PHYS_422 ) && (( port_num == 1)
				|| (port_num == 3)))
		{
			MPQTRACE3("Ipl3", devno, phys_link);
			return (EINVAL) ;
		}
	} /* end if port_num != 0 */

	/* Data protocol must be BSC or SDLC half duplex or full duplex  */
	if (!(p_start_dev->data_proto == DATA_PROTO_BSC))
	   if (!(p_start_dev->data_proto == DATA_PROTO_SDLC_FDX))
	      if (!(p_start_dev->data_proto == DATA_PROTO_SDLC_HDX))
	      {
		 MPQTRACE2("IbsX", p_start_dev->data_proto);
		 return (EINVAL) ;
	      } /* end of checking data_proto */

	/* adjusting baud rate */
	/* The baud rate is for RS232, RS422 and V.35 only */

	if (( baud_rate != 0) && ( phys_link == PHYS_X21))
	{
		MPQTRACE4("Ipl5", devno, baud_rate, phys_link);
		return (EINVAL);
	}

	if /* baud rate is invalid  > 38400 */
	(  baud_rate > BAUD_RATE_MAX )
	{
		MPQTRACE4("Ipl6", devno, baud_rate, phys_link);
		return (EINVAL) ;
	}

	for ( i=0; i<=11; i++)
	{
		if ( baud_rate < baud_rate_val[i] )
		{
			baud_rate_adap = baud_rate_conv[i] ;
			break ;
		}
	}

	/* copy all parameters into the dds */

	p_dds->dds_wrk.field_select = field_select;
	p_dds->dds_wrk.modem_intr_mask = p_start_dev->modem_intr_mask;
	p_dds->dds_wrk.phys_link = p_start_dev->phys_link ;
	p_dds->dds_wrk.dial_proto = p_start_dev->dial_proto;
	p_dds->dds_wrk.dial_flags = p_start_dev->dial_flags;
	p_dds->dds_wrk.data_proto = p_start_dev->data_proto;
	p_dds->dds_wrk.data_flags = p_start_dev->data_flags;
	p_dds->dds_wrk.modem_flags = p_start_dev->modem_flags;
	p_dds->dds_wrk.poll_addr = p_start_dev->poll_addr;
	p_dds->dds_wrk.select_addr = p_start_dev->select_addr;
	p_dds->dds_wrk.baud_rate = baud_rate_adap;
	p_dds->dds_wrk.rcv_timeout = p_start_dev->rcv_timeout;
 
	/*
	 * If the pointer is valid, move threshold structure provided by the
	 * application into temporary threshold area.  Otherwise, zero out the 
	 * structure.
	 *
	 * 5/22/94 - Added temporary threshold area to make thresholds
	 * configurable by 'chdev -l mpqx'.  If both the upper layer appli-
	 * cation (p_start_dev->p_err_threshold) and 'chdev' have non-default 
	 * value, upper layer application has higher priority making this 
	 * change upward compatible.
	*/
	if (p_start_dev->p_err_threshold == NULL) 
	{
	   bzero(( char *)&applic_thresh, sizeof(t_err_threshold));
	}
	else 
	{
	   if ( devflag & DKERNEL )		/* KERNEL process? */ 
	   {
	      bcopy((char *)p_start_dev->p_err_threshold,
	            (char *)&applic_thresh, (int)sizeof(t_err_threshold));
	   }
	   else 
	   {
	      rc = copyin((char *)p_start_dev->p_err_threshold, 
	                  (char *)&applic_thresh, (int)sizeof(t_err_threshold));
	      if ( rc != 0 )			/* copyin failed */
	      {
	         MPQTRACE4("IthX", p_start_dev->p_err_threshold,
	                    &applic_thresh, sizeof(t_err_threshold));
	         return(EFAULT);
	      }
	   }
	}

	/* If values in threshold table from application differ and
	 * values loaded in DDS threshold table differ from defaults,
	 * keep the changes, if both are different, values passed from
	 * application have higher priority.
	 */
	p_long1= (unsigned long *) &(applic_thresh.tx_err_thresh) ; 	
	p_long2= (unsigned long *) &(DDS_THRESH.tx_err_thresh) ; 	
	for (i=1; i < ( sizeof(t_err_threshold)/sizeof(unsigned long)); i++)
	{
	    if ( *p_long1 != *p_long2 )
	    {
		switch (i)
		{
		case 2:
		case 3: 	/* words 2 and 3 are percentages */
		    if ( *p_long1 != THRESHPERCENT ) 
		    	*p_long2 = *p_long1;
		    break;
		default:
		    if ( *p_long1 != ERRTHRESH ) 
		    	*p_long2 = *p_long1;
		    break;
		}
	    }
	}

	/* initialize stats. structure to zero's */
	bzero(( char *)&DDS_STATS, sizeof(t_cio_stats));

	/* get access to bus memory (shared space) */
	tmp_lcl_cmd.cmd_typ = SET_PARAM	 ;	/* set parameters command */
	tmp_lcl_cmd.port_nmbr = port_num ;
	p_parm_cmd = ( char *) &( tmp_lcl_cmd.u_data_area.d_ovl.data[0]) ;
	p_parm_cmd_int = ( unsigned int *) &( tmp_lcl_cmd.u_data_area.d_ovl.data[0]) ;

	/* copy all parameters from LLC to adapter command block */
	/* point to the FIELD SELECT byte   */

	/* if modem interrupt mask is set  */
	if (p_start_dev->modem_intr_mask)
	{
		/* turn the bit of MM on */
		field_select = field_select | FS_MDM_INT_MSK;
	}

	/* turn bit PL, PA, AP, DP, BR on */
	field_select = field_select | 0x3f ;

	*p_parm_cmd++ = field_select;
	/*modem interrrupt mask default to 0*/
	*p_parm_cmd++ = p_start_dev->modem_intr_mask;
	*p_parm_cmd++ = p_start_dev->phys_link;	   /* physical link	    */
	*p_parm_cmd++ = p_start_dev->poll_addr;	   /* poll address	    */
	*p_parm_cmd++ = p_start_dev->select_addr;  /* select address	    */
	*p_parm_cmd++ = baud_rate_adap;		   /* baud_rate		    */
	*p_parm_cmd++ = p_start_dev->dial_proto;   /* auto dial		    */
	*p_parm_cmd++ = p_start_dev->dial_flags;   /* dial protocol  flags  */
	*p_parm_cmd++ = p_start_dev->data_proto;   /* data transfer protocol*/
	*p_parm_cmd++ = p_start_dev->data_flags;   /* protocol flags	    */
	*(ushort *)p_parm_cmd++ = SWAPSHORT(p_start_dev->rcv_timeout); 
	MPQTRACE4("Istm", *p_parm_cmd_int++,*p_parm_cmd_int++,*p_parm_cmd_int);

#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
        old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */

	/* enque command to adapter */
	rc = que_command(p_acb, &tmp_lcl_cmd, NULL, 0, sleep_flag); 

#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
        i_enable(old_pri);
#endif /* _POWER_MPQP */

	if ( rc != 0)
	{
	   MPQTRACE4("IqcX", p_acb, tmp_lcl_cmd.cmd_typ, rc);
	   return(EIO);
	}

	bzero(( char *)&lcl_cmd, sizeof(adap_cmd_t));

	/* Begin setting up for Start Port Command to adapter */
	lcl_cmd.type = START_PORT;
	lcl_cmd.port = port_num;

	/* Set Receive Data Transfer Offset (RDTO) value in command block */
	lcl_cmd.cs.start_port.RDTO = SWAPSHORT( p_dds->dds_dvc.rdto );
	/* Set maximum receive buffer size in command block, this is used */
	/* for all physical link types, take care not to overwrite it in  */
	/* Hayes, V.25 BIS and X.21 switched modes.			  */
	lcl_cmd.cs.start_port.max_rx_bsiz =
	    SWAPSHORT( p_dds->dds_dvc.max_rx_bsiz );

	/* if X.21 switched */
	if ((phys_link & PHYS_X21) && ( !(modem_flags & MF_LEASED)) ) {

	    /* copy the data from the user space to kernel space */
	    if ((p_ker_buf = xmalloc(sizeof(t_adap_dial_data), 
				 2, pinned_heap)) == NULL) {
		MPQTRACE2("Ipl7", sizeof(t_adap_dial_data));
		return(ENOMEM);
	    }
	    p_dial_buf = (t_adap_dial_data *) p_ker_buf;

	    if  ( devflag & DKERNEL )		/* Kernel process */
	    {
		bcopy (&p_start_dev->t_dial_data.x21_data,
		       &p_dial_buf->t_dial.x21, sizeof(t_x21_data));
	    }
	    else				/* User process  */
	    {
		rc = copyin (&p_start_dev->t_dial_data.x21_data,
		             &p_dial_buf->t_dial.x21, sizeof(t_x21_data));
	        if ( rc != 0)			/* copyin failed */
	        {
		   xmfree (p_ker_buf, pinned_heap);
		   MPQTRACE2("IbcX",rc);
		   return (EFAULT);
	        }
	    }

	    num_short = sizeof( t_cps_data ) >> 1;
	    p_tmp = (unsigned short *)&p_dial_buf->t_dial.x21.cps;
	    for (i = 1; i < num_short; i++)
		p_tmp[i] = SWAPSHORT( p_tmp[i] );
	    p_dial_buf->t_dial.x21.len =
		 SWAPSHORT( p_dial_buf->t_dial.x21.len );

	    /* Turn on bit saying there is dial data to accompany command */
	    p_start_dev->modem_flags |= MF_DIAL_DATA;

	    /* Put the modem_flags into first byte of the data portion of the */
	    /* command block */ 
	    lcl_cmd.cs.data[0] = p_start_dev->modem_flags;

	    p_dds->dds_dvc.port_state = START_REQUESTED;

#ifdef _POWER_MPQP
            MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
            old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */

	    /* enque command to adapter */
	    rc = que_command( p_acb, &lcl_cmd, 
		p_dial_buf, sizeof( t_adap_dial_data ), sleep_flag );

#ifdef _POWER_MPQP
            MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
            i_enable(old_pri);
#endif /* _POWER_MPQP */

	    if (xmfree(p_ker_buf, pinned_heap) != 0)
	    {
		MPQTRACE2("Ibcf", devno);
		return (EIO);
	    }
	} 	/* end section of X.21 */
	else if ( phys_link & (PHYS_SM | PHYS_V25) ) { 
	    /* smart modem or V.25 BIS dial */
	    /* Turn on bit saying there is dial data to accompany command */
	    p_start_dev->modem_flags |= MF_DIAL_DATA;

	    lcl_cmd.cs.auto_dial.modem_flags = p_start_dev->modem_flags ;
	    lcl_cmd.cs.auto_dial.rdto = SWAPSHORT(p_dds->dds_dvc.rdto);
	    lcl_cmd.cs.auto_dial.connect_timer = 
	        SWAPSHORT(p_start_dev->t_dial_data.auto_data.connect_timer);
	    lcl_cmd.cs.auto_dial.v25b_tx_timer = 
	        SWAPSHORT(p_start_dev->t_dial_data.auto_data.v25b_tx_timer);

	    MPQTRACE5("Iqcs", p_start_dev->t_dial_data.auto_data.len,
		lcl_cmd.cs.auto_dial.modem_flags,
		lcl_cmd.cs.auto_dial.rdto, lcl_cmd.cs.auto_dial.connect_timer);
	    MPQTRACE2("Iqct", lcl_cmd.cs.auto_dial.v25b_tx_timer);

	    /* copy dial string from user space to kernel space */
	    if ((p_ker_buf = xmalloc(SELECT_SIG_LEN, 2, pinned_heap)) == NULL) 
	    {
		MPQTRACE2("Ipl7", sizeof(t_adap_dial_data));
		return(ENOMEM);
	    }

	    if  ( devflag & DKERNEL )		/* Kernel process */
	    {
		bcopy (&p_start_dev->t_dial_data.auto_data.sig,
		        p_ker_buf, SELECT_SIG_LEN);
	    }
	    else				/* User process  */
	    {
		rc = copyin (&p_start_dev->t_dial_data.auto_data.sig,
		              p_ker_buf, SELECT_SIG_LEN);
	        if ( rc != 0)			/* copyin failed */
	        {
	           xmfree (p_ker_buf, pinned_heap);
	           MPQTRACE2("IbcX",rc);
	           return (EFAULT);
	        }
	    }

	    /* Convert digits delimited by '\' to hex control character */
	    j = 0;   s = t = p_ker_buf;
	    for (i=0; i < p_start_dev->t_dial_data.auto_data.len; i++)
	    {
		if (*t != '\\')
		    *s++ = *t++;
		else
		{
		    b = 0;
		    for (k=1; k<=2; k++)
		    {
			/* ascii to hex conversion */
			a = *(t+k);
			if (a>='0' && a<='9') a -= '0';
			else 
			    if (a>='A' && a<='F') a -= 0x37;
			    else 
				if (a>='a' && a<='f') a -= 0x57;
				else
				{
				   *s = *t++;  
				   break;
				}
			b = ((b) << 4) + a;
		    }
		    if (k > 2) /* Control characters were converted */
		    {
			*s = b;  t+=3;  j++;
		    }
		    s++;
		}
	    }

	    /* blank out extra bytes that were converted control chars */
	    for (i=0; i < j*2; i++)
		*s++ = 0;

	    lcl_cmd.length =
		SWAPSHORT(p_start_dev->t_dial_data.auto_data.len - (j*2));

	    s=(unsigned int *)p_ker_buf;
	    MPQTRACE5("Ibuf", *s++, *s++, *s, lcl_cmd.length);

	    p_dds->dds_dvc.port_state = START_REQUESTED;

#ifdef _POWER_MPQP
            MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
            old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */

	    /* enque command to adapter */
	    rc = que_command( p_acb, &lcl_cmd, p_ker_buf, SELECT_SIG_LEN, 
		 sleep_flag );

#ifdef _POWER_MPQP
            MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
            i_enable(old_pri);
#endif /* _POWER_MPQP */

	    if (xmfree(p_ker_buf, pinned_heap) != 0)
	    {
		MPQTRACE2("Iqcf", devno);
		return (EIO);
	    }
	} 	/* end autodial section */
	else
	{
	    /* Put the modem_flags into first byte of the data portion of the */
	    /* command block */ 
	    lcl_cmd.cs.data[0] = p_start_dev->modem_flags;

	    p_dds->dds_dvc.port_state = START_REQUESTED;

#ifdef _POWER_MPQP
            MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
            old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */

	    /* enque command to adapter */
	    rc = que_command(p_acb, &lcl_cmd, NULL, 0, sleep_flag); 

#ifdef _POWER_MPQP
            MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
            i_enable(old_pri);
#endif /* _POWER_MPQP */

	}
	if ( rc )
	{
		MPQTRACE3("Isd1", rc, devno);
		return (EIO);
	}

	/* trace the good return code for the routine */

	MPQTRACE5("IsdX", devno, p_dds->dds_dvc.max_rx_bsiz, 
	    p_dds->dds_dvc.rdto, lcl_cmd.cs.start_port.max_rx_bsiz);

	return ( 0 ) ;				/* good return code . */

} /* end of start_dev	*/


/*
 * NAME:	start_ar
 *
 * FUNCTION:
 *	   - Start auto response
 *	     This procedure will send down the command
 *	     to the adapter that tell the adapter to
 *	     start auto response mode
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *	      ENOTREADY	      : the requested port is not in
 *				data transfer state
 */

int start_ar ( t_start_ar	*p_start_ar,
	       unsigned char	port_num,
	       t_acb		*p_acb,
	       t_mpqp_dds	*p_dds,
	       int		chan,
	       dev_t		devno,
	       unsigned int     sleep_flag)


{   /* beginning of start_ar */

	register char		*p_parm_cmd;	/* pointer to data in acb    */
	register char		*p_temp;	/* pointer to data in acb    */
	int			rc;		/* generic return code */
	t_adap_cmd		tmp_lcl_cmd;	/* local command */
	int			error=0;	/* error return code */
	int			old_pri;

	/* log a trace hook */
	MPQTRACE5("IarE", devno, p_start_ar, port_num, p_dds);

	bzero(( char *)&tmp_lcl_cmd, sizeof(t_adap_cmd));

	sleep_flag = 0;
	/* check the state of the port in dds of that port if the port is
       in data transfer state .If it's then continue , else return error */

	if ( p_dds->dds_dvc.port_state != DATA_XFER )
	{
		/* log a trace hook */
		MPQTRACE2("IsaX", p_dds->dds_dvc.port_state);
		return (ENOTREADY) ;
	}


	/* pointer to the current adapter command block in the adapter */

	tmp_lcl_cmd.cmd_typ = STRT_AUTO_RSP ; /* start auto response command */
	tmp_lcl_cmd.port_nmbr = port_num ; /* store port number in cmd block */
	p_parm_cmd = (char*) &( tmp_lcl_cmd.u_data_area.d_ovl.data[0]) ;
	/* point to 16th byte of adapter command block	*/
	p_temp = (char *) p_start_ar ;
	*p_parm_cmd++ = *p_temp++;     /* set to timer high byte  */
	*p_parm_cmd++ = *p_temp++;     /* set to timer low  byte  */
	*p_parm_cmd++ = *p_temp++;     /* set to Rx/Tx	    byte  */
	*p_parm_cmd++ = *p_temp++;     /* set to Tx control byte  */
	*p_parm_cmd++ = *p_temp++;     /* set to Rx control byte  */

	/* Issue start auto response  command to the adapter   */

#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
        old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */

	/* enque command to adapter */
	rc = que_command(p_acb, &tmp_lcl_cmd, NULL, 0, sleep_flag); 

#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
        i_enable(old_pri);
#endif /* _POWER_MPQP */

	if ( rc != 0)
	{
		error = EIO;
	}

	/* log a trace hook */
	MPQTRACE3("Isa1", error, devno);
	return (error) ;	/* good return code */

}  /* end of start auto response module */

/*
 * NAME:	stop_ar
 *
 * FUNCTION:
 *	  - The stop auto response will send a command to the adapter
 *	    indicating that auto response mode is to be terminated.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *	      ENOTREADY	      : The requested port is not in
 *				DATA TRANSFER state
 */


int stop_ar ( unsigned char	port_num,
	      t_acb		*p_acb,
	      t_mpqp_dds	*p_dds,
	      int		chan,
	      dev_t		devno,
	      unsigned int      sleep_flag)

{   /* beginning of stop_ar */

	int			rc;		/* generic return code */
	t_adap_cmd		tmp_lcl_cmd;	/* temporary local cmd block */
	int			error=0;
	int			old_pri;


	/* log a trace hook */

	MPQTRACE5("IpaE", devno, port_num, p_acb, p_dds);

	bzero(( char *)&tmp_lcl_cmd, sizeof(t_adap_cmd));

	sleep_flag = 0;
	/* check the state of the port in dds of that port if the port is
       in data transfer state .If it's then continue , else return error */

	if ( p_dds->dds_dvc.port_state != DATA_XFER )
	{
		MPQTRACE3("IpaX", p_dds->dds_dvc.port_state, devno);
		return (ENOTREADY) ;
	}

	tmp_lcl_cmd.cmd_typ = STOP_AUTO_RSP ; /* stop auto response command */
	tmp_lcl_cmd.port_nmbr = port_num ;

	/* Issue the stop auto response	 command to the adapter	  */

#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
        old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */

	/* enque command to adapter */
	rc = que_command(p_acb, &tmp_lcl_cmd, NULL, 0, sleep_flag); 

#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
        i_enable(old_pri);
#endif /* _POWER_MPQP */

	if ( rc != 0)
	{
		error = EIO;
	}

	MPQTRACE3("Ips1", rc, devno);
	return (error) ;	/* good return code */

}  /* end of stop auto response module	*/

/*
 * NAME:	trace_off
 *
 * FUNCTION:
 *	    - This procedure will turn the port trace off
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 */

int trace_off( unsigned char	port_num,
	       t_acb		*p_acb,
	       t_mpqp_dds	*p_dds,
	       int		chan,
	       dev_t		devno,
	       unsigned int     sleep_flag )

{   /* beginning of trace_off  */

	unsigned char		cmd_numbr;	/* command block number */
	int			rc;		/* generic return code */
	t_adap_cmd		tmp_lcl_cmd;	/* temporary local command */
	int			error=0;	/* error return code  */
	int			old_pri;

	/* log a trace hook */

	MPQTRACE5("ItoE", devno, port_num, p_acb, p_dds);

	bzero ( ( char *)&tmp_lcl_cmd, sizeof(t_adap_cmd));

	/* 
	** Get the next availble  command block index by 
	** invoking alloc_cpair 
	*/

	tmp_lcl_cmd.cmd_typ = TRACE_DISABL   ; /* turn trace facility off  */
	tmp_lcl_cmd.port_nmbr = port_num ;

	/* Issue the traceoff command to the adapter   */

#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
        old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */

	/* enque command to adapter */
	rc = que_command(p_acb, &tmp_lcl_cmd, NULL, 0, sleep_flag); 

#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
        i_enable(old_pri);
#endif /* _POWER_MPQP */

	if ( rc != 0)
	{
		error = EIO;
	}

	MPQTRACE3("ItoX", error, devno);
	return (error) ;		/* good return code */

}  /* end of trace_off module */



/*
 * NAME:	trace_on
 *
 * FUNCTION:
 *	   -  This procedure will turn the port trace on
 *	      for later used for display the port trace
 *	      trace queue elements
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 */

int trace_on ( unsigned char	port_num,
	       t_acb		*p_acb,
	       t_mpqp_dds	*p_dds,
	       int		chan,
	       dev_t		devno,
	       unsigned int     sleep_flag )

{   /* beginning of trace_on  */

	int			rc;		/* generic return code */
	t_adap_cmd		tmp_lcl_cmd;	/* temporary local command */
	int			error=0;	/* error return code */
	int			old_pri;

	/* log a trace hook */

	MPQTRACE5("ItnE", devno, port_num, p_acb, p_dds);

	bzero ( ( char *)&tmp_lcl_cmd, sizeof(t_adap_cmd));


	tmp_lcl_cmd.cmd_typ = TRACE_ENBL  ; /* trace facility turn on */
	tmp_lcl_cmd.port_nmbr = port_num ;

	/* Issue the traceon command to the adapter   */

#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
        old_pri = i_disable(INTCLASS1);
#endif /* _POWER_MPQP */

	/* enque command to adapter */
	rc = que_command(p_acb, &tmp_lcl_cmd, NULL, 0, sleep_flag); 

#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
        i_enable(old_pri);
#endif /* _POWER_MPQP */

	if ( rc != 0)
	{
		error = EIO;
	}

	MPQTRACE3("ItnX", error, devno);
	return (error) ;	/* good return code */

}  /* end of trace_on module */


/*
 * NAME:	get_stat
 *
 * FUNCTION:
 *	      This procedure will return the status of a
 *	      port. This is the implementation of CIO_GET_STAT
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *	      ENOMSG  : when the status queue is NULL
 */

int get_stat ( t_mpqp_dds	*p_dds,
	       char		*p_usr,
	       dev_t		devno)

{
	unsigned int	old_pri;	/* save old interrupt priority */
	t_chan_info	*p_ch_info;	/* channel info pointer */
	t_sel_que	*p_stat_elem;	/* select queue status elt */
	cio_stat_blk_t	*p_stat_blk;	/* Temporary  pointer	  */
	int		rc=0;		/* return code */
	unsigned int	code;		/* temp. loc. for status code */
	unsigned int	opt0;		/* temp. loc. for option 0 */

	/* log a trace hook */
	MPQTRACE4("IgsE", devno, p_dds, *p_usr);

	/* get pointer to channel information */

	p_ch_info = p_dds->dds_wrk.p_chan_info[0];

	/* now we have to disable the interrupts, then continue */

#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
        old_pri = i_disable(INTOFFL3);
#endif /* _POWER_MPQP */

	/* now check if any entries exist on the status queue */

	if /* status queue pointer is NULL, no entries */
	   (p_ch_info->p_stat_head == (t_sel_que *)NULL)
	{
		MPQTRACE3("Igs2", p_ch_info, devno);
#ifdef _POWER_MPQP
                MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
                i_enable(old_pri);
#endif /* _POWER_MPQP */

		return( ENOMSG) ;
	}

	p_stat_elem = p_ch_info->p_stat_head;

	p_stat_blk = (cio_stat_blk_t *) &(p_stat_elem->stat_block);

        /* reenable interrupts for copyout */
#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
        i_enable(old_pri);
#endif /* _POWER_MPQP */

	/* copy kernel memory to user memory */
	if ( (rc = copyout(p_stat_blk, p_usr, sizeof(cio_stat_blk_t))) )
	{
		MPQTRACE2("Igs3", rc);
		return ( EFAULT );
	}

	/* now we have to disable the interrupts, due to the copyout call */

#ifdef _POWER_MPQP
        MPQP_LOCK_DISABLE(old_pri, &mpqp_intr_lock);
#else
        old_pri = i_disable(INTOFFL3);
#endif /* _POWER_MPQP */


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
	/* to the select queue available chain if the stat. element was */
        /* not a CIO_LOST_STATUS or CIO_ASYNC_STATUS.                */

	p_stat_elem->rqe_value = 0;
	p_stat_elem->stat_block.code = 0;
	p_stat_elem->stat_block.option[0] = 0;
	p_stat_elem->stat_block.option[1] = 0;
	p_stat_elem->stat_block.option[2] = 0;
	p_stat_elem->stat_block.option[3] = 0;

	MPQTRACE3("Igs4", code, opt0);

        switch ( code )
        {
          case CIO_LOST_STATUS:                   /* lost status overflow */
          {
	    p_stat_elem->p_sel_que = (t_sel_que *)NULL;
	    p_ch_info->p_lost_stat = p_stat_elem;
	    break;
          }

          case CIO_ASYNC_STATUS:                  /* lost data from receive */
          {
	    /* make sure if this is a que element for LOST_DATA only */
            if ( opt0 == CIO_LOST_DATA )          
            { 
	      p_stat_elem->p_sel_que = (t_sel_que *)NULL;
	      p_ch_info->p_lost_rcv = p_stat_elem;
	      break;
	    }
          }

          default:                            /* must be one or the other */
	     p_stat_elem->p_sel_que = p_ch_info->p_sel_avail;
	     p_ch_info->p_sel_avail = p_stat_elem;

	     MPQTRACE3("Igs5", p_ch_info->p_sel_avail, code);
        }

#ifdef _POWER_MPQP
        MPQP_UNLOCK_ENABLE(old_pri, &mpqp_intr_lock);
#else
        i_enable(old_pri);
#endif /* _POWER_MPQP */


	MPQTRACE3("IgsX", rc, devno);
	return (0);
} /* get_stat */

/*
 * NAME:	stop_port
 *
 * FUNCTION:
 *	      Procedure:	      stop_port
 *	      Description:	      this procedure will stop a port
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 */

int stop_port ( unsigned char	port_num,
		t_acb		*p_acb,
		t_mpqp_dds	*p_dds,
		int		chan,
		dev_t		devno,
		unsigned int    sleep_flag )

{   /* beginning of stop_port  */

	int			rc;		/* generic return code */
	t_adap_cmd		tmp_lcl_cmd;	/* temporary local cmd block */
	int			error=0;
	int			old_pri;

	/* log a trace hook */
	MPQTRACE5("IspE", devno, port_num, p_acb, p_dds);

	bzero(( char *)&tmp_lcl_cmd, sizeof(t_adap_cmd));

	/* pointer to the current adapter command block in the adapter */

	tmp_lcl_cmd.cmd_typ = STOP_PORT;
	tmp_lcl_cmd.port_nmbr = port_num ;   /* set port number */

	p_dds->dds_dvc.port_state = HALT_REQUESTED;

	/* issue a send command to adapter */

#ifndef _POWER_MPQP
        old_pri = i_disable(INTOFFL3);
#endif /* _POWER_MPQP */

	/* enque command to adapter */
	rc = que_command(p_acb, &tmp_lcl_cmd, NULL, 0, sleep_flag); 

#ifndef _POWER_MPQP
        i_enable(old_pri);
#endif /* _POWER_MPQP */

	if ( rc != 0)
	{
		error = EIO;
	}

	/* set the flag to indicate that a halt has been requested on port */

	MPQTRACE4("IspX", rc, error, devno);
	return (error) ;	/* good return code */

}  /* end of stop_port	*/
