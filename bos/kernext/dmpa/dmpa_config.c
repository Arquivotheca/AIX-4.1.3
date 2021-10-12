static char sccsid[] = "@(#)71        1.1  src/bos/kernext/dmpa/dmpa_config.c, sysxdmpa, bos411, 9428A410j 4/30/93 12:48:56";
/*
 *   COMPONENT_NAME: (SYSXDMPA) MP/A DIAGNOSTICS DEVICE DRIVER
 *
 *   FUNCTIONS: alloc_acb
 *		alloc_resources
 *		free_acb
 *		get_acb
 *		mpa_init_dev
 *		mpa_set_pos
 *		mpa_term_dev
 *		mpaconfig
 *		shutdown_adapter
 *		
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

#include <errno.h>
#include <sys/adspace.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/file.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/malloc.h>
#include <sys/timer.h>
#include <sys/dmpauser.h>
#include <sys/dmpadd.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/uio.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>
#include <sys/errids.h>
#include <sys/poll.h>
#include <sys/signal.h>

/*******************************************************************
 *    External Function Declarations                               *
 *******************************************************************/

extern int mpaintr();           /* SLIH function */
extern int nodev();
extern int mpaopen();
extern int mpaclose();
extern int mparead();
extern int mpawrite();
extern int mpaioctl();
extern int mpaselect();
extern int mpampx();
extern int mpa_offlvl();


struct acb *acbheader = NULL;
int global_num_adapters = 0;
int acbglobal_lock = LOCK_AVAIL;         /* Global Lock Variable */
int mpaopened = 0;                          /* driver opened count */

/*******************************************************************
 *      Declarations for this translation unit                     *
 *******************************************************************/

static struct devsw mpa_dsw;   /* the devsw entry for devswadd */


/*******************************************************************
 *      External Declarations                                      *
 *******************************************************************/


/*
 * NAME: mpaconfig
 *
 * FUNCTION: mpaconfig performs operations necessary to the initialization of
 *           an individual port on an adapter.  mpaconfig will be called for
 *           each valid port during the bus configuration/device configuration
 *           phase of the boot procedure.  mpaconfig must set up various data
 *           data structures, validate requests and so forth.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Preemptable        : Yes
 *      VMM Critical Region: No
 *      Runs on Fixed Stack: Yes
 *      May Page Fault     : Yes
 *      May Backtrack      : Yes
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION: 0 - EVT mpaconfig return was successful
 */

int mpaconfig ( dev_t           dev,
		int             cmd,
		struct uio      *uiop )

{
    /* mpaconfig local variables */

    int                 rc=0;             /* general return code */
    struct acb          *acb;         /* pointer to the acb */

    /* log a trace hook */
    DDHKWD2 (HKWD_DD_MPADD, DD_ENTRY_CONFIG, 0, dev, cmd);

