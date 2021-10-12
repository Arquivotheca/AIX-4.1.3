
static char sccsid[] = "@(#)20	1.2  src/bos/usr/lpp/blkmux/samples/catkern.c, sysxcat, bos411, 9428A410j 10/25/91 09:47:34";
/*
 * COMPONENT_NAME: (SYSXCAT) - Channel Attach device handler
 *
 * FUNCTIONS: 
 *	Channel device driver application interface sample code
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES

 THE SOURCE CODE EXAMPLES PROVIDED BY IBM ARE ONLY INTENDED TO ASSIST IN THE
 DEVELOPMENT OF A WORKING SOFTWARE PROGRAM.  THE SOURCE CODE EXAMPLES DO NOT
 FUNCTION AS WRITTEN:  ADDITIONAL CODE IS REQUIRED.  IN ADDITION, THE SOURCE
 CODE EXAMPLES MAY NOT COMPILE AND/OR BIND SUCCESSFULLY AS WRITTEN.
 
 INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
 DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
 CORRECTION.

 IBM does not warrant that the contents of the source code examples, whether
 individually or as one or more groups, will meet your requirements or that
 the source code examples are error-free.

 IBM may make improvements and/or changes in the source code examples at
 any time.

 Changes may be made periodically to the information in the source code
 examples; these changes may be reported, for the sample device drivers
 included herein, in new editions of the examples.

 References in the source code examples to IBM products, programs, or
 services do not imply that IBM intends to make these available in all
 countries in which IBM operates.  Any reference to an IBM licensed
 program in the source code examples is not intended to state or imply
 that only IBM's licensed program may be used.  Any functionally equivalent
 program may be used.

 * RISC System/6000 is a trademark of International Business Machines 
   Corporation.
*/

/* Explain the structure a little */
struct global_data
{
	pid_t			pid;	/* process ID */
	cio_get_fastwrt_t	dd;	/* device driver attributes */
	struct mbuf		*p_rcvq_head;/* rcv queue head ptr */
	struct mbuf		*p_rcvq_tail;/* rcv queue tail ptr */
	struct mbuf		*p_txq;	/* TX queue */
	uint			events;	/* Events on the device */
	struct file		*devfp;	/* device file ptr */
};
typedef global_data global_t;

global_t	ctl;	/* declare global data structure */
   

/* Explain where defines are used and why */
#define STAT_STARTDONE	0x10000000	/* waiting for start done status */
#define STAT_HALTDONE	0x20000000	/* waiting for halt done status */
#define STAT_TXDONE	0x40000000	/* waiting for TX done status */
#define STAT_HARDFAIL	0x01000000	/* Hard failure has occured */
#define STAT_NRM_ENTER	0x02000000	/* Network Recovery Mode Enter */
#define STAT_NRM_EXIT	0x04000000	/* Network Recovery Mode Exit */
#define STAT_TXAVAIL	0x08000000	/* Tx is now available */
#define STAT_RCVDATA	0x10000000	/* rcv data is in queue */

int linkid;
int subchannel;

int mode;
int num; 
char name1[8];
char name2[8];
char name3[8];
char name4[8];
/*
 * NAME: rcv_fn
 *                                                                    
 * FUNCTION: Handles received data for the kernel process. 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine executes on the interrupt thread.
 *                                                                   
 * NOTES: 
 *	Does a quick check to filter out unwanted data.  If the rcv
 *	packet is not for us, the mbufs will be freed.  Otherwise, the
 *	mbufs will be placed onto the rcv queue.
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *	may put mbufs onto the rcv queue.
 *
 * ROUTINES CALLED:
 *
 * RETURNS: 
 *		Void
 */  
void
rcv_fn(	cio_read_ext_t	*p_readx,	/* read extention */
	struct mbuf	*p_mbuf);	/* ptr to mbufs */
{
       
