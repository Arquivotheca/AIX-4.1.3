static char sccsid[] = "@(#)78        1.1  src/bos/kernext/dmpa/dmpa_util.c, sysxdmpa, bos411, 9428A410j 4/30/93 12:49:26";
/*
 *   COMPONENT_NAME: (SYSXDMPA) MP/A DIAGNOSTICS DEVICE DRIVER
 *
 *   FUNCTIONS: PioGet
 *		PioPut
 *		async_status
 *		dma_request
 *		free_dma_elem
 *		free_irpt_elem
 *		free_recv_elem
 *		free_xmit_elem
 *		que_command
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


#include <sys/adspace.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <errno.h>
#include <stddef.h>
#include <sys/except.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/sysdma.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/errids.h>
#include <sys/timer.h>
#include <sys/xmem.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>

#include <sys/dmpauser.h>
#include <sys/dmpadd.h>



/*---------------------   Q U E _ C O M M A N D   ----------------------*/
/*                                                                      */
/*  NAME: que_command                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Issues a command to the adapter by placing it on the            */
/*      adapter's command location then sends the parms specified.      */
/*      The caller must supply a cmd_phase_t structure with the         */
/*      command and parms and number of parms specified.                */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    cmd_phase_t                                     */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*      EIO     PIO error occurred.                                     */
/*      EDEADLK The adapter is not responding it must be dead.          */
/*      0       If the queue succeeded.                                 */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                               i_disable,PioGet,                      */
/*                               i_enable,PioPut,mpalog                */
/*                                                                      */
/*                                                                      */
/*----------------------------------------------------------------------*/

int que_command (
		 struct acb  *acb )  /* pointer to adapter control block */
{
	int             i;             /* for loop counter               */
	int             rc;            /* return code for this routine   */
	int             cnt=0;         /* counter to prevent system hang */
	int             spl;           /* used for disabeling interrupts */


	DISABLE_INTERRUPTS(spl);


	/*
	**  Wait for command busy = 0  in the 8273 status register.
	**  This loops will exit if CBSY bit does not drop within 500 tries.
	**  This limit should avoid system hangs with interrupts disabled.
	**  Set rc = 0xFF initially to insure at least one pass through the
	**  loop.
	*/
	rc = 0xFF;

	while((rc & CBSY) && ++cnt<500) {
	    if( (rc=PIO_GETC( acb,  RD_STAT_OFFSET)) == -1) {
	       ENABLE_INTERRUPTS(spl);
	       return EIO;                       /* pio error occurred */
	    }
	}
	if(cnt==500) {
	   /*
	   **  If we waited 500 trys to see busy bit drop, assume the
	   **  adapter is down and exit with EDEADLK rc.
	   */
	   ENABLE_INTERRUPTS(spl);
	   return(EDEADLK);                      /* avoid system hang */
	}

	/*
	** Write the command to the 8273 command register, now that the
	** command reg is free. acb->cmd_parms.cmd..has the command value
	** as set by the called of que_command().
	*/
	if(PIO_PUTC( acb, WR_CMD_OFFSET, acb->cmd_parms.cmd)==-1) {
	   ENABLE_INTERRUPTS(spl);
	   return EIO;                           /* pio error occurred */
	}

	/*
	/* write the parameters to the 8273 parameter register.
	** NOTE: acb->cmd_parms.parm_count is set by the caller.
	**       it has the number of parms to write for this commnad.
	**       acb->cmd_parms.parm[i].. has the value of each parm.
	*/
	for(i=0; i<acb->cmd_parms.parm_count; i++) {
	     /*
	     **  Wait for free parm reg, CPBF bit = 0 in 8273 status reg.
	     **  Set rc = 0xFF initially to insure one pass through the
	     **  loop.
	     */
	     rc = 0xFF;

	     while((rc & CPBF) && ++cnt<500) {
		if( (rc=PIO_GETC( acb, RD_STAT_OFFSET)) == -1) {
		   ENABLE_INTERRUPTS(spl);
		   return EIO;                   /* pio error occurred */
		}
	     }
	     if(cnt==500) {
		ENABLE_INTERRUPTS(spl);
		return(EDEADLK);                 /* avoid system hang */
	     }

	     /*
	     ** Now that the parm reg is free , write parm # i
	     */
	     if(PIO_PUTC(acb, WR_PARM_OFFSET, acb->cmd_parms.parm[i])==-1) {
		ENABLE_INTERRUPTS(spl);
		return EIO;                      /* pio error occurred */
	     }
	}

