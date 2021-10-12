static char sccsid[] = "@(#)88	1.2  src/bos/kernext/fddidiag/fddiprim_b.c, diagddfddi, bos411, 9428A410j 11/8/93 09:51:48";
/*
 *   COMPONENT_NAME: DIAGDDFDDI
 *
 *   FUNCTIONS: fddi_async_status
 *		fddi_conn_done
 *		fddi_logerr
 *		fddi_pio_retry
 *		fddi_report_status
 *		fddi_trace
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "fddiproto.h"
#include "fddi_comio_errids.h"
#include <sys/poll.h>
#include <sys/trchkid.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/ioacc.h>
/* 
 * get access to the global device driver control block
 */
extern fddi_ctl_t	fddi_ctl;
extern fddi_trace_t	fdditrace;

/*
 * NAME: fddi_report_status()
 *                                                                    
 * FUNCTION: FDDI report status block function.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This executes on the process thread or interrupt thread.
 *                                                                   
 * NOTES:  
 *	This routines will give the specified user the status block
 *	passed in.  If the user is a user-mode process, the status block
 *	will be placed in the user's status block queue.  If the user is
 *	a kernel mode process, the user's stat_fn() will be called with
 *	the passed in status block.
 *	
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: 
 *	For user mode process, a status block will be placed on the 
 *	user's status queue.
 *
 * RETURNS: NONE
 */  

void
fddi_report_status(fddi_acs_t 		*p_acs,
		   fddi_open_t		*p_open,
		   cio_stat_blk_t  	*p_stat)
{

	int 	ipri;

	/* 
	 * serialization with the slih and timer routines.
	 */

	ipri = i_disable(INTOFFL1);
	FDDI_DBTRACE("PrsB", p_acs, p_open, p_stat);

	if (p_open->devflag & DKERNEL)
	{ 
		/* notify the kernel process
		 */
		FDDI_DBTRACE("Prs1", p_open->stat_fn, p_open->open_id, p_stat);
		(*(p_open->stat_fn)) (p_open->open_id, p_stat);
		FDDI_DBTRACE("Prs2", p_open->stat_fn, p_open->open_id, p_stat);
	}
	else
	{
		/* put status block on user's que
		 */
		if ( p_open->stat_in == p_open->stat_out && 
			p_open->stat_cnt > 0)
		{
			if (p_open->stat_ls_cnt == 0)
				p_open->stat_que_ovflw = TRUE;
			++p_acs->ras.ds.stat_que_ovflw;
		}
		else
		{
			/* put status block on the que
			 */
			p_open->p_statq[p_open->stat_in] = *p_stat;
			INCR_STATQ_INDEX(p_acs, p_open->stat_in);
			p_open->stat_cnt++;
			if ( p_open->stat_cnt > p_acs->ras.cc.sta_que_high)
				p_acs->ras.cc.sta_que_high = p_open->stat_cnt;

			/* if the user has an outstanding
			 * select/poll, let the user know via
			 * the selnotify().
			 */
			if ( p_open->selectreq & POLLPRI )
			{
				p_open->selectreq = 0;
				selnotify(p_open->devno, p_open->chan, POLLPRI);
			}
		}
	} /* end else user mode */

	FDDI_DBTRACE("PrsE", p_acs, p_open->stat_que_ovflw, p_open->selectreq);
	i_enable(ipri);

	return;
} /* end fddi_report_status() */

/*
 * NAME: fddi_async_status()
 *                                                                    
 * FUNCTION: FDDI report asynchronous status to all attached users.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This executes on the interrupt thread.
 *                                                                   
 * NOTES:  
 *	This routines will give the all the users attached to this
 *	device (p_acs) the status block passed in.
 *	
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: NONE
 */  