    /*
    ** Lock Global Lock, waits for lock or returns early on signal
    */
    if (lockl(&acbglobal_lock, LOCK_SIGRET) == LOCK_SIG) {
	    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,EINTR,dev,0xA0);
	    return EINTR;
    }

    switch (cmd)                /* switch based on command type */
    {

	case CFG_INIT:          /* initialize device driver and internal */
				/* data areas                            */
	{

	    /* first time through CFG_INIT requires special steps */
	    /* no active adapters means this is first time */

	    if (acbheader == NULL) { /* First adapter to be added */
		/*
		** Pin all of the driver code.
		*/
		if (rc = pincode(mpaconfig)) {
			unlockl(&acbglobal_lock);
			DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,rc,dev,0xA1);
			return rc;
		}

		/* add our entry points to the devsw table */

		mpa_dsw.d_open     = mpaopen;
		mpa_dsw.d_close    = mpaclose;
		mpa_dsw.d_read     = mparead;
		mpa_dsw.d_write    = mpawrite;
		mpa_dsw.d_ioctl    = mpaioctl;
		mpa_dsw.d_strategy = nodev;
		mpa_dsw.d_ttys     = (struct tty *)NULL;
		mpa_dsw.d_select   = mpaselect;
		mpa_dsw.d_config   = mpaconfig;
		mpa_dsw.d_print    = nodev;
		mpa_dsw.d_dump     = nodev;
		mpa_dsw.d_mpx      = mpampx;
		mpa_dsw.d_revoke   = nodev;
		mpa_dsw.d_dsdptr   = NULL;
		mpa_dsw.d_selptr   = NULL;
		mpa_dsw.d_opts     = 0;

		if ((rc = devswadd(dev, &mpa_dsw)) != 0) {
		    unpincode (mpaconfig);
		    unlockl(&acbglobal_lock);
		    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,rc,dev,0xA2);
		    return(rc);
		}

	    } /* end of first time...pin code and load devsw entry */

	    /*
	    ** Make sure this device wasn't already configured.
	    */
	    if (acb = get_acb(minor(dev))) {
		    unlockl(&acbglobal_lock);
		    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,EEXIST,dev,0xA3);
		    return EEXIST;
	    }

	    /*
	    ** make sure that they don't try to exceed the
	    ** maximum number of adapters
	    */
	    if (global_num_adapters > MAX_ADAPTERS) {
		    unlockl(&acbglobal_lock);
		    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,ENXIO,dev,0xA4);
		    return ENXIO;
	    }
	    global_num_adapters++;

	    /*
	    ** Allocate a acb structure.
	    */
	    if ((acb = alloc_acb()) == NULL) {
		    rc = ENOMEM;
		    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,rc,dev,0xA5);
		    goto release_drvr;
	    }
	    acb->adap_lock = LOCK_AVAIL;
	    /*
	    ** Copy cfg mthd data into device dependent structure.
	    */
	    if (rc = uiomove(&acb->mpaddp,sizeof(acb->mpaddp),UIO_WRITE,uiop)) {
		    free_acb(acb);
		    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,rc,dev,0xA6);
		    goto release_drvr;
	    }
	    acb->dev = dev;
	    if ( (rc = mpa_set_pos(acb)) ) {
		    free_acb(acb);
		    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,rc,dev,0xA7);
		    goto release_drvr;
	    }

	    /*
	    ** reset the 8273 on mpa card.. NOTE: do not remove this
	    ** reset it is needed to clean up the card to start. If
	    ** this reset is not done the cio_start may hang.
	    */
	    if ( (rc=shutdown_adapter(acb)) ) {
		    free_acb(acb);
		    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,rc,dev,0xA8);
		    goto release_drvr;
	    }

	    acb->flags |= MPAINIT;
	    if (mpaopened == 0)
			unpincode(mpaconfig);

	    break;

	} /* end of case CFG_INIT */


	case CFG_TERM:          /* terminate the device associated with */
				/* dev parameter                      */
	{
	    /*
	    ** Get the acb structure.
	    */

	    if ((acb = get_acb(minor(dev))) == NULL) {
		    unlockl(&acbglobal_lock);
		    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,EACCES,dev,0xA9);
		    return EACCES;
	    }

	    /*
	    ** Don't terminate if there are outstanding open()s.
	    */
	    if (acb->num_opens) {
		    unlockl(&acbglobal_lock);
		    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,EBUSY,dev,0xAA);
		    return EBUSY;
	    }

	    /*
	    ** Make sure the driver is pinned
	    */
	    if (mpaopened == 0){
		    pincode(mpaconfig);
	    }

	    /*
	    **  Shutdown the adapter puts 8273 in reset state.
	    */
	    shutdown_adapter(acb);

	    /*
	    ** Free system resources.
	    */
	    global_num_adapters--;

	    free_acb(acb);
	    if (acbheader == NULL)  {
		    rc = devswdel(dev);
	    }
	    if (acbheader == NULL || mpaopened == 0){
		    rc = unpincode(mpaconfig);
	    }

	    break;

	} /* end of case CFG_TERM */


	case CFG_QVPD:          /* query vital product data             */
	    break;

	default:
	    unlockl(&acbglobal_lock);
	    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,EINVAL,dev,0xAB);
	    return(EINVAL);

    } /* end of switch statement */

    unlockl(&acbglobal_lock);
    DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,rc,dev,0);
    return(rc);