	/*
	**  For immediate commands a result will be returned. If the
	**  caller expects a result, wait for it and return it to him
	**  in acb->cmd_parms.result.
	*/
	if(acb->cmd_parms.flag&RETURN_RESULT) {
	     /*
	     **  Wait for result available bit in status reg.
	     **  In this case we are waiting for a bit to come on so
	     **  set rc = 0x00 to insure at least one pass through the loop.
	     */
	     rc = 0x00;

	     while(!(rc & CRBF) && ++cnt<500) {
		if( (rc=PIO_GETC( acb, RD_STAT_OFFSET)) == -1) {
		   ENABLE_INTERRUPTS(spl);
		   return EIO;                   /* pio error occurred */
		}
	     }

	     if(cnt==500) {
		ENABLE_INTERRUPTS(spl);
		return(EDEADLK);                 /* avoid system hang */
	     }

	     /*
	     ** Now read the result reg, since the result bit came on.
	     */
	     if( (rc=PIO_GETC( acb, RD_RES_OFFSET)) == -1) {
		   ENABLE_INTERRUPTS(spl);
		   return EIO;                   /* pio error occurred */
	     }
	     /* set the result value for the caller */
	     acb->cmd_parms.result = (uchar) rc;
	}

	ENABLE_INTERRUPTS(spl);
	return( 0 );                            /* return successful */
}


/*--------------------- A S Y N C _ S T A T U S ------------------------*/
/*                                                                      */
/*  NAME: async_status                                                  */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Report status to the specified open structure.                  */
/*                                                                      */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    acb                                             */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*           0 ------ Success                                           */
/*           ENOMEM - Could not allocate a status queue element         */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                               bzero, i_disable, i_enable,            */
/*                               selnotify                              */
/*                                                                      */
/*----------------------------------------------------------------------*/

int
async_status (
	struct acb *acb,           /* pointer to adapter control block */
	int code,                  /* cio_stat_blk_t       code        */
	int opt0,                  /* cio_stat_blk_t option[0]         */
	int opt1,                  /* cio_stat_blk_t option[1]         */
	int opt2,                  /* cio_stat_blk_t option[2]         */
	int opt3)                  /* cio_stat_blk_t option[3]         */
{
	cio_stat_blk_t kstat_blk;  /* status structure from comio.h    */
	stat_elem_t *statp;        /* pointer to mpadd status element  */
	stat_elem_t *tp;           /* pointer to mpadd status element  */
	int spl;                   /* used for disable and enable irpts*/


	/*
	**  If the device is not open return invalid command
	*/
	if( !(OPENP.op_flags & OP_OPENED) ) return EINVAL;

	/*
	**  Handle the kernel caller.
	*/
	if (OPENP.op_mode & DKERNEL) {
		bzero(&kstat_blk, sizeof(cio_stat_blk_t));
		kstat_blk.code          = (ulong)code;
		kstat_blk.option[0]     = (ulong)opt0;
		kstat_blk.option[1]     = (ulong)opt1;
		kstat_blk.option[2]     = (ulong)opt2;
		kstat_blk.option[3]     = (ulong)opt3;
		(*(OPENP.mpa_kopen.stat_fn)) (OPENP.mpa_kopen.open_id, &kstat_blk);
		return 0;
	}

	DISABLE_INTERRUPTS(spl);

	/*
	** Get a free status element from the free list.
	*/
	if ((statp = acb->stat_free) == NULL) {
		acb->stats.ds.sta_que_overflow++;
		/*
		**  Set a flag so that this open can be notified of
		**  a lost status element if it is checking for status.
		*/
		OPENP.op_flags |= LOST_STATUS;
		ENABLE_INTERRUPTS(spl);
		return ENOMEM;
	}

	acb->stat_free=statp->st_next;   /* move the free pointer to the  */
					 /* next element on the free list */

	bzero(statp,sizeof(stat_elem_t));  /* clear this element          */

	/*
	**  Set the status q element valuse.
	*/
	statp->stat_blk.code            = (ulong)code;
	statp->stat_blk.option[0]       = (ulong)opt0;
	statp->stat_blk.option[1]       = (ulong)opt1;
	statp->stat_blk.option[2]       = (ulong)opt2;
	statp->stat_blk.option[3]       = (ulong)opt3;

	/*
	** Add the status element to the active list
	*/
	if(acb->act_stat_head==NULL) {  /* if the active q is empty.      */
	    acb->act_stat_head=statp;   /* set act head and tail to the   */
	    acb->act_stat_tail=statp;   /* current new q element          */
	}
	else {                         /* its going on the end of a chain */
	    acb->act_stat_tail->st_next=statp;
	    acb->act_stat_tail=statp;
	}
	statp->st_state |= ST_ACTIVE;       /* mark this element active   */

	/*
	** notify any waiting user process there is status available
	** NOTE: If there are no waiting users and no users ever poll
	**       for status, all the status goes into the bit bucket.
	*/
	if (OPENP.op_select & POLLPRI) {
		OPENP.op_select &= ~POLLPRI;
		selnotify((int)acb->dev, OPENP.op_chan, POLLPRI);
	}

	ENABLE_INTERRUPTS(spl);

	return 0;                         /* return successful  */

} /* async_status() */


