static char sccsid[] = "@(#)63	1.8  src/bos/kernext/dlc/head/dlcintr.c, sysxdlcg, bos411, 9428A410j 2/9/94 14:18:33";
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: dlcintr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
 * NAME: dlcintr
 *                                                                    
 * FUNCTION: Dlcintr handles the data buffers passed up by the protocol specific
 *  code.  The data is passed up in a mbuf, and the mbuf is placed on a list 
 *  headed by the appropriate channel id.  The mbuf is taken off the list, when
 *  the user makes a call to dlcread.  The routine will wake up a user sleeping
 *  on the read or the read select.
 *                                                                    
 * EXECUTION ENVIRONMENT: Dlcintr is called by the kproc started by dlcmpx.
 *                                                                     
 * RETURNS: DLC_OK
 */  

/* defect 122577 */
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
/* end defect 122577 */



#include        "dlcadd.h"

dlcintr(buf,ext,mpx)
struct mbuf        *buf;
struct dlc_io_ext  *ext;
struct dlc_chan    *mpx;
{
ulong              i;
ulong              rc;
struct mbuf        *next;
struct mbuf        *tmp;


/* defect 122577 */
  simple_lock(&mpx->lock);
/* end defect 122577 */



if ((tmp = m_get(M_WAIT,MT_PCB))==(struct mbuf *)DLC_NULL)
{


/* defect 122577 */
  simple_unlock(&mpx->lock);
/* end defect 122577 */
	
	return(ENOMEM);
}


bcopy(ext,mtod(tmp,caddr_t),sizeof(struct dlc_io_ext));

tmp->m_next = buf;
tmp->m_nextpkt = (struct mbuf *)DLC_NULL;

next = mpx->readlist;
if (next == (struct mbuf *) DLC_NULL) 
	mpx->readlist = tmp;
	else 
	{
		while (next->m_nextpkt != (struct mbuf *)DLC_NULL)
			next = next->m_nextpkt;
		next->m_nextpkt = tmp;
	}

if (mpx->state & S_READ)
{
	mpx->state &= ~S_READ;
	e_wakeup((int *) &mpx->readsleep);
}

if (mpx->revents & DPOLLIN)
{
	selnotify(mpx->dev, mpx, DPOLLIN);
	mpx->revents = DLC_NULL;
}


/* defect 122577 */
  simple_unlock(&mpx->lock);
/* end defect 122577 */


return(DLC_FUNC_OK);
}