release_drvr:
	if (acbheader == NULL)
		 devswdel(dev);
	if (acbheader == NULL || mpaopened == 0)
		 unpincode(mpaconfig);
	unlockl(&acbglobal_lock);
	DDHKWD2(HKWD_DD_MPADD,DD_EXIT_CONFIG,rc,dev,0xAB);
	return rc;
}  /* mpaconfig() */
/*****************************************************************************
** NAME:        shutdown_adapter
**
** FUNCTION:    This code resets and enables the adapter
**              It is need during bringup to reset the card and put it in
**              the correct initial state. It can also be used to reset
**              the device to a correct state.
**
** EXECUTION ENVIRONMENT:
**
** NOTES:
**
** RETURNS:     0 if successful
**            EIO on PIO error.
**
*****************************************************************************/
int shutdown_adapter (struct acb *acb)
{
	int                spl,rc,cntr=0;
	ulong              pos_ptr;


	/*
	**  this routine is called at config time and it sets the
	**  mode for the 8255.
	*/

	DISABLE_INTERRUPTS(spl);

	/*
	** Use the 8255 port b to reset the 8273.
	*/
	while ( ++cntr < 2) {
	     if(PIO_PUTC( acb, MODE_OFFSET, 0x98 )== -1) {
		     pos_ptr = MPA_IOCC_ATT;
		     BUS_PUTC( pos_ptr + POS2, acb->pos2 );
		     BUS_PUTC( pos_ptr + POS3, acb->pos3 );
		     IOCC_DET(pos_ptr);
	     }
	     else break;
	}
	if(cntr==2) {
		ENABLE_INTERRUPTS(spl);
		return EIO;
	}

	/*
	**  Save the current Pos reg values. This adds a needed delay
	**  Between the two 8255 accesses without calling a delay routine
	**  That will trap if called in irpt environment.
	*/
	pos_ptr = MPA_IOCC_ATT;

	    acb->pos0 = (uchar) BUS_GETC( pos_ptr + POS0 );
	    acb->pos1 = (uchar) BUS_GETC( pos_ptr + POS1 );
	    acb->pos2 = (uchar) BUS_GETC( pos_ptr + POS2 );
	    acb->pos3 = (uchar) BUS_GETC( pos_ptr + POS3 );

	IOCC_DET( pos_ptr );

	/*
	** This next write will disable all timers. and set the
	** 8273 in a reset state. It will stay in this state
	** until the CIO_START is issued.
	*/
	if(PIO_PUTC( acb, PORT_B_8255, 0x17 )== -1) {
	    ENABLE_INTERRUPTS(spl);
	    return EIO;
	}

	/*
	** since shutdown adapter kills recv, 0 the flag
	*/
	acb->flags &= ~RECEIVER_ENABLED;

	/*
	** free up any recv dma being held in acb
	*/
	acb->hold_recv = NULL;

	/*
	** Mark the adapter dead - RIP  MPA is brought back to life
	** by CIO_START.
	*/
	acb->flags &= ~STARTED_CIO;
	acb->flags |= MPADEAD;


	ENABLE_INTERRUPTS(spl);
	return 0;
} /* shutdown_adapter() */



/*****************************************************************************
** NAME:        mpa_set_pos
**
** FUNCTION:    Verifies the CARD ID value and sets the POS register values
**              according the information in the DDS.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environent only.
**
** NOTES:
**      Inputs:
**              pointer to a channel adapter structure
**      Outputs:
**              return status
**      Called By:
**              mpaconfig()
**      Calls:
**
** RETURNS:     0 - Success
**              (-1) - failed
**
*****************************************************************************/
int
mpa_set_pos (struct acb *acb)
{
    ulong                         pos_ptr;
    uchar                         t_pos;
    int                           spl;

	DISABLE_INTERRUPTS(spl);

	/* get access to bus I/O space */
	pos_ptr = MPA_IOCC_ATT;

	t_pos = (unsigned char)0;


	/* POS2 contains CARD ENABLE + Mode selection + DMA ENABLE   */
	/* V.25 bis support                                     */
	/*   7     6     5     4     3     2     1     0
	  +-----+-----+-----+-----+-----+-----+-----+-----+
	  | n/a | V.25| DMA |  Mode selection code  | CD  |
	  +-----+-----+-----+-----+-----+-----+-----+-----+
	 */

	/* set the card enable and n/a bits on */
	/* for now default V.25 bit to 1 ( not enabled) */
	/* set DMA on for SDLC and set mode to 8 for SDLC */
	/* so this reg will be set to 0xF1 */
	/* later on I can use some odm or smit input to set up in
	   differnent mode or to enable V.25  */
	if( (DDS.io_addr&0x0a0) == 0x0a0)
	   t_pos = (P2_ENABLE|P2_ALT_SDLC );
	else
	   t_pos = (P2_ENABLE|P2_SDLC_MODE );

	/* turn on the n/a bit */
	t_pos |= 0x80;

	BUS_PUTC( pos_ptr + POS2, t_pos );

	/* POS3 contains arb level.             */
	/*   7     6     5     4     3     2     1     0
	  +-----+-----+-----+-----+-----+-----+-----+-----+
	  |  1  |  1  |  1  |  1  |   arbitration level   |
	  +-----+-----+-----+-----+-----+-----+-----+-----+
	 */

	t_pos = (unsigned char)0;
	t_pos = DDS.dma_lvl;
	t_pos |= 0xF0;   /* set the one bits back */

	BUS_PUTC( pos_ptr + POS3, t_pos );

	IOCC_DET( pos_ptr );
	ENABLE_INTERRUPTS(spl);

	return(0);
} /* mpa_set_pos() */

