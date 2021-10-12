static char sccsid[] = "@(#)73	1.3  src/bos/kernext/fddi/fddi_prim.c, sysxfddi, bos411, 9428A410j 3/23/94 15:31:47";
/*
 *   COMPONENT_NAME: SYSXFDDI
 *
 *   FUNCTIONS: fddi_logerr
 *		fddi_pio_retry
 *		fddi_trace
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "fddiproto.h"

/* 
 * get access to the global device driver control block
 */
extern fddi_tbl_t	fddi_tbl;

/*
 * NAME: fddi_pio_retry
 *
 * FUNCTION: This routine is called when a pio routine returns an
 *	exception. It will retry the PIO 
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Called by interrupt and processes level
 *	This routine is invoked by the PIO_xxxX routines
 *
 * RETURNS:
 *	Boolean: set to whether the pio was successful within the 
 *		PIO_RETRY_COUNT attempts
 */

int
fddi_pio_retry (
	fddi_acs_t	*p_acs,	   	/* tells which adapter this came from */
	enum pio_func 	iofunc,		/* io function to retry	*/
	void 		*ioaddr,	/* io address of the exception	*/
	long		ioparam,	/* parameter to PIO routine	*/
	int		cnt)		/* for string copies */

{
	int	retry_count = PIO_RETRY_COUNT;
	int	ipri;
	int 	rc;


	while (retry_count > 0)
	{
		/* 
		 * retry the pio function, return if successful
		 */
		switch (iofunc)
		{
			case PUTC:
				rc = BUS_PUTCX((char *)ioaddr,
							(char)ioparam);
				break;
			case PUTSR:
				rc = BUS_PUTSRX((short *)ioaddr,
							(short)ioparam);
				break;
			case PUTLR:
				rc = BUS_PUTLRX((long *)ioaddr,
							(long)ioparam);
				break;
			case GETC:
				rc = BUS_GETCX((char *)ioaddr,
							(char *)ioparam);
				break;
			case GETSR:
				rc = BUS_GETSRX((short *)ioaddr,
							(short *)ioparam);
				break;
			case GETLR:
				rc = BUS_GETLRX((long *)ioaddr,
							(long *)ioparam);
				break;
			case PUTSTR:
				rc = BUS_PUTSTRX((long *)ioaddr,
							(long *)ioparam,
							cnt);
				break;
			case GETSTR:
				rc = BUS_GETSTRX((long *)ioaddr,
							(long *)ioparam,
							cnt);
				break;
			default:
				FDDI_ASSERT(0);
		}

		if (rc == 0)
		{
			/* ok */
			return(FALSE);
		}

		retry_count--;
	}
	fddi_logerr(p_acs, ERRID_CFDDI_PIO, __LINE__, __FILE__, rc,0,0);
	return (TRUE);
}

/*
 * NAME: fddi_trace
 *                                                                    
 * FUNCTION: 
 *	This routine puts a trace entry into the internal device
 *	driver trace table.
 *                                                                    
 * EXECUTION ENVIRONMENT: process or interrupt environment
 *                                                                   
 * NOTES:  See the design for the trace format
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
	register int	ipri;

	ipri = disable_lock(CFDDI_OPLEVEL,&fddi_tbl.trace_lock);
	/*
	 * get the current trace point index 
	 */
	i= fddi_tbl.trace.next;

	p_str = &str[0];

	/*
	 * copy the trace point data into the global trace table.
	 */
	fddi_tbl.trace.table[i] = *(ulong *)p_str;
	++i;
	fddi_tbl.trace.table[i] = arg2;
	++i;
	fddi_tbl.trace.table[i] = arg3;
	++i;
	fddi_tbl.trace.table[i] = arg4;
	++i;


	if ( i < FDDI_TRACE_SIZE )
	{
		fddi_tbl.trace.table[i] = 0x21212121;	/* end delimeter */
		fddi_tbl.trace.next = i;
	}
	else
	{
		fddi_tbl.trace.table[0] = 0x21212121;	/* end delimeter */
		fddi_tbl.trace.next = 0;
	}


	/*  Make the AIX trace point call */
	unlock_enable(ipri,&fddi_tbl.trace_lock);

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
 *	values will be logged to the system.  All other error log values
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
		char		*p_file,
		int		status1,
		int		status2,
		int		status3)
{ 

	int		i;
	fddi_errlog_t	log;
	char		errbuf[300];
	int 		iocc;

	/*
	 * Store the error id into the log entry
	 */
	log.errhead.error_id = errid;

	/* plug in the resource name for this device */
	strncpy( log.errhead.resource_name, p_acs->dds.lname, ERR_NAMESIZE);

 	for (i=0;i<CFDDI_NADR_LENGTH; ++i)
	  	log.src_addr[i] = p_acs->addrs.src_addr[i];

 	for (i=0; i<11; ++i)
  		log.stest[i] = p_acs->dev.stest[i];

	/* plug in the line number and filename */
	sprintf(errbuf, "line: %d file: %s", line, p_file);

	strncpy(log.file, errbuf, (size_t)sizeof(log.file));

	log.ls = p_acs->ls;

	log.mcerr = p_acs->dev.mcerr; 
	log.iox = p_acs->dev.iox;
	log.status1 = status1;
	log.status2 = status2;
	log.status3 = status3;
	log.attach_class = p_acs->dev.attach_class;

	log.state = p_acs->dev.state;

	/* 
	 * Get the pos regs from the card 
	 */
	iocc = IOCC_ATT(p_acs->dds.bus_id, (IO_IOCC + (p_acs->dds.slot<<16)));

	/*
	 * use the system macro's (no retry logic).
	 */
	for (i = 0; i < 7; ++i)
		(void)BUS_GETCX( (iocc+i), &log.pos[i] );

	IOCC_DET(iocc);

	/* log the error */
	errsave(&log, sizeof(fddi_errlog_t) );
	return;
} /* end fddi_logerr() */

