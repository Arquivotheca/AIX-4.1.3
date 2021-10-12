static char sccsid[] = "@(#)67	1.11  src/bos/kernext/dlc/head/dlcread.c, sysxdlcg, bos411, 9435A411a 8/29/94 13:05:41";
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: dlcread
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
 * NAME: dlcread
 *                                                                    
 * FUNCTION: Dlcread copies the data from the protocol out of an mbuf on a list
 *  headed by the channel and gives it to the application user.  The mbuf is 
 *  freed from the list.  It is put on the list by the protocol's call to the
 *  dlcintr routine.  If there is no data then if the call is made from a user 
 *  who is blocking the process is put to sleep until data arrives.  If the 
 *  user is nonblocking then the routine returns a DLC_OK.  (the user will  
 *  receive a 0 bytes received). 
 *                                                                    
 * EXECUTION ENVIRONMENT: Dlcread should only be called by a application user.
 *                                                                     
 * RETURNS: DLC_OK
 */  

/* defect 122577 */
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
/* end defect 122577 */

#include        "dlcadd.h"

/*ARGSUSED*/
dlcread(dev,uiop,mpx,ext)
dev_t   dev;
struct uio      *uiop;
struct dlc_chan *mpx;
struct ext_read *ext;
{
ulong 			temp;
ulong			savelen;     /* defect 152850 */
struct dlc_io_ext 	buf;
struct dlc_io_ext 	*tbuf;
struct mbuf 		*lbuf;
ulong 			rc;

/**************************************************************************/
/*                                                                        */
/*     If there is nothing on the list then return a OK unless the        */
/* BLOCKED_READ flag is set in which case the call will sleep until the   */
/* channel has data to return.  e_sleepl is used to keep the lock release */
/* and sleep atomic.  S_READ is released when data is in the channel.     */
/*                                                                        */
/**************************************************************************/

simple_lock(&mpx->lock);  /* defect 122577 */

if (mpx->readlist == (struct mbuf *)DLC_NULL)
{
	if (!(uiop->uio_fmode & (FNDELAY|FNONBLOCK)))
	{
		mpx->state |= S_READ;
		do  
                        if (e_sleep_thread( (int *)&mpx->readsleep,
                             (int *)&mpx->lock, (LOCK_SIMPLE | INTERRUPTIBLE) )
                            != THREAD_AWAKENED)  /* defect 122577 */ 
			{
				simple_unlock(&mpx->lock);  /* defect 122577 */
				return(EINTR);
			}
		while(mpx->state & S_READ);
	}
	else
	{
		simple_unlock(&mpx->lock);  /* defect 122577 */
		return(DLC_OK); 
	}
}

lbuf = mpx->readlist;

/**************************************************************************/
/*                                                                        */
/*     Copyin the extension because the last field, dlh_len, needs to be  */
/* consulted to determine if the data link header is to be returned to the*/
/* user.  The dlh_len set to non-zero returns the header with the buffer  */
/* with dlc_len holding the length of this header.                        */
/*      tbuf holds the extension passed up with the buffer.               */
/*                                                                        */
/**************************************************************************/

rc = copyin(ext, &buf, sizeof(struct dlc_io_ext));

if (rc != DLC_OK)
{
	simple_unlock(&mpx->lock);  /* defect 122577 */
	return(rc);
}

tbuf = (struct dlc_io_ext *)mtod(lbuf,caddr_t);

/* defect 152850 */
/**************************************************************************/
/* The length of the buffer is compared with that of the user's space     */
/* and the OFLO bit is set accordingly.  If overflow occurs, the data     */
/* which fits in the user's buffer is transferred and the remaining data  */
/* is discarded.  Data to be transferred includes the DLC Header if       */
/* requested via a non-zero dlh_len on input.  Otherwise, only the data   */
/* is returned.                                                           */
/*                                                                        */
/* "temp" holds the user's buffer length or the data length, which ever   */
/* is smaller.                                                            */
/*                                                                        */
/* "tbuf->dlh_len" holds the length of the DLC header or zero if none is  */
/* requested.                                                             */
/*                                                                        */
/* "savelen" holds the readlist length of the DLC header in case of       */
/*  system errors that fail the read.                                     */
/*                                                                        */
/* The extension output is copied after the check for uio_resid because   */
/* the flags are modified when overflow occurs.                           */
/**************************************************************************/

if (buf.dlh_len == EMPTY)
{
	savelen = tbuf->dlh_len;
	tbuf->dlh_len = EMPTY;
}

if ((lbuf->m_next->m_len + tbuf->dlh_len) > uiop->uio_resid)
{
	tbuf->flags |= DLC_OFLO;
	temp = uiop->uio_resid;
}
else temp = lbuf->m_next->m_len + tbuf->dlh_len;

rc = copyout(tbuf, ext, sizeof(struct dlc_io_ext));

if (rc != DLC_OK)
{
	/* restore the readlist dlc_io_ext */
	tbuf->flags &= ~DLC_OFLO;
	tbuf->dlh_len = savelen;

	simple_unlock(&mpx->lock);
	return(rc);
}

rc = uiomove(mtod(lbuf->m_next,caddr_t)-tbuf->dlh_len, temp, 
             UIO_READ,uiop);

if (rc != DLC_OK)
{
	/* restore the readlist dlc_io_ext */
	tbuf->flags &= ~DLC_OFLO;
	tbuf->dlh_len = savelen;

	simple_unlock(&mpx->lock);
	return(rc);
}
/* end defect 152850 */

mpx->readlist = (struct mbuf *)mpx->readlist->m_nextpkt;
m_freem(lbuf);

simple_unlock(&mpx->lock);   /* defect 122577 */

return(DLC_OK);
}
