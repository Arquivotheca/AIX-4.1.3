static char sccsid[] = "@(#)34  1.7  src/bos/kernext/lft/fonts/fsqueue.c, lftdd, bos411, 9434A411a 8/18/94 12:21:48";
/*
 *   COMPONENT_NAME: LFTDD
 *
 * FUNCTIONS: - fsp_enq(): enqueue a command for DD and process by fkproc() 
 *            - fsp_deq(): dequeue a command for fkproc() 
 *
 *
 *   ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <lft.h>
#include <sys/malloc.h>
#include <sys/sleep.h>

/* #define DEBUG   */                   /* to turn on BUGLPR.  Uncomment when*/
                                        /* everything works                  */

#include <sys/syspest.h>                /* debugging flags */
#include <graphics/gs_trace.h>
#include <lft_debug.h> 

BUGVDEF(db_fsqueue,0x0);

/*---------------------------------------------------------------------------
    Fsp_enq:

	Queues a work request to fkproc (graphics kernel process)

   Invoke: i_disable, i_enable, e_post

   Called by: make_shm, umake_shm, kill_fkproc, midintr (mid_intr_font_request)

---------------------------------------------------------------------------*/
fsp_enq (pqe,lftptr)

fkprocQEPtr	pqe;    /* ptr to queue element */
lft_t		*lftptr;

{
    int     tail;
    int     old_pri;
    fkprocState * ptr_fsqueue ; 

    GS_ENTER_TRC1(HKWD_GS_LFT,fsqueue,1,fsp_enq, pqe);
    BUGLPR(db_fsqueue, BUGNFO,("->entering fsp_enq, lft_ptr=%x\n",lftptr));

    ptr_fsqueue = &(lftptr->lft_fkp.fsq); /* pointer to queue */

    BUGLPR(db_fsqueue, BUGNFO,("fsp_enq: old tail=%d, old head =%d\n",ptr_fsqueue->Q.tail,ptr_fsqueue->Q.head));

    /* if we are supposed to wait for ack */
    if (pqe->command & FKPROC_COM_WAIT)
    {
	/* put in the pointer to our per-thread stack wait structure */
	pqe->pfkproc_com_wait = xmalloc (sizeof (struct fkproc_com_wait),
								3, pinned_heap);

	BUGLPR(db_fsqueue, BUGNFO,("fsp_enq: pqe->pfkproc_com_wait 0x%x\n",
							pqe->pfkproc_com_wait));

	if (pqe->pfkproc_com_wait == NULL)
	{
	    lfterr(NULL,"LFTDD", "fsqueue", NULL, 0, LFT_FKPROC_Q_NO_MEM,
								UNIQUE_2);
	    BUGPR(("###### fsp_enq ERROR no mem \n")); /* log overflow error */
            GS_EXIT_TRC1(HKWD_GS_LFT, fsqueue, 1, fsp_enq, FKPROC_Q_NO_MEM);
	    return (FKPROC_Q_NO_MEM);
	}

	pqe->pfkproc_com_wait->done = 0;		/* clear done flag */
	pqe->pfkproc_com_wait->sleep_done = EVENT_NULL;/* init the sleep word */
    }

    /********* START Critical Section **********/

    old_pri = i_disable(INTMAX);        /* disable interrupts */

    /* make sure there is a queue */
    if (!(ptr_fsqueue->flags & FKPROC_INIT))
    {
	i_enable(old_pri);              /* turn interrupts back on  */

/*	lfterr(NULL,"LFTDD", "fsqueue", NULL, 0, LFT_FKPROC_NOTINIT, UNIQUE_1); */
	if (pqe->command & FKPROC_COM_WAIT)	/* release any storage */
	    xmfree (pqe->pfkproc_com_wait, pinned_heap);

        BUGLPR(db_fsqueue,0,("fsp_enq: fkproc isn't init\n"));
        GS_EXIT_TRC0(HKWD_GS_LFT, fsqueue, 1, fsp_enq);
	return(FKPROC_NO_INIT);
    }

    /* if kill cmd, prevent any more enqueueing to this process */
    if ((pqe->command & FKPROC_COM_CMD_MASK) == FKPROC_COM_TERM)
	ptr_fsqueue->flags &= ~FKPROC_INIT;

    /* calculate new tail */
    tail = (ptr_fsqueue->Q.tail + 1) & (MAX_QES - 1);
	                                /* queue_size is power of 2, using   */
	                                /* this value minus 1 will give us a */
	                                /* mask for taking the MOD of tail + */
	                                /* one (for ring wrap).              */

    if (tail == ptr_fsqueue->Q.head)         /* check for queue overflow */
    {
	i_enable(old_pri);              /* turn interrupts back on  */

	if (pqe->command & FKPROC_COM_WAIT)	/* release any storage */
	    xmfree (pqe->pfkproc_com_wait, pinned_heap);

	lfterr(NULL,"LFTDD", "fsqueue", NULL, 0, LFT_FKPROC_Q_OVRFLW, UNIQUE_2);
	BUGPR(("###### fsp_enq ERROR q overflow \n")); /* log overflow error */
        GS_EXIT_TRC1(HKWD_GS_LFT, fsqueue, 1, fsp_enq, FKPROC_Q_OVRFLW);
	return (FKPROC_Q_OVRFLW);
    }

    /* copy in the item to be queued into the ring  */
    ptr_fsqueue->Q.qe[ptr_fsqueue->Q.tail] = *pqe;
    ptr_fsqueue->Q.tail = tail;

    BUGLPR(db_fsqueue, BUGNFO,("fsp_enq: enq OK so wake up dequeue\n"));

    e_post (FKPROC_WAIT_QE, ptr_fsqueue->pid);     /* wake up fkproc    */

    /* if we are supposed to wait for ack */
    if (pqe->command & FKPROC_COM_WAIT)
    {
	while (!pqe->pfkproc_com_wait->done)
	    (void) e_sleep (&pqe->pfkproc_com_wait->sleep_done, EVENT_SHORT);
    }

    i_enable(old_pri);                        /* enable interrupts */

    /********* END Critical Section **********/

    /* we are sure fkproc won't wakeup this sleep cell any more */
    if (pqe->command & FKPROC_COM_WAIT)
	xmfree (pqe->pfkproc_com_wait, pinned_heap);

    BUGLPR(db_fsqueue,BUGNFO, ("===== exit fsp_enq, new tail=%d\n",ptr_fsqueue->Q.tail));

    GS_EXIT_TRC1(HKWD_GS_LFT, fsqueue, 1, fsp_enq, FKPROC_Q_SUCC);

    return (FKPROC_Q_SUCC);
}