/*****************************************************************************
** NAME:        mpa_init_dev
**
** FUNCTION:    Sets the POS register values according the
**                              information in the DDS.
**                              Install the interrupt handler.
**                              Install the off-level interrupt handler.
**                              Acquire a DMA channel.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environent only.
**
** NOTES:
**      Inputs:
**              pointer to a channel adapter structure
**      Outputs:
**              return status
**      Called By:
**              mpaconfig()
**      Calls:
**
** RETURNS:     0  - Success
**              ~0 - Failure
**
*****************************************************************************/
int
mpa_init_dev (struct acb *acb)
{
	int rc;

	/*
	** pin the driver code
	*/
	if (mpaopened++ == 0) {
		pincode(mpaconfig);
	}

	/*
	** Initialize the adapter structure
	*/
	acb->act_xmit_head=acb->act_xmit_tail = NULL;
	acb->act_recv_head=acb->act_recv_tail = NULL;
	acb->act_stat_head=acb->act_stat_tail = NULL;
	acb->act_irpt_head=acb->act_irpt_tail = NULL;
	acb->act_dma_head=acb->act_dma_tail = NULL;
	acb->hold_recv = NULL;
	acb->xmit_free = NULL;
	acb->recv_free = NULL;
	acb->stat_free = NULL;
	acb->irpt_free = NULL;
	acb->dma_free = NULL;

	acb->mbuf_event = EVENT_NULL;
	acb->xmitbuf_event = EVENT_NULL;
	acb->irptbuf_event = EVENT_NULL;
	acb->dmabuf_event = EVENT_NULL;
	acb->op_rcv_event = EVENT_NULL;

	/*
	**  Allocate memory for the resource chains...
	**  recv_elems, irpt_elems, stat_elems, dma_elems, xmit_elems.
	*/
	if( (rc = alloc_resources(acb)) ) {
	      mpa_term_dev(acb);
	      return rc;
	}

	/*
	** Install interrupt handler for level 3.
	*/
	acb->caih_struct.next            = NULL;
	acb->caih_struct.handler         = mpaintr;
	acb->caih_struct.bus_type        = DDS.bus_type;
	acb->caih_struct.level           = 3;
	acb->caih_struct.flags           = 0;
	acb->caih_struct.priority        = DDS.intr_priority;
	acb->caih_struct.bid             = DDS.bus_id;
	acb->caih_struct.i_count         = 0;

	if (i_init(&(acb->caih_struct)) != 0) {
		mpa_term_dev( acb );
		return EBUSY;
	}
	acb->flags |= MPAIINSTALL3;   /* interrupt handler 3 installed */

	/*
	** Install off-level interrupt handler.
	*/
	acb->ofl.next            = NULL;
	acb->ofl.handler         = mpa_offlvl;
	acb->ofl.bus_type        = BUS_NONE;
	acb->ofl.flags           = 0;
	acb->ofl.level           = INT_OFFL1;
	acb->ofl.priority        = INTOFFL1;
	acb->ofl.bid             = DDS.bus_id;
	acb->ofl.i_count         = 0;

	/*
	** Acquire a DMA channel to use
	*/
	acb->dma_channel = d_init(DDS.dma_lvl, (DMA_SLAVE|MICRO_CHANNEL_DMA),
		DDS.bus_id);
	if (acb->dma_channel == DMA_FAIL) {
		mpa_term_dev(acb);
		return EBUSY;
	}
	acb->flags |= MPADMACHAN;        /* DMA channel acquired */

	return 0;
} /* mpa_init_dev() */