/*--------------------- F R E E _ R E C V _ E L E M --------------------*/
/*                                                                      */
/*  NAME: free_recv_elem                                                */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Free up the given recv element. Take it off the active list     */
/*      and put it back on the free list and notify anyone sleeping     */
/*      on a free recv element event.                                   */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    acb, recv_elem_t   (defined in mpadd.h)         */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*           VOID                                                       */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                               startrecv, e_wakeup, i_disable,        */
/*                               i_enable, bzero                        */
/*                                                                      */
/*----------------------------------------------------------------------*/

void
free_recv_elem (
	struct acb *acb,            /* pointer to adapter control block */
	recv_elem_t *recvp)         /* pointer to recv element to free  */
{
	recv_elem_t *tmp_recvp;     /* temp recv element pointer        */
	int                spl;     /* used to disable interrupts       */

	/*
	**  A bad programming error has occurred if recvp is null so
	**  kill the machine at this point.
	*/
	ASSERT(recvp);

	/*
	**  It the state of this recv element is not active it has
	**  already been freed to just return to the caller.
	*/
	if(!(recvp->rc_state&RC_ACTIVE)) {
	    return;
	}

	DISABLE_INTERRUPTS(spl);

	/*
	** Free any mbufs associated with this element if the user
	** is not a kernel mode process.
	** Wake_up any user waiting on mbufs.
	*/
	if ((recvp->rc_mbuf_head) && !(OPENP.op_mode&DKERNEL)) {
		m_freem(recvp->rc_mbuf_head);
		if (acb->mbuf_event != EVENT_NULL)
			e_wakeup(&acb->mbuf_event);
	}

	/*
	** Take the receive element off the active list.
	*/
	if (acb->act_recv_head==recvp)
		acb->act_recv_head =recvp->rc_next;
	else {
	   tmp_recvp = acb->act_recv_head;
	   while( (tmp_recvp->rc_next != recvp) && (tmp_recvp != NULL) )
		  tmp_recvp = tmp_recvp->rc_next;
	   /*
	   **  If tmp_recvp is NULL the recv element has not yet been
	   **  put on the active list so just put it back on the free list
	   */
	   if(tmp_recvp) tmp_recvp->rc_next = recvp->rc_next;
	}
	if (acb->act_recv_head == NULL) acb->act_recv_tail = NULL;

	bzero(recvp,sizeof(recv_elem_t));    /* clear the element */

	/*
	** Put it on the free list
	*/
	if (acb->recv_free == NULL)
	   acb->recv_free = recvp;
	else {
	   tmp_recvp = acb->recv_free;
	   while(tmp_recvp->rc_next != NULL) tmp_recvp=tmp_recvp->rc_next;
	   tmp_recvp->rc_next = recvp;
	}

	/*
	** If the receiver is disabled due to someone running out of
	** recevie elements, start the receiver now that an element
	** is free.
	*/
	if(acb->flags & NEED_RECV_ELEM) {
	   if(!(acb->flags & RECEIVER_ENABLED)) startrecv(acb);
	   acb->flags &= ~NEED_RECV_ELEM;
	}

	/*
	** If anyone is sleeping waiting for recv elements wake them
	** up.
	*/
	if (acb->op_rcv_event != EVENT_NULL)
		e_wakeup(&acb->op_rcv_event);

	ENABLE_INTERRUPTS(spl);


	return;
} /* free_recv_element() */