/*---------------------------------------------------------------------------
    Fsp_deq:

	Dequeues a work request for kproc to process 

   Pseudo-code:
		 Disable interrupt
		 While queue empty wait
		 get first entry in queue
                 calculate new head pointer to queue 
                 Enable interrupt
                 return data (queue element)  
	
   Invoke: i_disable, i_enable

   Called by: fkproc

---------------------------------------------------------------------------*/
int fsp_deq(pqe,lftptr)

fkprocQE    	*pqe;      /* ptr to queue element */
lft_t		*lftptr;

{

    int     old_pri;
    int rc;
    fkprocState * ptr_fsqueue ;

    GS_ENTER_TRC1(HKWD_GS_LFT,fsqueue,1,fsp_deq, pqe);
    BUGLPR(db_fsqueue, BUGNFO, ("===== entering fsp_deq, lft_ptr=%x\n",lftptr));

    ptr_fsqueue = &(lftptr->lft_fkp.fsq);

    BUGLPR(db_fsqueue, BUGNFO, ("fsp_deq: old head=%d, old tail=%d\n",ptr_fsqueue->Q.head,ptr_fsqueue->Q.tail));

    /********* START Critical Section **********/

    old_pri = i_disable(INTMAX);

    /*
     *  Loop until something appears in the queue.
     *
     *  If nothing is there, sleep until some signal or event
     *  posting activity occurs, then check the queue again.
     */
    while (ptr_fsqueue->Q.head == ptr_fsqueue->Q.tail) 
    {

        BUGLPR(db_fsqueue, BUGNFO, ("fsp_deq: queue is empty so wait\n"));

	/*
	 *  When awakened, we must cope with the fact that both
	 *  an e_post and a delivered signal may have arrived.
	 *  Our policy is to throw away any signal, and check
	 *  for any queue entry.
	 */
	e_wait (FKPROC_WAIT_QE, FKPROC_WAIT_QE, EVENT_SIGRET);

	purge_sigs ();
    }

    /********* END Critical Section **********/

    BUGLPR(db_fsqueue, BUGNFO, ("fsp_deq: queue isn't empty so return data\n"));

    /* return data to caller          */
    *pqe = ptr_fsqueue->Q.qe[ptr_fsqueue->Q.head]; 

    /* update head ptr, not critical, only fkproc updates head */
    ptr_fsqueue->Q.head = (ptr_fsqueue->Q.head + 1) & (MAX_QES - 1);
    i_enable(old_pri);


    BUGLPR(db_fsqueue, BUGNFO, ("-> exit fsp_deq, new head=%d\n",ptr_fsqueue->Q.head));

    /* return */
    GS_EXIT_TRC0(HKWD_GS_LFT, fsqueue, 1, fsp_deq);
    return(0);
}


int  purge_sigs ()
{
    int  rc;

    while (rc = sig_chk ())
    {
    }
}
