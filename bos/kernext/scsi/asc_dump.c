static char sccsid[] = "@(#)07	1.1  src/bos/kernext/scsi/asc_dump.c, sysxscsi, bos411, 9428A410j 9/29/93 14:11:38";
/*
 *   COMPONENT_NAME: SYSXSCSI
 *
 *   FUNCTIONS: asc_dump
 *		asc_dump_write
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "ascincl.h"
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/dump.h>
#include <sys/pin.h>
#include <sys/ascsidd.h>
#include <string.h>

int asc_dump_write (ndd_t  *ndd, struct ctl_elem_blk *ctl_blk);
int asc_dump_start (struct adapter_info *ap);

/*  Global storage area  used to manage all adapters. */
extern	adp_ctrl_t	adp_ctrl;

/*  Global device driver component dump table pointer. */
extern	struct	asc_cdt_tab	*asc_cdt;

/**************************************************************************/
/*                                                                        */
/* NAME: asc_dump                                                         */
/*                                                                        */
/* FUNCTION: Fast and half-wide SCSI Adapter device driver dump entry     */
/*           point.							  */
/*                                                                        */
/*      This driver is the interface between the SCSI adapter device      */
/*      driver and the actual SCSI adapter for handling requests for      */
/*      dumping data.                                                     */
/*                                                                        */
/*      Several ioctls are defined to provide for system management       */
/*      and adapter diagnostic functions.                                 */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*                                                                        */
/*      This routine is called on the interrupt level. Limited resources  */
/*      are available. No page faults allowed, polling mode operation.    */
/*                                                                        */
/*  NOTES:  This routine handles the following operations :               */
/*      DUMPINIT   - initializes SCSI device as primary dump device.	  */
/*      DUMPSTART  - prepares device for dump                             */
/*      DUMPQUERY  - returns the maximum and minimum number of bytes that */
/*                   can be transferred in a single DUMPWRITE command     */
/*      DUMPWRITE  - performs write to disk                               */
/*      DUMPEND    - cleans up device on end of dump                      */
/*      DUMPTERM   - terminates the bus attached disk as dump device      */
/*                                                                        */
/*                                                                        */
/* INPUTS:                                                                */
/*      p_ndd   - pointer to an ndd                                       */
/*      cmd     - Type of dump operation being requested                  */
/*      arg     - Pointer to dump query structure or buf_struct for cmd   */
/*                                                                        */
/* DATA STRUCTURES INHERITED FROM ADAPTER DRIVER: 			  */
/*                    adapter_info   - adapter info structure             */
/*                    dev_info       - device info structure              */
/*									  */
/* INTERNAL PROCEDURES CALLED:                                            */
/*      asc_dump_start                                                    */
/*      asc_dump_write                                                    */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      A zero will be returned on successful completion, otherwise,      */
/*      one of the errno values listed below will be given.               */
/*                                                                        */
/*      EBUSY     -  request sense inuse                                  */
/*      EINVAL    -  Invalid iovec argumenmt,or invalid cmd               */
/*      EIO       -  I/O error, quiesce or scsi writefailed.              */
/*      ENOMEM    -  Unable to allocate resources                         */
/*      ENXIO     -  Not inited as dump device                            */
/*      ETIMEDOUT -  adapter not responding                               */
/*                                                                        */
/**************************************************************************/
int
asc_dump (ndd_t * p_ndd,
	  int cmd,
	  caddr_t arg)
{
    struct adapter_info *ap;
    uchar     pio_error = 0;
    ulong     ioaddr;				/* IO handle */

    /*
     * Get the adapter structure for this adapter. Note: the ndd_correlator
     * field contains the address of the adapter structure. 
     */
    ap = (struct adapter_info *) p_ndd->ndd_correlator;
    if (ap == NULL) {
	return (EIO);
    }

    /*
     * Ensure adapter and device are opened and started. 
     */
    if (!ap->opened) {
	return (EIO);
    }

    /*
     * Determine what operation has been requested. 
     */
    switch (cmd)
    {
      case DUMPINIT:
	/*
	 * Initialize this device as the target dump device. 
	 */
	ap->dump_state = DUMP_INIT;
	break;

      case DUMPTERM:
	/*
	 * Unitialize this device as the target dump device. 
	 */
	if (ap->dump_state != (uint)DUMP_INIT)
	    return (EINVAL);

	ap->dump_state = DUMP_IDLE;
	break;

      case DUMPSTART:
	/*
	 * Prepare a device for a kernel dump.
	 */
	if (ap->dump_state != (uint)DUMP_INIT &&
	    ap->dump_state != (uint)DUMP_START)
	    return (EINVAL);

	/* don't do anything if already done DUMPSTART */
	if (!(ap->dump_state == (uint)DUMP_START)) 
	{
	    ap->dump_state = DUMP_START;

	    /*
	     * Disable Target Mode.
	     * Which target eid should be disabled?
	     * ap->ndd.ndd_ctl (&ap->ndd, NDD_TGT_SUS_DEV, NULL);
	     */

/*
TEST CODE
*/
ap->num_buf_cmds = 0;

	    /*
	     * Set the adapter so it doesn't clear the status
	     * register on a read.
	     */
	    ioaddr = (ulong) io_att( ap->ddi.bus_id, NULL ); 
	    WRITE_BCR(ap->ddi.base_addr, BCR_ENABLE_INTR | BCR_ENABLE_DMA);
	    io_det( ioaddr );

	    /*
	     * Overlay ndd_output with asc_dump_write since all
	     * output will now be to the dump device.
	     */
	    ap->ndd.ndd_output = (int (*) ()) asc_dump_write;
	}
	break;

      case DUMPEND:
	/*
	 * The dump to this device has ended.
	 */
	if (ap->dump_state != (uint)DUMP_START)
	    return (EINVAL);

	/* 
	 * Restore ndd_output to its former glory.
	 */
	ap->ndd.ndd_output = (int (*) ()) asc_output;

	ap->dump_state = DUMP_IDLE;
	break;

      case DUMPQUERY:
	/*
	 * Unused
	 */
	return (EINVAL);
	break;

      case DUMPWRITE:
	/*
	 *  Unused.  The asc_dump_write() routine is called directly.
	 */
	return (EINVAL);
	break;

      default:
	return (EINVAL);
    }
    return (0);
}