/*--------------------- F R E E _ D M A _ E L E M ----------------------*/
/*                                                                      */
/*  NAME: free_dma_elem                                                 */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Free up the given dma  element. Take it off the active list     */
/*      and put it back on the free list and notify anyone sleeping     */
/*      on a free dma  element event.                                   */
/*      This routine is called to clean up after every recv or xmit     */
/*      dma. It does the d_complete and counts the recv and xmit dma's. */
/*                                                                      */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    acb, dma_elem_t  (defined in mpadd.h)           */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*           VOID                                                       */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                               e_wakeup, i_disable,                   */
/*                               i_enable, bzero, mpalog, d_complete    */
/*                               startrecv                              */
/*                                                                      */
/*----------------------------------------------------------------------*/

void
free_dma_elem (
	struct acb *acb,            /* pointer to adapter control block */
	dma_elem_t *dmap)           /* pointer to dma element to free   */
{
	dma_elem_t *tmp_dmap;       /* temp dma element pointer.        */
	int         spl;            /* used to disable interrupts       */

	ASSERT(dmap);     /* die here on a bad dma element pointer      */


	if(!(dmap->dm_state)) {     /* don't try to free a free element */
	    return;
	}

	DISABLE_INTERRUPTS(spl);

	if(dmap->dm_state==DM_STARTED) {
	    /*
	    ** Cleanup after the DMA if dma is currently active
	    */
	    d_complete(acb->dma_channel, dmap->dm_flags,
		    dmap->dm_buffer, dmap->dm_length,
		    dmap->dm_xmem, NULL);
	    dmap->dm_state = DM_FREE;

	    if(dmap->dm_req_type == DM_XMIT)
		++acb->stats.ds.xmit_dma_completes;   /* count xmit dma */
	    if(dmap->dm_req_type == DM_RECV)
		++acb->stats.ds.recv_dma_completes;   /* count recv dma */
	}

	/*
	** Get rid of the mark that says theres an outstanding recv dma.
	** being held on the hold recv dma q, if this is a recv dma elem.
	*/
	if(dmap->dm_req_type==DM_RECV) {
	     acb->hold_recv = NULL;

	     acb->flags &= ~RECV_DMA_ON_Q;
	}

	/*
	** Take the dma element off the active list.
	*/
	if (acb->act_dma_head==dmap)
		acb->act_dma_head =dmap->dm_next;
	else {
	   tmp_dmap = acb->act_dma_head;
	   while( (tmp_dmap->dm_next != dmap) && (tmp_dmap != NULL) )
		  tmp_dmap = tmp_dmap->dm_next;
	   /*
	   **  If tmp_dmap is NULL the dma element has not yet been
	   **  put on the active list so just put it back on the free list
	   */
	   if(tmp_dmap) tmp_dmap->dm_next = dmap->dm_next;
	}
	if (acb->act_dma_head == NULL) acb->act_dma_tail = NULL;

	bzero(dmap,sizeof(dma_elem_t));       /* clean the element */

	/*
	** Put it on the free list
	*/
	if (acb->dma_free == NULL)
	   acb->dma_free = dmap;
	else {
	   tmp_dmap = acb->dma_free;
	   while(tmp_dmap->dm_next != NULL) tmp_dmap=tmp_dmap->dm_next;
	   tmp_dmap->dm_next = dmap;
	}

	/*
	**  Was the receiver disabled waiting for dma elements?
	**  If so start the receiver now.
	*/
	if(acb->flags & NEED_DMA_ELEM) {
	   if(!(acb->flags & RECEIVER_ENABLED)) startrecv(acb);
	   acb->flags &= ~NEED_DMA_ELEM;
	}

	/*
	** If anyone is sleeping waiting for dma elements wake them
	** up.
	*/
	if (acb->dmabuf_event != EVENT_NULL)
		e_wakeup(&acb->dmabuf_event);

	ENABLE_INTERRUPTS(spl);

	return;
} /* free_dma_elem() */


