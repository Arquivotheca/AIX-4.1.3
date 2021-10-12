static char sccsid[] = "@(#)62	1.6  src/bos/kernext/dlc/head/dlcinte.c, sysxdlcg, bos411, 9428A410j 2/9/94 14:17:58";
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: dlcinte
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
 * NAME: dlcinte
 *                                                                    
 * FUNCTION: Dlcinte handles the exceptions passed up by the protocol specific 
 *  code.  The exception is put in an mbuf and the mbuf on a list headed in the
 *  appropriate channel id.  The mbuf is taken from the list and given to the 
 *  user when the user makes a dlcioctl (GET_EXCEPTION) call.  The routine will
 *  wake up any user sleeping on the exception select.
 *                                                                    
 * EXECUTION ENVIRONMENT: Dlcinte is called by a kproc started by dlcmpx.
 *                                                                     
 * RETURNS: DLC_OK
 */  


/* defect 122577 */
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
/* end defect 122577 */


#include        "dlcadd.h"

dlcinte(buf,mpx)
struct dlc_getx_arg *buf;
struct dlc_chan     *mpx;
{
struct mbuf        *next;
struct mbuf        *tmp;
ulong              rc;

/**************************************************************************/
/*                                                                        */
/*     The channel passed in is an addition none of the kernel users know */
/* about.  It is passed and is needed, but not for the general case.  The */
/* length problem in dlcintr does not exist due to the set size of the    */
/* buffer passed.                                                         */
/*                                                                        */
/**************************************************************************/

/* defect 122577 */
  simple_lock(&mpx->lock);
/* end defect 122577 */



/**************************************************************************/
/*                                                                        */
/*     Get an mbuf to put the exception buffer in, copy it in and add it  */
/* to the tail of the list (preserving the order the buffers arrived in)  */
/*                                                                        */
/**************************************************************************/

if ((tmp = m_get(M_WAIT,MT_PCB))==(struct mbuf *)DLC_NULL)
{

/* defect 122577 */
  simple_unlock(&mpx->lock);
/* end defect 122577 */


	return(ENOMEM);
}
bcopy(buf,tmp->m_dat,sizeof(struct dlc_getx_arg));
tmp->m_nextpkt = (struct mbuf *)DLC_NULL;
next = mpx->exceptlist;
if (next == (struct mbuf *)DLC_NULL) 
	mpx->exceptlist = tmp;
	else 
	{
		while (next->m_nextpkt != (struct mbuf *)DLC_NULL)
			next = next->m_nextpkt;
		next->m_nextpkt = tmp;
	}

/**************************************************************************/
/*                                                                        */
/*    If a select is waiting then the pid is stored in the channel and    */
/* the exception will issue a wake to the select.                         */
/*                                                                        */
/**************************************************************************/

if (mpx->revents & DPOLLPRI)
{
	selnotify(mpx->dev, mpx, DPOLLPRI);
	mpx->revents = DLC_NULL;
}

/* defect 122577 */
  simple_unlock(&mpx->lock);
/* end defect 122577 */


return(DLC_OK);
}
