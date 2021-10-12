static char sccsid[] = "@(#)65	1.26  src/bos/kernext/dlc/head/dlcmpx.c, sysxdlcg, bos412, 9446B 11/15/94 16:48:16";

/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: dlcmpx
 *
 * ORIGINS: 27
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

/*
 * NAME: dlcmpx
 *                                                                    
 * FUNCTION: Dlcmpx is called by the operating system whenever a open or close
 *  is called.  The mpx routine will allocate and deallocate the channel id and
 *  portcb as necessary.   If the name field == NULL then the call is a 
 *  deallocation.  The channel passed in is deleted and if it is the last 
 *  channel is the last on the port the portcb is deleted.  If the name is 
 *  nonNULL, a channel is created and inited and if it is the first channel on
 *  the port the portcb is done likewise, if not then the portcb's chan_count is
 *  incremented. 
 *                                                                    
 * EXECUTION ENVIRONMENT:  Dlcmpx is called whenever the open or close routines
 *  are called.
 *                                                                     
 * RETURNS: DLC_OK
 */  


/* defect 122577 */
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
/* end defect 122577 */


#include        <sys/fp_io.h>
#include        <sys/fullstat.h>

#include        "dlcadd.h"

struct s_mpx_tab   dlcsum;      /* global table of the channels and adapters */

extern init_proc();