/*--------------------- F R E E _ X M I T _ E L E M --------------------*/
/*                                                                      */
/*  NAME: free_xmit_elem                                                */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Free up the given xmit element. Take it off the active list     */
/*      and put it back on the free list and notify anyone sleeping     */
/*      on a free xmit element event.                                   */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    acb, xmit_elem_t   (defined in mpadd.h)         */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*           VOID                                                       */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                                          e_wakeup, i_disable,        */
/*                               i_enable, bzero,                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
void
free_xmit_elem (
	struct acb *acb,       /* pointer to adapter control block      */
	xmit_elem_t *xmitp)    /* pointer to xmit element to free       */
{
	xmit_elem_t *tmp_xmitp;   /* temp xmit element pointer          */
	int         spl;          /* used to disable interrupts         */

	ASSERT(xmitp);          /* die here on bad xmit element pointer */

	if(!(xmitp->xm_state&XM_ACTIVE)) {  /* don't free a free one    */
	    return;
	}

	DISABLE_INTERRUPTS(spl);

	/*
	** Free any mbufs associated with this element, and wakeup anyone
	** waiting ofr mbufs, unless this is kernel process.
	*/
	if (xmitp->xm_mbuf && (OPENP.op_mode & DKERNEL) == 0) {
		m_freem(xmitp->xm_mbuf);
		xmitp->xm_mbuf = NULL;
		if (acb->mbuf_event != EVENT_NULL)
			e_wakeup(&acb->mbuf_event);
	}

	/*
	** Take the xmit element off the active list.
	*/
	if (acb->act_xmit_head==xmitp)
		acb->act_xmit_head =xmitp->xm_next;
	else {
	   tmp_xmitp = acb->act_xmit_head;
	   while( (tmp_xmitp->xm_next != xmitp) && (tmp_xmitp != NULL) )
		  tmp_xmitp = tmp_xmitp->xm_next;
	   /*
	   **  If tmp_xmitp is NULL the xmit element has not yet been
	   **  put on the active list so just put it back on the free list
	   */
	   if(tmp_xmitp) tmp_xmitp->xm_next = xmitp->xm_next;
	}
	if (acb->act_xmit_head == NULL) acb->act_xmit_tail = NULL;

	bzero(xmitp,sizeof(xmit_elem_t));     /* clean the element  */

	/*
	** Put it on the free list
	*/
	if (acb->xmit_free == NULL)
	   acb->xmit_free = xmitp;
	else {
	   tmp_xmitp = acb->xmit_free;
	   while(tmp_xmitp->xm_next != NULL) tmp_xmitp=tmp_xmitp->xm_next;
	   tmp_xmitp->xm_next = xmitp;
	}

	/*
	** If anyone is sleeping waiting for xmit elements wake them
	** up.
	*/
	if (acb->xmitbuf_event != EVENT_NULL)
		e_wakeup(&acb->xmitbuf_event);

	ENABLE_INTERRUPTS(spl);


	return;
} /* free_xmit_element() */