/*****************************************************************************
** NAME:     mpa_term_dev
**
** FUNCTION:    Disable the adapter's interrupts, remove the interrupt
**              handler, unpin the code, and release the DMA channel.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environment only.
**
** NOTES:
**      Inputs:
**              pointer to a channel adapter structure
**      Outputs:
**              return status
**      Called By:
**              mpaclose()
**      Calls:
**              free_all_resources(),  unpincode()
**
** RETURNS:     0 - Success
**
*****************************************************************************/
int
mpa_term_dev (struct acb *acb)
{
	int spl;


	DISABLE_INTERRUPTS(spl);

	/*
	** Free all the resources.
	*/
	free_all_resources(acb);

	/*
	** unpin the driver code
	*/
	if (mpaopened == 0 || --mpaopened == 0) {
		unpincode(mpaconfig);
	}

	ENABLE_INTERRUPTS(spl);
	return 0;
} /* mpa_term_dev() */


/*****************************************************************************
** NAME: alloc_acb
**
** FUNCTION: Allocates memory for acb structure and puts into a linked list.
**      Also allocates sub-structures for dds and timer.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environment only.
**
** NOTES: Assumes we have the global lock...
**
**    Input:
**              nothing
**    Output:
**              pointer to the allocated channel adapter structure
**    Called From:
**              mpaconfig()
**    Calls:
**              xmalloc() bzero()
**
** RETURNS:     pointer to the allocated acb structure - Success
**              NULL - allocation failed
**
*****************************************************************************/
alloc_acb ()
{
	struct acb *acb;

	/*
	** allocate acb structure and put into linked list
	** This will only work for two adapter structs.
	*/
	if ((acb = KMALLOC(struct acb)) == NULL)
		return((struct acb *) NULL);

	bzero((caddr_t)acb, sizeof(struct acb ));

	if (acbheader) {               /* add the second adpater struct */
		acb->next = acbheader;
	}
	else {                         /* put on the first adpater struct */
		acb->next = NULL;
	}
	acbheader = acb;

	return(acb);
} /* alloc_acb() */



/*****************************************************************************
** NAME:        free_acb
**
** FUNCTION:    Finds the given channel adapter structure and frees it.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environment only.
**
** NOTES:
**    Input:
**              Pointer to a channel adapter structure
**    Output:
**              status code
**    Called From:
**              mpaconfig()
**    Calls:
**              xmalloc()
**
** RETURNS:
**              0 - Success
**              EINVAL - couldn't find 'acb' structure to free
**              ENOMEM - KFREE() (xmfree()) failed to free allocated memory
**
*****************************************************************************/
int     free_acb(struct acb *acb)
{
	struct acb **nextptr;

	for (nextptr = &acbheader; *nextptr != acb; nextptr = &(*nextptr)->next) {
		if (*nextptr == NULL)
			return(EINVAL);
	}
	*nextptr = acb->next;

	KFREE(acb);
	return 0;
} /* free_acb() */


/*****************************************************************************
** NAME:        get_acb
**
** FUNCTION:    Searches for and returns the acb structure associated with the
**              minor device number.
**
** EXECUTION ENVIRONMENT:       Can be called from the process environment only.
**
** NOTES:
**    Input:
**              Major/Minor device numbers
**    Output:
**              Pointer to a channel adapter structure
**    Called From:
**
**    Calls:
**              nothing...
**
** RETURNS:     pointer to a channel adapter structure - Success
**              NULL - couldn't find the given channel adapter structure
**
*****************************************************************************/

get_acb(dev_t dev)
{
	struct acb *acb;

	for (acb = acbheader; acb != NULL; acb = acb->next) {
		if (minor(acb->dev) == dev)
			break;
	}
	return(acb);
} /* get_acb() */


