static char sccsid[] = "@(#)95	1.7.3.5  src/bos/kernext/rcm/rcmhsc.c, rcm, bos41J, 9520A_all 5/3/95 11:44:25";

/*
 * COMPONENT_NAME: (rcm) Rendering Context Manager Heavy Switch Controller
 *
 * FUNCTIONS:
 *
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989-1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <lft.h>                    /* includes for all lft related data */
#include <sys/signal.h>			/* SIGstuff */
#include <sys/syspest.h>
#include <sys/sleep.h>
#include "gscsubr.h"                    /* functions */
#include "rcm_mac.h"
#include "rcmhsc.h"                     /* functions */
#include "xmalloc_trace.h"

/* external functions without .h files */
extern pid_t    creatp();

BUGVDEF(dbg_rcmhsc,0)          /* for system call functions */
BUGVDEF(dbg_rcmhsc_int,0)      /* for interrupt/fault call functions */


/*
	The HSC state structure
*/
hscState    hsc;

/* CREATE THE HSC */

/*
    The scheme of having one HSC for all devices means that there are two
    possibilities: (a) the HSC could serialize all requests to finish the
    rcx switches by calling device dependent end_switch functions that
    wait for DMA to finish, via sleep or polling; thus any DMAs are also
    serialized; (b) the HSC could perform switches in parallel by calling
    device dependent "end_start" functions that simply get the DMA going
    and then return; subsequently, the interrupt handler that catches the
    DMA complete interrupt could send a "dma complete" command to the HSC,
    which would then call a device dependent "end_end" function to finish
    off the switch procedure.

    The HSC currently implements the former.


    The hsc might possibly be used to call dma_free in a timely manner
    for processes that have used dma, but not lately.  This would unpin
    any pinning that might affect system performance.
*/


int create_hsc()

{
    int         rc;
    long        rcmhsc();               /* HSC Process function */
    char        process_name[] = "HSCz";

    BUGLPR(dbg_rcmhsc, BUGNFO, ("===== entering create_hsc\n"));
    BUGLPR(dbg_rcmhsc, BUGNFO, ("===== create_hsc, phsc=0x%x\n",
	                        &hsc));

    /* create hsc process */
    hsc.pid = creatp();
    if (hsc.pid == (pid_t)(-1) ) {
	/* log error and return error code */
	BUGPR(("###### create_hsc ERROR can't create process\n"));
	return( EAGAIN );
    }
	
    /* initialize the state structure */
    hsc.flags = 0;
    hsc.Q.head = 0;
    hsc.Q.tail = 0;

    /* get the process id of the creator */
    hsc.i_pid = getpid();

    /* initialize the HSC process */
    rc = initp (hsc.pid, rcmhsc, NULL, 0, process_name );

    /* Was the process initialized */
    if (rc != 0) {
	/* Log error and return error code */
	BUGPR(("###### create_hsc ERROR can't initialize process \n"));
	return (rc);
    }

    /*
     *  Wait until the HSC is ready to process queue entries.
     *
     *  We have the option of being waked up by signals, but we
     *  don't want this, because we don't want to lose any signal
     *  directed to the process leader (us), and we don't know what
     *  to do with them here.
     */
    e_wait (HSC_WAIT_INIT, HSC_WAIT_INIT, 0);

    RCM_ASSERT (hsc.flags & HSC_INIT, 0, 0, 0, 0, 0);

    BUGLPR(dbg_rcmhsc, BUGNFO, ("===== exit create_hsc, pid=%d\n",
	                        hsc.pid));

    return (0);
}



/* kill_hsc

   kill the Heavy Switch Controller

*/

void kill_hsc ()

{
    int             flag, rc;
    hscQE           qe;               /* qe for hsc */

    BUGLPR(dbg_rcmhsc, BUGNFO, ("===== entering kill_hsc\n"));

    RCM_TRACE (0x555, getpid (), 0, 0);

    /* send a terminate command to the HSC */
    qe.command = HSC_COM_TERM;
    if (hsc_enq (&qe)) {
	    BUGPR(("###### kill_hsc ERROR bad nq \n"));
    }

}



/* rcmhsc

   the Heavy Switch Controller

*/

long rcmhsc ()