/*--------------------- F R E E _ I R P T _ E L E M --------------------*/
/*                                                                      */
/*  NAME: free_irpt_elem                                                */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Free up the given irpt element. Take it off the active list     */
/*      and put it back on the free list and notify anyone sleeping     */
/*      on a free irpt element event.                                   */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    acb, xmit_elem_t   (defined in mpadd.h)         */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*           VOID                                                       */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                               startrecv  e_wakeup, i_disable,        */
/*                               i_enable, bzero,                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
void
free_irpt_elem (
	struct acb *acb,       /* pointer to adapter control block      */
	irpt_elem_t *irptp)    /* pointer to irpt element to free       */
{
	irpt_elem_t *tmp_irptp;   /* temp irpt element pointer          */
	int         spl;          /* used to disable interrupts         */

	ASSERT(irptp);            /* die here if irptp is bad           */


	if(!(irptp->ip_state&IP_ACTIVE)) {     /* don't free a free one */
	    return;
	}

	DISABLE_INTERRUPTS(spl);

	/*
	** Take the irpt element off the active list.
	*/
	if (acb->act_irpt_head==irptp)
		acb->act_irpt_head =irptp->ip_next;
	else {
	   tmp_irptp = acb->act_irpt_head;
	   while( (tmp_irptp->ip_next != irptp) && (tmp_irptp != NULL) )
		  tmp_irptp = tmp_irptp->ip_next;
	   /*
	   **  If tmp_irptp is NULL the irpt element has not yet been
	   **  put on the active list so just put it back on the free list
	   */
	   if(tmp_irptp) tmp_irptp->ip_next = irptp->ip_next;
	}
	if (acb->act_irpt_head == NULL) acb->act_irpt_tail = NULL;

	bzero(irptp,sizeof(irpt_elem_t));    /* clean the elemnet */

	/*
	** Put it on the free list
	*/
	if (acb->irpt_free == NULL)
	   acb->irpt_free = irptp;
	else {
	   tmp_irptp = acb->irpt_free;
	   while(tmp_irptp->ip_next != NULL) tmp_irptp=tmp_irptp->ip_next;
	   tmp_irptp->ip_next = irptp;
	}

	/*
	**  Was the receiver disabled waiting for irpt elements?
	**  If so start the receiver now.
	*/
	if(acb->flags & NEED_IRPT_ELEM) {
	   if(!(acb->flags & RECEIVER_ENABLED)) startrecv(acb);
	   acb->flags &= ~NEED_IRPT_ELEM;
	}

	/*
	** If anyone is sleeping waiting for irpt elements wake them
	** up.
	*/
	if (acb->irptbuf_event != EVENT_NULL)
		e_wakeup(&acb->irptbuf_event);

	ENABLE_INTERRUPTS(spl);

	return;
} /* free_irpt_element() */