void
fddi_async_status(fddi_acs_t 		*p_acs,
		   cio_stat_blk_t  	*p_stat)
{
	fddi_open_t	*p_open;
	int		opens = 0;
	int 		i = 0; 

	/* go thru all the open elements for 
	 * the whole driver.  Report status to those users
	 * who have the same ACS ptr.
	 */

	FDDI_DBTRACE("PasB", p_acs, p_stat, 0);
	while (opens < p_acs->ctl.open_cnt) 
	{
		p_open = fddi_ctl.p_open_tab [i];
		if ( (p_open != NULL) && (p_open->p_acs == p_acs) )
		{
			fddi_report_status(p_acs, p_open, p_stat);
			++opens;
		}	
		++i;
		
	} /* end while */
	FDDI_DBTRACE("PasE", p_acs->ctl.open_cnt, p_stat->option[0], 
		p_stat->option[1]);
	return;

} /* end fddi_async_status() */

/*
 * NAME: fddi_conn_done()
 *                                                                    
 * FUNCTION: 
 *	FDDI report the completion of the CIO_START to
 *	all users who have a pending start.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This executes on the interrupt thread.
 *                                                                   
 * NOTES:  
 *	This routines will notify all users who have registered
 *	a netid via a CIO_START that the start has completed.
 *	
 *	The START is always successful even if the CONNECT fails. 
 *	If the PHYSICAL connection fails then LIMBO will be entered 
 *	at that time.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: NONE
 */  

void
fddi_conn_done(
	fddi_acs_t *p_acs,
	int	status)
{
	int i=0, cnt=0;
	fddi_open_t	*p_open;
	cio_stat_blk_t 	start_blk;	/* CIO_START_DONE blk */


	FDDI_DBTRACE("PcdB", p_acs, p_acs->ctl.netid_cnt, status);

	start_blk.code = CIO_START_DONE;
	start_blk.option[0] = status;
	start_blk.option[3] = 0;

	/*
	 * copy the source address that the device
	 * is configured with.  This is the address 
	 * that is put into the FORMAC chip on the
	 * adapter.
	 */
	bcopy(&p_acs->ctl.long_src_addr[0], &start_blk.option[2],
		FDDI_NADR_LENGTH);

	/* 
	 * go thru all the netids for this device.
	 * Report Start Done status to those users who 
	 * have netids registered.
	 */
	while ( (i < FDDI_MAX_NETIDS) && (cnt < p_acs->ctl.netid_cnt) )
	{
		/* 
		 * is this netid in use?
		 * if so, send the CIO_START_DONE status block
		 */
		if ( (p_open=p_acs->ctl.p_netids[i]) != NULL )
		{
			start_blk.option[1] = i;
			fddi_report_status(p_acs, p_open, &start_blk);
			++cnt;
		}
		++i;
	}
	FDDI_DBTRACE("PcdE", start_blk.option[1], start_blk.option[2],
		 start_blk.option[3]);
	return;

} /* end fddi_conn_done() */

/*
 * NAME: fddi_pio_retry
 *
 * FUNCTION: This routine is called when a pio routine returns an
 *	exception. It will retry the the PIO and do error logging.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Called by interrupt and processes level
 *	This routine is invoked by the PIO_xxxX routines
 *
 * RETURNS:
 *	0 - exception was retried successfully
 *	exception code of last failure - if not successful
 */

int
fddi_pio_retry (
	fddi_acs_t	*p_acs,	   	/* tells which adapter this came from */
	int 		excpt_code,	/* exception code from original PIO */
	enum pio_func 	iofunc,		/* io function to retry	*/
	void 		*ioaddr,	/* io address of the exception	*/
	long		ioparam,	/* parameter to PIO routine	*/
	int		cnt)		/* for string copies */