/*ARGSUSED*/
dlcmpx(dev, mpxr, name)
dev_t            dev;
int		*mpxr;
char            *name;
{
#define STANZA_LEN 30
char            device[STANZA_LEN + 5]; /* extra space to be extra careful */
ulong           mask,len,tempdev,i,cb_index,slotnum,rc;
struct fullstat  stat;
struct file     *fp;
char            path[50];
struct dlc_port  *tempcb;
struct dlc_chan  *tempmpx;
struct dlc_chan	*mpx;
struct mbuf 	*buf;
struct mbuf 	*lbuf;    
struct initparms 
{
	struct dlc_port *x;
} initparms;


/**************************************************************************/
/*                                                                        */
/*     The lock on the mpx list is used to control access to the list     */
/* itself not its elements.  The lock by convention must be obtained      */
/* before running the list or altering it.                                */
/*                                                                        */
/**************************************************************************/


/* defect 122577 */
simple_lock(&dlcsum.lock);
/* end defect 122577 */



/**************************************************************************/
/*                                                                        */
/*    If the name field is null then the mpx field holds the address of   */
/* the control block to be deallocted.  By this time the interrupt handler*/
/* has exited and the device handler has closed.                          */
/*                                                                        */
/**************************************************************************/

if (name == (char *) DLC_NULL)
{
	mpx= (struct dlc_chan *)*mpxr;

/**************************************************************************/
/*                                                                        */
/*    After getting the channel list lock (to prevent an open from        */
/* occuring while the channel is closing), put the channel cb in the port */
/* and post the interrupt routine.  If the channel is the last one on the */
/* port then zero out the devt of the device handler in the port cb.      */
/* This is necessary so that an open for that device handler does not     */
/* arrive between the close and the mpx call which frees these control    */
/* blocks.  If the channel is not the only one on the port then the       */
/* devt does not need to be touched as an open arriving does not need to  */
/* restart the port and interrupt handler.                                */
/*                                                                        */
/**************************************************************************/


	/* defect 122577 */
	simple_lock(&mpx->cb->lock);
	/* end defect 122577 */



	mpx -> cb -> kcid = mpx;


	/* defect 122577 */
	simple_unlock(&mpx->cb->lock);
	/* end defect 122577 */




#define ECB_CLOSE 0x08000000

#ifdef DLC_DEBUG
/* <<< THREADS >>> */
	printf("CLOSE pid=%x tid=%x\n",mpx->cb->chpid,mpx->cb->kproc_tid);
/* <<< end THREADS >>> */

#endif
	/* trace the mpx close, defect 167068 */
	static_trace(mpx->cb,"MpxC",mpx->cb->kproc_tid);

/* defect 101311 */
/* defect 115819 */
	/* if the kproc isn't already being terminated */
	if (mpx->cb->term_kproc != TRUE)
/* end defect 115819 */
	{
		mpx->proc_id = EVENT_NULL;

		/* trace the post, defect 167068 */
		static_trace(mpx->cb,"Post",mpx->cb->kproc_tid);

/* <<< THREADS >>> */
		et_post(ECB_CLOSE,mpx->cb->kproc_tid);
/* <<< end THREADS >>> */

		do 
		{
			/* trace the sleep, defect 167068 */
			static_trace(mpx->cb,"Slpb",mpx->proc_id);
			rc = e_sleep((int *)&mpx->proc_id,EVENT_SHORT);
			static_trace(mpx->cb,"Slpe",rc);

		} while (rc != EVENT_SUCC);
	}
/* end defect 101311 */

/* defect 115819 */
/* removed mpx->cb->dev_devt = 0; */
/* defect 115819 */

	/* defect 122577 */
	simple_lock(&mpx->lock);
	/* end defect 122577 */



	while (mpx -> readlist != (struct mbuf *)EMPTY)
	{
		lbuf = mpx -> readlist;
		mpx -> readlist = mpx -> readlist -> m_nextpkt;
		m_freem(lbuf);
	} /* end while */

	while (mpx -> exceptlist != (struct mbuf *)EMPTY)
	{
		lbuf = mpx -> exceptlist;
		mpx -> exceptlist = mpx -> exceptlist -> m_nextpkt;
		m_freem(lbuf);
	} /* end while */

	if (mpx->cb->chan_count == 1)
	{
		xmfree(mpx->cb, pinned_heap);
		unpincode(init_proc);
	}
	else 
	{

		/* defect 122577 */
		simple_lock(&mpx->cb->lock);
		/* end defect 122577 */

		
		mpx->cb->chan_count--;

		/* defect 122577 */
		simple_unlock(&mpx->cb->lock);
		/* end defect 122577 */


		
	}

	if ((mpx->mpx == (struct dlc_chan *)DLC_NULL) && 
	    (mpx->bmpx == (struct dlc_chan *)DLC_NULL))
		dlcsum.chanp = (struct dlc_chan *)DLC_NULL;
		else  /* not the last channel */
		{
		/* copy this channel's backward pointer to the next channel   */
                /* in the list ( only if there is a next channel)             */
		/* copy this channel's forward pointer to the previos channel */
                /* in the list ( only if there is a previos channel )         */

			if (mpx->mpx != (struct dlc_chan *)DLC_NULL)
				mpx->mpx->bmpx = mpx->bmpx;
			if (mpx->bmpx != (struct dlc_chan *)DLC_NULL)
				mpx->bmpx->mpx = mpx->mpx;
			else /* there is no previous channel */
                             /* set the anchor channel pointer to the  */
                             /* next mpx in the list                   */
                             dlcsum.chanp = mpx->mpx;
		}


	/* defect 122577 */
	simple_unlock(&mpx->lock);
	lock_free(&mpx->lock);
	/* end defect 122577 */

 	
	free(mpx);

	/* defect 122577 */
	simple_unlock(&dlcsum.lock);
	/* end defect 122577 */

	/* trace the mpx open completion, defect 167068 */
	static_trace(mpx->cb,"Mpxe",DLC_OK);

	return(DLC_OK);
} 
   
/**************************************************************************/
/*                                                                        */
/*     If the name is not DLC_NULL then the mpx call is a creation.  The mpx  */
/* structure is palloced and all fields which can be filled are.          */
/*                                                                        */
/**************************************************************************/

mpx = (struct dlc_chan *) palloc(sizeof(struct dlc_chan),WORD);


#ifdef DLC_DEBUG
printf("channel = %x\n",mpx);
#endif

/* trace the mpx open, defect 167068 */
static_trace(0,"MpxO",mpx);

if (mpx == (struct dlc_chan *) DLC_NULL)
{

	/* defect 122577 */
	simple_unlock(&dlcsum.lock);
	/* end defect 122577 */

	
	mpxr = (int *)DLC_ERR;

	/* trace the mpx open error, defect 167068 */
	static_trace(0,"Mpx1",ENOMEM);

	return(ENOMEM);
}

/* defect 122577 */
lock_alloc(&mpx->lock, LOCK_ALLOC_PIN, CHANNEL_LOCK, -1);
simple_lock_init(&mpx->lock);
/* end defect 122577 */

dlcsum.tmpx = mpx;


mpx->readsleep = (struct proc *)EVENT_NULL;
mpx->writesleep = EVENT_NULL;
mpx->revents = DLC_NULL;
mpx->dev = dev;
mpx->state = EMPTY;
mpx->saps = EMPTY;
mpx -> readlist = mpx -> exceptlist = (struct mbuf *)EMPTY;

/***************************************************************************/
/*                                                                         */
/*                 Look to see if the adapter is already defined.  If so   */
/*  copy the address of the control block and return, if not palloc a new  */
/*  control block, start an interrupt child, and call open_dev routine of  */
/*  the data link protocol specific code.                                  */
/*                                                                         */
/***************************************************************************/

tempmpx = dlcsum.chanp;
if (tempmpx != EMPTY)
	while ((strcmp(name,tempmpx->cb->namestr)) && (tempmpx -> mpx != EMPTY))
		tempmpx = tempmpx->mpx;
if ((tempmpx != EMPTY) && (!(strcmp(name,tempmpx->cb->namestr))))
{
/***************************************************************************/
/*                                                                         */
/*     Since this device handler has already been opened there already is  */
/* a port control block associated with the device handler, so one only    */
/* needs to increment the chan_count which holds the number of channels    */
/* using the port_cb.  The count is used to decide when one can free the   */
/* port control block.                                                     */
/*                                                                         */
/***************************************************************************/
	

	/* defect 122577 */
	simple_lock(&tempmpx->cb->lock);
	/* end defect 122577 */


	
        if (tempmpx->cb->chan_count >= tempmpx->cb->maxchan)
	{

		/* defect 122577 */
		simple_unlock(&tempmpx->cb->lock);
		simple_unlock(&dlcsum.lock);
		/* end defect 122577 */


		free(mpx);
		/* defect 122577 */
		lock_free(&mpx->lock);
		/* end defect 122577 */

		/* trace the mpx open error, defect 167068 */
		static_trace(tempmpx->cb,"Mpx2",EINVAL);

		return(EINVAL);
	}
	tempmpx->cb->chan_count++;

	/* defect 122577 */
	simple_unlock(&tempmpx->cb->lock);
	/* end defect 122577 */


		
	mpx->cb = tempmpx->cb;
	
	mpx -> mpx = dlcsum.chanp;
	dlcsum.chanp -> bmpx = mpx;
	mpx -> bmpx = (struct dlc_chan *)DLC_NULL;
	dlcsum.chanp = mpx;
	/* defect 122577 */

	simple_unlock(&dlcsum.lock);
	/* end defect 122577 */

}
 
else
{
/***************************************************************************/
/*                                                                         */
/*     If the device handler has not been opened before then palloc a port */
/* control block, initialize its values and connect it to the channel.     */
/*                                                                         */
/***************************************************************************/
	len = len_port_cb();
	tempcb = (struct dlc_port *) xmalloc(len,WORD,pinned_heap);

#ifdef DLC_DEBUG
	printf("PORT CONTROL BLOCK=%x\n",tempcb);
	printf("len port dcl=%d\n",len);
#endif

	if (tempcb == DLC_NULL)
	{
		free(mpx);
	
		/* defect 122577 */
		lock_free(&mpx->lock);
		simple_unlock(&dlcsum.lock);
		/* end defect 122577 */

		
		mpxr = (int *)DLC_ERR;

		/* trace the mpx open error, defect 167068 */
		static_trace(0,"Mpx3",ENOMEM);

		return(ENOMEM);
	}
	rc = pin(tempcb,sizeof(struct dlc_port));
	if (rc == DLC_ERR)
	{
		/* defect 122577 */
		lock_free(&mpx->lock);
		/* end defect 122577 */

		free(mpx);
		free(tempcb);
		/* defect 122577 */
		simple_unlock(&dlcsum.lock);
		/* end defect 122577 */

		
		mpxr = (int *)DLC_ERR;

		/* trace the mpx open error, defect 167068 */
		static_trace(0,"Mpx4",ENOMEM);

		return(ENOMEM);
	}

	/*defect 122577 */
	/* allocate the port control block lock - defect 127690 */
	lock_alloc(&tempcb->lock, LOCK_ALLOC_PIN, PORT_LOCK, PORT_CB_LOCK);
	simple_lock_init(&tempcb->lock);
	/* end defect 122577 */

	bzero(tempcb,len);

	tempcb -> chan_count = 1;
	mpx -> cb = tempcb;
	
	tempcb->maxchan = max_opens();
	strcpy(tempcb->namestr,name);
	tempcb->cid = (struct dlc_chan *) mpx;
	if (pincode(init_proc) != 0)
	{
		/*defect 122577 */
		lock_free(&mpx->lock);
		lock_free(&tempcb->lock);
		/* end defect 122577 */

		free(mpx);
		xmfree(tempcb,pinned_heap);
		/* defect 122577 */
		simple_unlock(&dlcsum.lock);
		/* end defect 122577 */

		
		mpxr = (int *)DLC_ERR;

		/* trace the mpx open error, defect 167068 */
		static_trace(0,"Mpx5",ENOMEM);

		return(ENOMEM);
	}
	init_cb(tempcb);

/***************************************************************************/
/*                                                                         */
/*      Then, since this is the first open to this device handler, we must */
/* create a kernel process to handel upcoming messages.  The process is    */
/* then initialized and kicked off.                                        */
/*                                                                         */
/***************************************************************************/
	rc = creatp();
 
	if ((long)rc < 0)
	{
		/*defect 127690 */
		lock_free(&mpx->lock);
		lock_free(&tempcb->lock);
		/* end defect 127690 */
		free(mpx);
		xmfree(tempcb,pinned_heap);
		unpincode(init_proc);
		/* defect 122577 */
		simple_unlock(&dlcsum.lock);
		/* end defect 122577 */

		mpxr =  (int *)DLC_ERR;

		/* trace the mpx open error, defect 167068 */
		static_trace(0,"Mpx6",ECHILD);

		return(ECHILD);
	}

	tempcb -> chpid = rc;
	mpx->proc_id = rc;
	initparms.x = tempcb;
	rc = initp(tempcb->chpid, init_proc, &initparms, sizeof(initparms), 
		   "dlci");
       
	if (rc != 0)
	{

#ifdef DLC_DEBUG
		printf("ERROR : dlcmpx : initp failed\n");
#endif

		/*defect 127690 */
		lock_free(&mpx->lock);
		lock_free(&tempcb->lock);
		/* end defect 127690 */
		free(mpx);
		xmfree(tempcb,pinned_heap);
		unpincode(init_proc);

		/* defect 122577 */
		simple_unlock(&dlcsum.lock);
		/* end defect 122577 */

		
		mpxr = (int *)DLC_ERR;

		/* trace the mpx open error, defect 167068 */
		static_trace(0,"Mpx7",ENOMEM);

		return(ENOMEM);
	}
/* <<< defect 130901 >>> */
	if /* kproc has not obtained a thread id yet */
	   (tempcb->kproc_tid == 0)
	  {
	    /* issue a delay to swap out the running process */
            delay(10);
	  }
/* <<< end defect 130901 >>> */

	rc = setpri(tempcb -> chpid ,38);

	if (rc < 0)
	{

		/*defect 127690 */
#ifdef DLC_DEBUG
		printf("ERROR : dlcmpx : setpri failed=%d\n", rc);
#endif
		/* if the kproc isn't already being terminated */
		if (mpx->cb->term_kproc != TRUE)
		{
			mpx->proc_id = EVENT_NULL;
			et_post(ECB_CLOSE,mpx->cb->kproc_tid);

			do 
			{
				rc = e_sleep((int *)&mpx->proc_id,EVENT_SHORT);
			} while (rc != EVENT_SUCC);
		}
		lock_free(&mpx->lock);
		lock_free(&tempcb->lock);
		/* end defect 127690 */

		free(mpx);
		xmfree(tempcb,pinned_heap);
	/*      unpincode(init_proc); */

		/* defect 122577 */
		simple_unlock(&dlcsum.lock);
		/* end defect 122577 */
		
		mpxr = (int *)DLC_ERR;

		/* trace the mpx open error, defect 167068 */
		static_trace(0,"Mpx8",ENOMEM);

		return(ENOMEM);
	}

/** START MULT_PU
    The pr_open call was moved to dlcopen.c so that the open extension is
    completed first.
END MULT_PU**/

	if (tempmpx == EMPTY)
	{
		dlcsum.chanp = mpx;
		mpx -> mpx = mpx -> bmpx = EMPTY;
	}
	else 
	{
		mpx -> mpx = dlcsum.chanp;
		dlcsum.chanp -> bmpx = mpx;
		mpx -> bmpx = (struct dlc_chan *)DLC_NULL;
		dlcsum.chanp = mpx;
	}

	/* defect 122577 */
	simple_unlock(&dlcsum.lock);
	/* end defect 122577 */
	
}
 
*mpxr= (int )mpx;

/* trace the mpx open completion, defect 167068 */
static_trace(mpx->cb,"Mpxe",DLC_OK);

return(DLC_OK);
}