/*---------------------   D M A _ R E Q U E S T   ----------------------*/
/*                                                                      */
/*  NAME: dma_request                                                   */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      This function takes the dma element pointer it is passed        */
/*      and puts it on the dma q. If the request is for XMIT it always  */
/*      goes on the q. If it is for RECV, it goes on only if the q      */
/*      is empty. It then starts the next dma on the q if there is      */
/*      no dma active.                                                  */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from process or interrupt environment.            */
/*                                                                      */
/*  DATA STRUCTURES:    acb, dma_elem_t    (defined in mpadd.h )        */
/*                                                                      */
/*                                                                      */
/*  RETURNS:                                                            */
/*       0 ------ queued successfully                                   */
/*       EINVAL-- if dma element has bad type                           */
/*       EIO----- PIO error occured in que_command                      */
/*       EDEADLK- Card hung occured in que_command                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED:                                         */
/*                               i_disable,d_slave, mpalog              */
/*                               i_enable,que_command                   */
/*                                                                      */
/*                                                                      */
/*----------------------------------------------------------------------*/
int
dma_request (
	struct acb *acb,
	dma_elem_t  *new_dmap)
{

	dma_elem_t  *dmap;
	int         rc=0;
	int         spl;
	int         que_it=0;

	/*
	** If new_dmap is NULL this is an error so ASSERT
	** If new_dmap is not null
	** then add it to the list and start dma if one is not already
	** started.
	*/
	ASSERT(new_dmap);

	DISABLE_INTERRUPTS(spl);

	if ( new_dmap->dm_req_type == DM_XMIT ) {
	     /*
	     ** Set a flag to Put it on the dma q. Always q XMIT requests.
	     */
	     que_it = 1;
	}
	else {
	     /*
	     ** Put RECV on the list, only when the list is empty
	     ** and set a flag to tell the other code that a recv
	     ** is on the q and a recv dma request is pending.
	     */
	     if ( acb->act_dma_head == NULL )  {
		 que_it = 1;
		 DDHKWD2(HKWD_DD_MPADD, MPA_RECV_Q, 0, acb->dev,0x00);
		 acb->flags |= RECV_DMA_ON_Q;
	     }
	}

	if(que_it)  {
	     if(acb->act_dma_head==NULL) {  /* its first one */
		 acb->act_dma_head=new_dmap;
		 acb->act_dma_tail=new_dmap;
	     }
	     else {   /* Theres already at least one on the chain */
		      /* so put this one on the end               */
		 acb->act_dma_tail->dm_next=new_dmap;
		 acb->act_dma_tail=new_dmap;
	     }
	}

	/*
	** Now check the head of the q to see if theres one to start
	*/
	if ((dmap = acb->act_dma_head) == NULL) {
		ENABLE_INTERRUPTS(spl);
		return 0;
	}

	switch (dmap->dm_state) {


	case DM_STARTED:
		break;

	case DM_READY: {                /* start a DMA */
		/*
		** Set up the DMA channel for the transfer
		** d_slave is a void function so we are working
		** on faith here.
		*/
		d_slave(acb->dma_channel, dmap->dm_flags,
			dmap->dm_buffer,dmap->dm_length, dmap->dm_xmem);
		dmap->dm_state = DM_STARTED;

		switch (dmap->dm_req_type) {
		case DM_XMIT:
			/* send MPA xmit command */
			acb->cmd_parms.cmd=XMIT_CMD;
			acb->cmd_parms.parm[0]=dmap->dm_length;
			acb->cmd_parms.parm[1]=(dmap->dm_length>>8);
			if(acb->state.oper_mode_8273&SET_BUFFERED_MODE) {
			   acb->cmd_parms.parm[2]=dmap->adr;
			   acb->cmd_parms.parm[3]=dmap->cntl;
			   acb->cmd_parms.parm_count=4;
			}
			else acb->cmd_parms.parm_count=2;
			rc=que_command(acb);
			break;
		case DM_RECV:
			/*
			**  Now kick off a read command..
			*/
			if(!(acb->flags & RECEIVER_ENABLED)) {
			   if(acb->station_type & PRIMARY) {
			      acb->cmd_parms.cmd=GEN_RECEIVE_CMD;
			      acb->cmd_parms.parm[0]=MAX_FRAME_SIZE;
			      acb->cmd_parms.parm[1]=(MAX_FRAME_SIZE>>8);
			      acb->cmd_parms.parm_count=2;
			   }
			   else {
			      acb->cmd_parms.cmd=SEL_RECEIVE_CMD;
			      acb->cmd_parms.parm[0]=MAX_FRAME_SIZE;
			      acb->cmd_parms.parm[1]=(MAX_FRAME_SIZE>>8);
			      acb->cmd_parms.parm[2]=acb->station_addr;
			      acb->cmd_parms.parm[3]=acb->station_addr;
			      acb->cmd_parms.parm_count=4;
			   }
			   if( (rc=que_command(acb)) ) break;

			   DDHKWD2(HKWD_DD_MPADD, MPA_RECV_ENAB, 0, acb->dev,0x00);
			   acb->flags |= RECEIVER_ENABLED;
			}
			break;
		default:
			rc = EINVAL;
			break;

		} /* end switch base on dm_req_type */
	} break;

	default:
		rc = EINVAL;
		break;

	} /* END SWITCH  based on dm_state */

	ENABLE_INTERRUPTS(spl);
	if (rc) free_dma_elem(acb, dmap);
	return(rc);

} /* dma_request() */
/*======================================================================*/
/*                        PIO ACCESS PRIMITIVES                         */
/*======================================================================*/

/*
 *  All character, word, and long programmed-I/O (PIO) operations
 *  to/from the adapter are handled here.  These primitives are not
 *  typically called directly -- the PIO accessor macros in mpadd.h
 *  resolve at compile time to calls to these routines.
 */

/*---------------------------  P I O G E T  ----------------------------*/
/*                                                                      */
/*  NAME: PioGet                                                        */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Performs a PIO read of a byte, short, reversed short, long,     */
/*      or reversed long depending on the type of access specified.     */
/*      If an IO exception occurs during access, it is retried several  */
/*      times -- a -1 is returned if the retry limit is exceeded.       */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:                                                            */
/*      -1      If pio error, value of the read otherwise.              */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: setjumpx, longjumpx, clrjumpx,          */
/*                              BUS_GETC, BUSIO_ATT, BUSIO_DET          */
/*                                                                      */
/*----------------------------------------------------------------------*/