/*************************************************************************/
/*                                                                      */
/* NAME:        asc_delay                                               */
/*                                                                      */
/* FUNCTION:    provide variable delay in 1 usec increments             */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*     process or interrupt                                             */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*                                                                      */
/* INPUTS:                                                              */
/*      delay - number of usecs to delay                                */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      none                                                            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*                                                                      */
/************************************************************************/
void
asc_delay (struct adapter_info *ap, int delay)
{
    caddr_t iocc_addr;
    int     i;
    uchar   no_val;

    iocc_addr = (caddr_t)BUSIO_ATT((ap->ddi.bus_id | 0x000c0060), 0);
    for (i = 0; i < delay; i++)
	/* GETCX due to hardware restriction */
	(void) BUS_GETCX((iocc_addr + 0xe0), &no_val);
    BUSIO_DET(iocc_addr);
}



/**************************************************************************/
/*									  */
/* NAME:	asc_dump_write						  */
/*									  */
/* FUNCTION: 	adapter driver's dump output routine			  */
/*									  */
/* EXECUTION ENVIRONMENT:						  */
/*		interrupt level only.					  */
/*									  */
/* NOTES:								  */
/*									  */
/* CALLED FROM:	asc_dump						  */
/*									  */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*		ndd_ouptut						  */
/*									  */
/* INPUT:								  */
/*		ndd	- network device driver structure		  */
/*		ctl_blk - control element block				  */
/*									  */
/* RETURN VALUE DESCRIPTION:                                              */
/*      A zero will be returned on successful completion, otherwise,      */
/*      one of the errno values listed below will be given.               */
/*                                                                        */
/*		EIO	- kernel service failure or invalid operation	  */
/*									  */
/**************************************************************************/
int
asc_dump_write (ndd_t  *ndd,
		struct ctl_elem_blk *ctl_blk)
{
    struct    timestruc_t current_time;
    long int  start_time;
    long int  timeout_value;
    volatile  uchar bsr;
    int       rc;
    struct    adapter_info	*ap;
    ulong     *reply;
    ulong     eid;
    uchar     pio_error = 0;
    ulong     ioaddr;				/* IO handle */
/*
TEST CODE
*/
uchar bsr2,bsr3;