{
	int	retry_count = PIO_RETRY_COUNT;
	int	ipri;

	FDDI_DBTRACE("PprB", p_acs, excpt_code, iofunc);

	while (TRUE)
	{
		FDDI_DBTRACE("Ppr1", p_acs, retry_count, excpt_code);
		/* 
		 * log error 
		 */
		p_acs->dev.iox = excpt_code;
		fddi_logerr(p_acs, ERRID_FDDI_PIO, __LINE__, __FILE__);

		if (p_acs->dev.piowall)
		{
			FDDI_DBTRACE("Ppr3", excpt_code, 0, 0);
			return;
		}
		/* 
		 * check if out of retries
		 */
		if (retry_count <= 0)
		{
			FDDI_DBTRACE("Ppr2", p_acs, retry_count, excpt_code);
			/* 
			 * Bugout Condition: 
			 *	must be scheduled in offlevel since we 
			 *	could be in a process or interrupt thread.
			 */
			ipri = i_disable (INTCLASS2);		
			p_acs->dev.oflv_events |= FDDI_DEAD_IO;
			p_acs->dev.piowall = TRUE;
			if (p_acs->dev.oflv_running == FALSE)
			{
				/*
				 * schedule offlevel process/thread
				 */
				i_sched (&p_acs->dev.ihs);
				p_acs->dev.oflv_running = TRUE;
			}
			i_enable (ipri);

			break;
		}
		retry_count--;

		/* 
		 * retry the pio function, return if successful
		 */
		switch (iofunc)
		{
			case PUTC:
				excpt_code = BUS_PUTCX((char *)ioaddr,
							(char)ioparam);
				break;
			case PUTSR:
				excpt_code = BUS_PUTSRX((short *)ioaddr,
							(short)ioparam);
				break;
			case PUTLR:
				excpt_code = BUS_PUTLRX((long *)ioaddr,
							(long)ioparam);
				break;
			case GETC:
				excpt_code = BUS_GETCX((char *)ioaddr,
							(char *)ioparam);
				break;
			case GETSR:
				excpt_code = BUS_GETSRX((short *)ioaddr,
							(short *)ioparam);
				break;
			case GETLR:
				excpt_code = BUS_GETLRX((long *)ioaddr,
							(long *)ioparam);
				break;
			case PUTSTR:
				excpt_code = BUS_PUTSTRX((long *)ioaddr,
							(long *)ioparam,
							cnt);
				break;
			case GETSTR:
				excpt_code = BUS_GETSTRX((long *)ioaddr,
							(long *)ioparam,
							cnt);
				break;
			default:
				ASSERT(0);
		}

		if (excpt_code == 0)
		{
			/* ok */
		 	FDDI_DBTRACE("Ppr3", ioaddr, ioparam, cnt);
			break;
		}

	}
	FDDI_DBTRACE("PprE", ioaddr, ioparam, cnt);
	return (excpt_code);
}

/*
 * NAME: fddi_trace
 *                                                                    
 * FUNCTION: 
 *	This routine puts a trace entry into the internal device
 *	driver trace table.  It also calls the AIX trace service.
 *                                                                    
 * EXECUTION ENVIRONMENT: process or interrupt environment
 *                                                                   
 * NOTES: 
 *	Each trace point is made up of 4 words (ulong).  Each entry
 *	is as follows:
 *
 *		| Footprint | data | data | data |
 *
 *	The 4 byte Footprint is contructed as follows:
 *
 *		byte 1 = Type of operation 
 *		byte 2&3 = function identifier
 *		byte 4 = location identifier
 *
 *	where byte 1 type of operation may be:
 *
 *		"O"	- open
 *		"C"	- close
 *		"M"	- mpx
 *		"T"	- transmit
 *		"R"	- receive
 *		"N"	- Network Recovery Mode 
 *		"A"	- Adapter activation/de-activation
 *		"D"	- diagnostic
 *		"P"	- primitive routine
 *		"I"	- ioctl
 *		"c"	- configuration
 *		"o"	- oflv routines
 *		"S"	- SLIH routine
 *		"t"	- timer routines
 *		"d"	- dump routines
 *
 *	where bytes 2&3 are two characters of the actual function name:
 *
 *	where byte 4 location identifier may be:
 *
 *		"B"	- beginning
 *		"E"	- end
 *		"C"	- trace point continuation
 *		"1-9"	- function trace point number
 *		"a-z"	  may be the characters 1 thru 9 or
 *			  lower case a thru z
 *
 *
 *
 * RECOVERY OPERATION: none
 *
 * DATA STRUCTURES: 
 *	Modifies the global static FDDI trace table.
 *
 * RETURNS:  none
 */  