	if (p_mbuf doesn't have the expected data)
	{
		m_freem(p_mbuf);
		return;
	}

	/* 
	 * put the packet onto the rcv queue 
	 */
	if (ctl.p_rcvq_head == NULL)
		ctl.p_rcvq_head = p_mbuf;
	else
		ctl.p_rcvq_tail->m_nextpkt = p_mbuf;

	e_post(STAT_RCVDATA, ctl.pid)
	return;

} /* end rcv_fn */
/*
 * NAME: tx_fn
 *                                                                    
 * FUNCTION: Handles transmit avail for the kernel process.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine executes on the interrupt thread.
 *                                                                   
 * NOTES: 
 *	This routine gets called when the device's transmit queue
 *	is no longer full.
 *
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *	may take mbufs off of the tx queue
 *
 * ROUTINES CALLED:
 *
 * RETURNS: 
 *		Void
 */  
void
tx_fn(
{

cat_write_ext_t        wrt_ext;       /* defined in catuser.h */
	/* 
	 * if there are requests on the TX queue. send one.
	 */

	if (ctl.p_txq != NULL)
	{

		/*
		 * take a packet off the tx queue (save the packet at the head 
	 	 * of the queue and advance the queue to the next packet)
		 */
		p_mbuf = dd.p_txq;
		dd.p_txq = p_mbuf->m_nextpkt;
		p_mbuf->m_nextpkt = NULL;

	        wrt_ext.cio_ext.flag = 0;          /* clear flags */
	        wrt_ext.cio_ext.status   = 0;	 /* clear status */
	        wrt_ext.cio_ext.netid = subchannel;

	        /* if CLAW mode */
	        wrt_ext.cio_ext.write_id = linkid; /* CLAW linkid */
					       
	  
		/* 
		 * call the fast write function
		 */
		rc = *(ctl.dd.fastwrt_fn)(ctl.dd.devno, p_mbuf, &wrt_ext));

		if ( rc != 0)
		{
			/*
			 * an error occured. put the packet back on the 
			 * TX queue.  Determine what the error was.
			 * Post status if necessary.
			 */
			
			p_mbuf->m_nextpkt = ctl.p_txq;
			ctl.p_txq = p_mbuf;

			if ( rc == EAGAIN )
			{
				/* we didn't get our Tx request in.
				 * some other tx_fn put data in the
				 * queue before us.
				 */
				return;
			}

		}
		else
			e_post(STAT_TXAVAIL, ctl.pid)

	}
	else
		e_post(STAT_TXAVAIL, ctl.pid)
	return;
} /* end tx_fn */


/*
 * NAME: stat_fn
 *                                                                    
 * FUNCTION: Handles asynchronous status blocks for kernel process
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine executes on the interrupt thread.
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *
 * ROUTINES CALLED:
 *
 * RETURNS: 
 *	Void
 */  
void
stat_fn( cio_stat_blk_t	*p_statblk);	/* status block ptr */
{


	/*
	 * switch on the status block's code to determine what status to pass up
	 */
	switch(p_statblk->code)
	{
		case CIO_START_DONE:
			/*
			 * we have a start done status block 
			 * determine if start was successful.
			 */

			if (p_statblk->option[0] == CIO_OK)
			{
				 /*
				  * read in starting subchannel and
				  * if CLAW mode specified, read
				  * option[2] for CLAW linkid
				  */

				 subchannel = p_statblk->option[1];
				 linkid = p_statblk->option[2];
				/* the start was successful */
				e_post(STAT_STARTDONE, ctl.pid);
			}
			else 
			{
				/* the start of the device failed
				 * post that the start is done and 
				 * that we have a hard error 
				 */

				e_post( (STAT_STARTDONE|STAT_HARDFAIL), 
					ctl.pid);
			}
			break;
		case CIO_HALT_DONE:
		
			/*
			 * we have a halt done status block 
			 */
			e_post(STAT_HALTDONE, ctl.pid);
			break;
		case CIO_TX_DONE:
			/*
			 * we have a tx done status block 
			 */
			e_post(STAT_TXDONE, ctl.pid);
			break;

		default:
			/*
			 * other status blocks are don't cares
			 * for this kproc
			 */
			break;

	} /* end switch */
	return;

} /* end stat_fn */

/*
 * NAME: start_device()
 *                                                                    
 * FUNCTION: This routine will open and start the CAT device driver.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine executes on the process thread.
 *                                                                   
 * NOTES: 
 *	This routine  assumes:
 *		- the kproc has been created
 *		- that all required code has been pinned
 *		- The kproc interrupt routines have addressability to
 *		  the global data structures
 *
 *	The fast write attributes of the CAT device driver are
 *	obtained via the CIO_GET_FASTWRT ioctl.
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *
 * ROUTINES CALLED:
 *
 * RETURNS: 
 *		0	- successful
 *		EIO	- failed
 */  

typedef struct {
    dev_t		devno;		/* device number of device */
    char		channame[20];	/* channel name of device (if any) */
    int			flags;		/* device open flags */
} DRVR_OPEN;

DRVR_OPEN   open;

int
start_device()
{
	cio_kopen_ext_t		openx; 	/* input struct for open */
	struct cat_set_sub      startsub;
	char			path[80];
	int			rc=0;
	ulong			events;


	/*
	 * set up the open extension 
	 */
	openx.rx_fn = rcv_fn;
	openx.tx_fn = tx_fn;
	openx.stat_fn = stat_fn;

	strcpy(path, "/dev/cat0");


	rc = fp_opendev(path, (O_RDWR | O_NDELAY), open.channame,
		&openx, &ctl.devfp);

	if ( rc != 0 )
		return(EIO);


	/*
	 * get the fast write attributes of the 
	 * CAT device driver
	 */
	ctl.dd.status = 0;
	ctl.dd.devno = 0;
	ctl.dd.chan = 0;
	ctl.dd.fastwrt_fn = 0;

	/* 
	 * Get the address of the fastwrite function for future reference
	 */
	rc = fp_ioctl(ctl.devfp, CIO_GET_FASTWRT,
			&ctl.dd, NULL);

	if ( rc != 0 )
	{
		fp_close(ctl.devfp);
		return(EIO);
	}

	/*
	 * start our network ID
	 */
        startsub.sb.netid = subchannel; /* set subchannel to use  */
	startsub.sb.length = 0;         /* not used in CAT driver */
	startsub.sb.status = 0;         /* exception code         */
	startsub.specmode = mode;       /* No special / CLAW      */ 
	startsub.subset = num;          /* number of subchannel   */
					/* will be used           */
	startsub.set_default = $default;/* use default? TRUE/FALSE */

	if (!startsub.set_default)
	{
	    startsub.shrtbusy = $busy_status;
	    startsub.startde = $startde;/* unsolicate device end ? */
					/* TRUE / FALSE            */
        }
		  
        /* if startsub.specmode = CAT_CLAW_MOD */
	startsub.claw_blk.WS_appl[] = name1;
	startsub.claw_blk.H_appl[] = name2;
	startsub.claw_blk.WS_adap[] = name3;
	startsub.claw_blk.H_name[] = name4;


	rc = fp_ioctl(ctl.devfp, CIO_START, &startsub, NULL)

	if ( rc != )
	{
		/* 
		 * the start ioctl failed
		 */
		fp_close(ctl.devfp);
		return(EIO);
	}

	/* 
	 * the asynchronous start done status block
	 * will be handled by the stat_fn()
	 */
	ctl.events = e_wait( (STAT_STARTDONE | STAT_HARDFAIL), 
			STAT_STARTDONE,
			EVENT_SHORT);

	if (ctl.events | STAT_HARDFAIL)
	{
		/*
		 * start was unsuccessful 
		 */
		return(EIO);
	}

	return(0);

} /* end start_device() */
/*
 * NAME: halt_device()
 *                                                                    
 * FUNCTION: This routine will halt and close the CAT device driver.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine executes on the process thread.
 *                                                                   
 * NOTES: 
 *	This routine  assumes:
 *		- the kproc has been created
 *		- that all required code has been pinned
 *		- The kproc interrupt routines have addressability to
 *		  the global data structures
 *
 * RECOVERY OPERATION: 
 *	None.
 *
 * DATA STRUCTURES: 
 *
 * ROUTINES CALLED:
 *
 * RETURNS: 
 *	void
 */  

void
halt_device()
{
	struct cat_set_sub   haltsub;
	int rc=0;

	haltsub.sb.netid = subchannel;       /* subchannel to halt */
	haltsub.claw_blk.linkid = linkid;    /* CLAW linkid */
	haltsub.claw_blk.WS_appl[] = name1;  /* CLAW workstation name */ 
	haltsub.claw_blk.H_appl[] = name2;   /* CLAW Host application */
	haltsub.claw_blk.WS_adap[] = name3;  /* CLAW workstation appl */
	haltsub.claw_blk.H_name[] = name4;   /* CLAW Host name        */

	/* 
	 * issue a halt to stop the use of our netid that we registered with the
	 * CIO_START command
	 */
	rc = fp_ioctl(ctl.devfp, CIO_HALT, &haltsub, NULL)

	if ( rc != )
	{
		/* 
		 * the start ioctl failed
		 */
		fp_close(ctl.devfp);
		return;
	}

	return;

} /* end halt_device() */