    /*
     *  Get the adapter structure for this adapter.
     *  Note: the ndd_correlator field contains the address
     *  of the adapter structure.
     */
    ap = (struct adapter_info *) ndd->ndd_correlator;
    if (ap == NULL) {
	return ( EIO );
    }

/*
TEST CODE
*/
ap->num_buf_cmds++;

    /*
     *  Zero out the bus memory field so as not to confuse
     *  driver into believing the resources have already been
     *  allocated.
     */
    if (ctl_blk->pd_info != NULL) 
    {
	ctl_blk->pd_info->mapped_addr = 0x0;
    }
    
    /*
     *  Mark this as a DUMP element which indicates that 
     *  this was the reply the dequeue logic was looking for
     *  during a dump.
     */
    ctl_blk->status = DUMP_ELEMENT;

    /*  
     *  Translate the addresses to real addresses.
     *  asc_vtor() will translate the virtual addresses 
     *  to real addresses and allocates and maps the
     *  bus memory space(TCWs).
     */
    rc = asc_vtor (ap, ctl_blk);	
    if ( rc ) 
    {
	/*
	 *  No resources available to issue
	 *  command. Leave the cmd on the waitq.
	 */
	return ( EIO );
    }

    /*
     *  Push the ctl_elem_blk onto the end of the wait queue.
     */
    if ( ap->wait_head == NULL ) {
	ap->wait_head = ctl_blk;
	ap->wait_tail = ctl_blk;
	ctl_blk->prev = NULL;
	ctl_blk->next = NULL;
    }
    else {
	ap->wait_tail->next = ctl_blk;
	ctl_blk->prev = ap->wait_tail;
	ap->wait_tail = ctl_blk;
	ctl_blk->next = NULL;
    }
    
    /*  Increment the number of q'd commands counter. */
    ap->num_cmds_queued++;



/*
WORK
*/
	    ioaddr = (ulong) io_att (ap->ddi.bus_id, NULL); 
	    READ_BSR(ap->ddi.base_addr, &bsr3);
	    if ((bsr3 & BSR_INTERRUPT )) {
		ATTENTION( ap->ddi.base_addr, 0xE0 );
	    }
	    io_det (ioaddr);