PioGet (struct acb      *acb,
	caddr_t         addr)                  /* address to read from */
{
	int             rc;
	label_t         jump_buf;               /* jump buffer */
	volatile int    retry = PIO_RETRY_COUNT;
	ulong           iob;

	iob = MPA_BUSIO_ATT;
	while (TRUE) {                          /* retry loop */
	    /*                                                            */
	    /*  Set up for context save by doing setjumpx.  If it returns */
	    /*  zero, all is well; otherwise, an exception occurred.      */
	    /*                                                            */
	    if ((rc = setjmpx(&jump_buf)) == 0) {
		if (retry--) {                  /* retry? */
		    /*                                                   */
		    /*  If retries have not been used up, do the read.   */
		    /*  Select the accessor according to the access      */
		    /*  type.                                            */
		    /*                                                   */
		    rc = (int)BUS_GETC( iob + addr );
		    break;                      /* exit retry loop */
		} else {
		    /*                                                   */
		    /*  Out of retries, so return an error.              */
		    /*                                                   */
		    rc = -1;
		    break;
		}
	    } else {
		/*                                                       */
		/*  An exception has occurred or reoccurred -- if it is  */
		/*  a PIO error, simply retry; else, it is an exception  */
		/*  not handled here so longjmpx to the next handler on  */
		/*  the stack.                                           */
		/*                                                       */
		if (rc != EXCEPT_IO) {
		    longjmpx(rc);
		}
	    }
	}
	/*  Out of retry loop -- remove jump buffer from exception       */
	/*  stack and return.                                            */
	/*                                                               */
	clrjmpx(&jump_buf);
	BUSIO_DET(iob);
	return (rc);
}

/*---------------------------  P I O P U T  ----------------------------*/
/*                                                                      */
/*  NAME: PioPut                                                        */
/*                                                                      */
/*  FUNCTION:                                                           */
/*      Performs a PIO write of a byte, short, reversed short, long,    */
/*      or reversed long depending on the type of access specified.     */
/*      If an IO exception occurs during access, it is retried several  */
/*      times -- a -1 is returned if the retry limit is exceeded.       */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Can be called from interrupt level or on a process thread.      */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS:                                                            */
/*      0       Pio write was successful.                               */
/*      -1      Pio error or illegal access type.                       */
/*                                                                      */
/*  EXTERNAL PROCEDURES CALLED: setjumpx, longjumpx, clrjumpx,          */
/*                              BUS_PUTC, BUSIO_ATT, BUSIO_DET          */
/*                                                                      */
/*----------------------------------------------------------------------*/

PioPut (struct acb      *acb,
	caddr_t         addr,                   /* address to write to */
	int             val)                    /* value to write */
{
	int             rc = 0;
	label_t         jump_buf;               /* jump buffer */
	volatile int    retry = PIO_RETRY_COUNT;
	ulong           iob;

	iob = MPA_BUSIO_ATT;
	while (TRUE) {                          /* retry loop */
	    /*                                                            */
	    /*  Set up for context save by doing setjumpx.  If it returns */
	    /*  zero, all is well; otherwise, an exception occurred.      */
	    /*                                                            */
	    if ((rc = setjmpx(&jump_buf)) == 0) {
		if (retry--) {                  /* retry? */
		    /*                                                   */
		    /*  If retries have not been used up, do the write.  */
		    /*  Select the accessor according to the access      */
		    /*  type. If we make it out of the switch, we didn't */
		    /*  get a PIO error.                                 */
		    /*                                                   */
		    BUS_PUTC( (iob + addr), (uchar) val );

		    break;                      /* exit retry loop */
		} else {
		    /*                                                   */
		    /*  Out of retries, so return an error.              */
		    /*                                                   */
		    rc = -1;
		    break;
		}
	    } else {
		/*                                                       */
		/*  An exception has occurred or reoccurred -- if it is  */
		/*  a PIO error, simply retry; else, it is an exception  */
		/*  not handled here so longjmpx to the next handler on  */
		/*  the stack.                                           */
		/*                                                       */
		if (rc != EXCEPT_IO) {
		    longjmpx(rc);
		}
	    }
	}
	/*  Out of retry loop -- remove jump buffer from exception       */
	/*  stack and return.                                            */
	/*                                                               */
	clrjmpx(&jump_buf);
	BUSIO_DET(iob);
	return (rc);
}    /* PioPut() */