/*****************************************************************************
** NAME:     alloc_resources
**
** FUNCTION:    Allocates memory for and builds free list chain for
**              recv_elems, irpt_elems, stat_elems, dma_elems, xmit_elems.
**
** EXECUTION ENVIRONMENT:   Can be called from the process environment only.
**
** NOTES:
**      Inputs:
**              pointer to a channel adapter structure
**      Outputs:
**              return status
**      Called By:
**              mpa_init_dev()
**      Calls:
**
** RETURNS:     0 - Success
**
*****************************************************************************/
int
alloc_resources (struct acb *acb)
{
	irpt_elem_t     *irpt_elem_p;
	irpt_elem_t     *tmp_irpt_p;
	recv_elem_t     *recv_elem_p;
	recv_elem_t     *tmp_recv_p;
	xmit_elem_t     *xmit_elem_p;
	xmit_elem_t     *tmp_xmit_p;
	dma_elem_t      *dma_elem_p;
	dma_elem_t      *tmp_dma_p;
	stat_elem_t     *tmp_stat_p;
	stat_elem_t     *stat_elem_p;
	int             i;


	/*
	 * Allocate memory for irpt q structures and put them on a
	 * free list attached to acb.
	*/

	for(i=0; i<MAX_IRPT_QSIZE; i++) {
	   if ((irpt_elem_p = KMALLOC(irpt_elem_t)) == NULL) {
	     free_open_struct(acb);
	     return (ENOMEM);
	   }
	   bzero(irpt_elem_p,sizeof(irpt_elem_t));
	   if (acb->irpt_free == NULL) acb->irpt_free=irpt_elem_p;
	   else {
		tmp_irpt_p = acb->irpt_free;
		while(tmp_irpt_p->ip_next) tmp_irpt_p=tmp_irpt_p->ip_next;
		tmp_irpt_p->ip_next=irpt_elem_p;
	   }
	}
	/*
	 * Allocate memory for recv q structures and put them on a
	 * free list attached to acb.
	*/

	for(i=0; i<MAX_RECV_QSIZE; i++) {
	   if ((recv_elem_p = KMALLOC(recv_elem_t)) == NULL) {
	     free_open_struct(acb);
	     return (ENOMEM);
	   }
	   bzero(recv_elem_p,sizeof(recv_elem_t));
	   if (acb->recv_free == NULL) acb->recv_free=recv_elem_p;
	   else {
		tmp_recv_p = acb->recv_free;
		while(tmp_recv_p->rc_next) tmp_recv_p=tmp_recv_p->rc_next;
		tmp_recv_p->rc_next=recv_elem_p;
	   }
	}
	/*
	 * Allocate memory for xmit q structures and put them on a
	 * free list attached to acb.
	*/

	for(i=0; i<MAX_XMIT_QSIZE; i++) {
	   if ((xmit_elem_p = KMALLOC(xmit_elem_t)) == NULL) {
		free_open_struct(acb);
		return (ENOMEM);
	   }
	   bzero(xmit_elem_p,sizeof(xmit_elem_t));
	   if (acb->xmit_free == NULL) acb->xmit_free=xmit_elem_p;
	   else {
		tmp_xmit_p = acb->xmit_free;
		while(tmp_xmit_p->xm_next) tmp_xmit_p=tmp_xmit_p->xm_next;
		tmp_xmit_p->xm_next=xmit_elem_p;
	   }
	}
	/*
	 * Allocate memory for dma q structures and put them on a
	 * free list attached to acb.
	*/

	for(i=0; i<MAX_DMA_QSIZE; i++) {
	   if ((dma_elem_p = KMALLOC(dma_elem_t)) == NULL) {
	     free_open_struct(acb);
	     return (ENOMEM);
	   }
	   bzero(dma_elem_p,sizeof(dma_elem_t));
	   if (acb->dma_free == NULL) acb->dma_free=dma_elem_p;
	   else {
		tmp_dma_p = acb->dma_free;
		while(tmp_dma_p->dm_next) tmp_dma_p=tmp_dma_p->dm_next;
		tmp_dma_p->dm_next=dma_elem_p;
	   }
	}

	/*
	 * Allocate memory for stat q structures and put them on a
	 * free list attached to acb.
	*/

	for(i=0; i<MAX_STAT_QSIZE; i++) {
	   if ((stat_elem_p = KMALLOC(stat_elem_t)) == NULL) {
	     free_open_struct(acb);
	     return (ENOMEM);
	   }
	   bzero(stat_elem_p,sizeof(stat_elem_t));
	   if (acb->stat_free == NULL) acb->stat_free=stat_elem_p;
	   else {
		tmp_stat_p = acb->stat_free;
		while(tmp_stat_p->st_next) tmp_stat_p=tmp_stat_p->st_next;
		tmp_stat_p->st_next=stat_elem_p;
	   }
	}
	return 0;
}  /* alloc_resources() */