void
fddi_trace (	register uchar	str[],	/* trace data Footprint */
		register ulong	arg2,	/* trace data */
		register ulong	arg3,	/* trace data */
		register ulong	arg4)	/* trace data */

{
	register int	i;
	register char	*p_str;

	/*
	 * get the current trace point index 
	 */
	i= fdditrace.next;

	p_str = &str[0];

	/*
	 * copy the trace point data into the global trace table.
	 */
	fdditrace.table[i] = *(ulong *)p_str;
	++i;
	fdditrace.table[i] = arg2;
	++i;
	fdditrace.table[i] = arg3;
	++i;
	fdditrace.table[i] = arg4;
	++i;


	if ( i < FDDI_TRACE_SIZE )
	{
		fdditrace.table[i] = 0x21212121;	/* end delimeter */
		fdditrace.next = i;
	}
	else
	{
		fdditrace.table[0] = 0x21212121;	/* end delimeter */
		fdditrace.next = 0;
	}


	/*  Make the AIX trace point call */

	 TRCHKGT(HKWD_DD_FDDIDD | HKTY_GT | 4, *(ulong *)p_str, arg2, 
 			arg3, arg4, 0);

	return;
} /* end fddi_trace */


/*
 * NAME: fddi_logerr()
 *                                                                    
 * FUNCTION: 
 *	This routine logs an error to the system
 *                                                                    
 * EXECUTION ENVIRONMENT: process or interrupt environment
 *                                                                   
 * NOTES: 
 *	This routine will read the POS registers from the adapter
 *	and store them in the temp error log data struct.  These
 *	values willbe logged to the system.  All other error log values
 *	are pulled in from the ACS via the passed in ACS ptr.
 *
 * RECOVERY OPERATION: None
 *
 * DATA STRUCTURES: None
 *
 * RETURNS:  none
 */  

void
fddi_logerr (	fddi_acs_t	*p_acs,
		ulong		errid,
		int		line,
		char		*p_file)
{ 

	int		i;
	fddi_elog_t	log;
	char		errbuf[300];
	int 		iocc;

	FDDI_DBTRACE("PlgB", p_acs, errid, line);

	/*
	 * Store the error id into the log entry
	 */
	log.errhead.error_id = errid;

	/* plug in the resource name for this device */
	strncpy( log.errhead.resource_name, p_acs->dds.lname, ERR_NAMESIZE);

 	for (i=0;i<FDDI_NADR_LENGTH; ++i)
	  	log.src_addr[i] = p_acs->ctl.long_src_addr[i];

 	for (i=0; i<11; ++i)
  		log.stestrc[i] = p_acs->dev.stestrc[i];

	/* plug in the line number and filename */
	sprintf(errbuf, "line: %d file: %s", line, p_file);

	strncpy(log.file, errbuf, (size_t)sizeof(log.file));

	log.nrm = p_acs->ctl.limbo_blk;
	log.ls = p_acs->ras.ls;
	log.mcerr = p_acs->dev.mcerr; 
	log.iox = p_acs->dev.iox;
	log.status2 = p_acs->dev.smt_event_mask;
	log.status3 = p_acs->dev.smt_error_mask;
	log.status1 = p_acs->dev.smt_control;
	log.attach_class = p_acs->dev.attach_class;
	log.piowall = p_acs->dev.piowall;
	log.carryover = p_acs->dev.carryover;

	log.state = p_acs->dev.state;

	/* 
	 * Get the pos regs from the card 
	 */
	iocc = IOCC_ATT(p_acs->dds.bus_id, (IO_IOCC + (p_acs->dds.slot<<16)));

	/*
	 * use the no exception handling macro
	 */
	for (i = 0; i < 7; ++i)
		(void)BUS_GETCX( (iocc+i), &log.pos[i] );

	IOCC_DET(iocc);

	/* log the error */
	errsave(&log, sizeof(fddi_elog_t) );
	FDDI_DBTRACE("PlgE", p_acs->dev.state, 0, 0);
	return;
} /* end fddi_logerr() */