    /*
     *  Translate the control element to an adapter 
     *  control element and enqueue onto pipe.
     */
    rc = asc_translate (ap, ctl_blk);
    if ( !rc )
    {
	/*
	 * Initialize the reply element to a known value.
	 */
	reply = (ulong *)ctl_blk->reply_elem;
	reply[EID_WORD] = 0xDEADF00D;

	/*
	 *  The element was enqueued on the outbound 
	 *  pipe successfully.
	 * 
	 * Start a polling loop, waiting for data to be written or a timeout.
	 */
	timeout_value = DUMP_TIMEOUT + 1;

	curtime (&current_time);	/* get current system time */
	
	for (start_time = (long int) current_time.tv_sec;
	     ((long int) current_time.tv_sec - start_time) < timeout_value;
	     curtime (&current_time)) 
	{
	    /* 
	     * Wait 1/10000 second before reading.
	     */
	    asc_delay (ap, 100);

	    /*
	     * Check status register for an interrupt or error.
	     */
	    ioaddr = (ulong) io_att (ap->ddi.bus_id, NULL); 
	    READ_BSR(ap->ddi.base_addr, (uchar *) &bsr);
	    io_det (ioaddr);
	    
	    if ( pio_error == TRUE ) {
		return ( EIO );
	    }
	    
	    /*
	     *  Determine if an exception was experienced.
	     */
	    if ( bsr & BSR_EXCEPTION ) {
		return ( EIO );
	    }
	    
	    /*
	     *  Determine if our adapter posted an interrupt.
	     */
	    if ( ! (bsr & BSR_INTERRUPT )) {
		continue;
	    }
	    
	    /*
	     * At this point we should have a valid interrupt with
	     * no errors.
	     */

		/* WORK */
	    asc_delay (ap, 100);

	    /*
	     * Get data from the inbound pipe.
	     */
	    asc_dequeue (ap);

	    /*
	     *  Issue an End Of Interrupt(EOI) signal to
	     *  the adapter. This is done by setting the 
	     *  EIO bit in the Attention register.  This
	     *  will clear the BSR.
	     */
	    ioaddr = (ulong) io_att (ap->ddi.bus_id, NULL); 
	    ATTENTION( ap->ddi.base_addr, 0xE0 );
	    io_det (ioaddr);

	    /*
	     * Check to see that the write completed successfully.
	     */
	    reply = (ulong *)ctl_blk->reply_elem;
	    eid = ( reply[EID_WORD] & 0xFF000000 );
            if ( eid == 0x40000000 ) {
		return ( 0 );
	    }
	    else if ( eid == 0xDE000000 ) {
		continue;
	    }
	    else {
		return ( EIO );
	    }
        }
	/*
	 * A timeout occurred.
	 */

/*
TEST CODE
*/
ioaddr = (ulong) io_att (ap->ddi.bus_id, NULL);
READ_BSR(ap->ddi.base_addr, &bsr2);
ATTENTION( ap->ddi.base_addr, 0xE0 );
io_det (ioaddr);

	asc_sendlist_dq (ap, ctl_blk);
	ap->num_cmds_active--;

	return ( EIO );
    }
    else
    {
	/*
	 *  The element was NOT enqueued on the outbound 
	 *  pipe successfully.
	 */
	return (EIO);
    }
}

/*
 * NAME:        asc_cdt_func
 *
 * FUNCTION:    
 *		Adapter driver's component dump table function.
 *              This function builds the driver dump table during a 
 *		system dump.
 *
 * EXECUTION ENVIRONMENT:
 *              Interrupt level only.
 *
 * INPUT:
 *              arg     -  1 dump dd is starting to get dump table entries
 *                         2 dump dd is finished getting the dump table entries
 *
 * RETURN VALUE DESCRIPTION:
 *              Return code is a pointer to the struct cdt to be dumped for
 *              this component.
 */
struct cdt *
asc_cdt_func(
            int    arg) 
{
    int size,                      /* used to compute size of this cdt entry */
        i;                         /* for loop variable to walk scsi_ptrs    */
    struct    adapter_info    *ap; /* adapter structure 		     */

    if (arg == 1) {
        /* Only build the dump table on the initial dump call */
        
        /* Init the table. */
        bzero((char *) asc_cdt, sizeof(struct asc_cdt_tab));
      
        /* Init the head structure. */
        asc_cdt->asc_cdt_head._cdt_magic = DMP_MAGIC;
        strcpy(asc_cdt->asc_cdt_head._cdt_name, "ascsi");

        size = sizeof(struct cdt_head);

        /* 
	 *  Now begin filling in elements.
         *  Loop through adapter structure pointers.
	 */
        for (i = 0; i < MAX_ADAPTERS; i++) {
	    if (adp_ctrl.ap_ptr[i] == NULL)
		continue;
	    else
	    {
        	ap = adp_ctrl.ap_ptr[i];
                asc_cdt->asc_entry[i].d_segval = 0x0;

                /* Copy name to element. */
                bcopy((char *) ap->ddi.resource_name,
                (char *) asc_cdt->asc_entry[i].d_name, 8);
                asc_cdt->asc_entry[i].d_ptr = (char *) ap;
                asc_cdt->asc_entry[i].d_len = sizeof(struct adapter_info);
                size += sizeof(struct cdt_entry);
	    }
        } 
        /* fill in the actual table size */
        asc_cdt->asc_cdt_head._cdt_len = size;
    } 

    return((struct cdt *) asc_cdt);

}