{
    int             flag, rc;
    hscQEPtr        pqe;

    BUGLPR(dbg_rcmhsc_int, BUGNFO, ("===== entering rcmhsc\n"));

    enter_funnel_nest ();

    /* isolate us from the current group's signals */
    setpgid (0,0);

    /* make the 'init' process our parent */
    setpinit ();

    /* purge any signals received to this point */
    purge_sigs ();

    /* indicate to everyone that hsc is initialized */
    hsc.flags |= HSC_INIT;

    /* allow starter to proceed */
    e_post (HSC_WAIT_INIT, hsc.i_pid);

    /***********************************************/
    /* go into infinite loop, wait for items to be */
    /* queued to the HSC.                          */
    /***********************************************/

#ifdef RCMDEBUG
    printf ("rcmhsc started\n");
#endif

    flag = TRUE;
    while (flag)
    {
	/* get a command */
	rc = hsc_deq (&pqe);
	if (rc) {
	    BUGPR(("###### rcmhsc ERROR cannot get qe, rc=%d \n", rc));
	    RCM_TRACE (0x541, getpid (), 0, 0);
	    flag = FALSE;
	    continue;
	}
	
	BUGLPR(dbg_rcmhsc_int,BUGNFO,
	       ("==== rcmhsc, pdev=0x%x, pold=0x%x, pnew=0x%x\n",
	        pqe->pDev, pqe->pOld, pqe->pNew));

	/* decode the command */
	switch (pqe->command)
	{
	case HSC_COM_SWITCH:
	    /* switch rendering contexts for a device */
	    BUGLPR(dbg_rcmhsc_int,BUGACT,
	           ("====== rcmhsc... about to switch\n"));

	    /* call device specific end switch */
	    RCM_TRACE(0x510,pqe->pOld,pqe->pNew,pqe->seq);

	    /************** WARNING ***********************************/
	    /* do not have to guard the device specific end_switch    */
	    /* function because it is part of the process of switching*/
	    /************** WARNING ***********************************/

	    gsctrace (HSC_END_SWITCH, PTID_ENTRY);
	    rc = pfrm_end_switch (pqe->pNew->pDomain,
					pqe->pOld, pqe->pNew, pqe->seq);
	    gsctrace (HSC_END_SWITCH, PTID_EXIT);

	    break;
	
	case HSC_COM_TERM:
	    /* exit the HSC */

	    RCM_TRACE (0x545, getpid (), 0, 0);

	    rc = 0;
	    flag = FALSE;
	    break;

	default:
	    BUGPR(("###### rcmhsc ERROR bad qe \n"));
	    break;
	
	}

    } /* end while */

    BUGLPR(dbg_rcmhsc_int, BUGNFO, ("===== exit rcmhsc\n"));
#ifdef RCMDEBUG
    printf ("rcmhsc exited, code %d\n", rc);
#endif
    return  rc;
}



/*
    hsc_enq queues a work request to the Heavy Switch Controller
*/

int hsc_enq (pqe)

    hscQEPtr    pqe;    /* ptr to queue element */

{
    int     tail;
    int     old_pri;

    BUGLPR(dbg_rcmhsc_int, BUGNFO, ("===== entering hsc_enq\n"));

    /* make sure there is a queue */
    if (!(hsc.flags & HSC_INIT))
	return (HSC_NO_INIT);

    /********* START Critical Section **********/
    /*
     *  This makes the queue update and event
     *  posting a unitary operation.
     */

    /* disable interrupts */
    old_pri = i_disable(INTMAX);

    /* calculate new tail */
    tail = (hsc.Q.tail + 1) % MAX_HSC_QES;

    /* check for queue overflow */
    if (tail == hsc.Q.head)
    {
        ASSERT (tail != hsc.Q.head);
	/* turn interrupts back on */
	i_enable(old_pri);

	RCM_TRACE (0x551, getpid (), 0, 0);
	BUGPR(("###### hsc_nq ERROR q overflow \n"));

	return (HSC_Q_OVRFLW);
    }
    else
    {
       /* copy in the item to be queued into the ring  */
	hsc.Q.qe[hsc.Q.tail] = *pqe;
	hsc.Q.tail = tail;
    }

    /* wake up HSC */
    e_post (HSC_WAIT_Q, hsc.pid);

    i_enable(old_pri);

    /********* END Critical Section **********/

    BUGLPR(dbg_rcmhsc_int, BUGNFO, ("===== exit hsc_enq\n"));

    return (HSC_Q_SUCC);
}



/*
    hsc_deq dequeues a work request for the Heavy Switch Controller
*/

int hsc_deq (ppqe)

    hscQEPtr    *ppqe;      /* ptr to ptr to queue element */

{

    int     old_pri;

    BUGLPR(dbg_rcmhsc_int, BUGNFO, ("===== entering hsc_deq\n"));

    /********* START Critical Section **********/

    old_pri = i_disable (INTMAX);

    /*
     *  Loop until something appears in the queue.
     *
     *  If nothing is there, sleep until some signal or event
     *  posting activity occurs, then check the queue again.
     */
    while (hsc.Q.head == hsc.Q.tail)
    {
	/*
	 *  When awakened, we must cope with the fact that both
	 *  an e_post and a delivered signal may have arrived.
	 *  Our policy is to throw away any signal, and check
	 *  for any queue entry.
	 *
	 *  E_wait bit mask maintenance should work out OK.  There
	 *  is a question as to what the HSC_WAIT_Q bit will be
	 *  set to when we wake up under the various conditions, but
	 *  we don't have to decide.  It will cause at most one extra
	 *  pass around the loop before sleeping.  It is not possible
	 *  for this function to sleep if something is on the queue.
	 */
	e_wait (HSC_WAIT_Q, HSC_WAIT_Q, EVENT_SIGRET);

	purge_sigs ();
    }

    /* generate qe pointer           */
    *ppqe = &hsc.Q.qe[hsc.Q.head];

    /* update head ptr, not critical, only hsc updates head */
    hsc.Q.head = (hsc.Q.head + 1) % MAX_HSC_QES;

    i_enable(old_pri);

    /********* END Critical Section **********/

    BUGLPR(dbg_rcmhsc_int, BUGNFO, ("===== exit hsc_deq\n"));

    return  (0);
}


int  purge_sigs ()
{
    int  rc;

    while (rc = sig_chk ())
    {
#ifdef RCMDEBUG
	printf ("HSCz pid %d, signal %d purged\n", getpid (), rc);
#endif
    }
}
