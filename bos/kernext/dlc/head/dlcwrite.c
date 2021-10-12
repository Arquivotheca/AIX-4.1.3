static char sccsid[] = "@(#)21	1.22  src/bos/kernext/dlc/head/dlcwrite.c, sysxdlcg, bos411, 9440A411c 10/4/94 13:42:02";
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: dlcwrite
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
 * NAME: dlcwrite
 *                                                                    
 * FUNCTION: Dlcwrite puts an application users data into an mbuf and passes it
 *  down to the protocol speceific code.  For kernel users the data should 
 *  in a mbuf pointed to by uio->uio_iov->iov_base and the routine passes it on 
 *  to the protocol specific code.
 *                                                                    
 * EXECUTION ENVIRONMENT: Dlcwrite is called by both application and kernel 
 *  users.
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
dlcwrite(dev, uiop, mpx, ext)
ulong          dev;
struct uio     *uiop;
struct dlc_chan *mpx;
struct dlc_io_ext *ext;

{
struct mbuf *buf;
/* <<< feature CDLI >>> */
struct mbuf *tempbuf;
/* <<< end feature CDLI >>> */
struct dlc_io_ext s_ext;
ulong rc;
ulong i;

/**************************************************************************/
/*                                                                        */
/*      If the call came from user space (uio_segflg set to              */
/* UIO_USERSPACE), then read it into an mbuf, and pass the buffer through */
/* to the protocol specific code.                                         */
/*                                                                        */
/**************************************************************************/

if (uiop->uio_segflg == UIO_USERSPACE)
{
	rc = copyin(ext, &s_ext, sizeof(struct dlc_io_ext));

	if (rc != 0)
	{
		return(rc);
	}

/* <<< feature CDLI >>> */
	/******************************************************************/
	/* Issue m_get of type=data from the kernel heap.                 */
	/* Note: all CDLI writes must have an mbuf of type=header as the  */
	/*       first buffer in the chain.  However, LAN protocol code   */
	/*       always prepends a data link header mbuf except in the    */
	/*       case of network data.  It is up to the LAN protocol      */
	/*       code to insure that the first mbuf is always set to type */
	/*       header.  COMIO will accept an mbuf of type=data, and all */
	/*       CDLI chained mbufs (other than the first) should be of   */
	/*       type data.                                               */
	/******************************************************************/

	if((buf = m_get(M_WAIT,MT_DATA))==(struct mbuf *)DLC_NULL)
/* <<< end feature CDLI >>> */
	{
		return(ENOMEM);
	}

/**************************************************************************/
/*                                                                        */
/*     Check to see if the buffer will fit into an mbuf or if it needs a  */
/* cluster.  If it needs a cluster, assume mclget will adjust m_data to   */
/* point to the top of the 4k block.                                      */
/*                                                                        */
/**************************************************************************/

	if (uiop->uio_resid > MHLEN)         /* defect 164559 */
	{
		/* LEHb defect 38748 */
		if (m_clgetm(buf, M_WAIT, CLBYTES) == 0)
		/* LEHe */
		{
			m_freem(buf);
			return(ENOSPC);
		}
	}
	else /* defect 164559, the small mbuf is large enough, including
                some pad for pkthdr data if it gets converted to MT_HEADER  */
	{
		/* so adjust the data pointer to allow struct pkthdr */
		buf->m_data += (MLEN - MHLEN);
	}
		
/****************************************************************************/
/*                                                                          */
/*      We need to check that the information to be sent will fit into      */
/*      the mbuf.                                                           */
/*                                                                          */
/****************************************************************************/

	if (MAXDATASIZE < uiop->uio_resid )
	{
		m_freem(buf);
		return(EINVAL);
	}

	buf->m_len = uiop->uio_resid;

/* <<< feature CDLI >>> */
/* <<< removed s_ext.dlh_len >>> */
/* <<< end feature CDLI >>> */

	if (buf->m_len > 0)
	{
		rc = uiomove(mtod(buf,caddr_t),buf->m_len,UIO_WRITE,uiop);
		
		if (rc != 0)
		{
			m_freem(buf);
			return(rc);
		}
	}

	rc = pr_write(uiop, mpx, buf, &s_ext); /* Defect 115926 */
/* <<< THREADS >>> */
	while ((rc == EAGAIN) && !(uiop->uio_fmode & (FNDELAY|FNONBLOCK))
	     && (thread_self() != mpx->cb->kproc_tid))
/* <<< end THREADS >>> */

	{
/* defect 122577 */
                if (e_sleep_thread(&mpx->writesleep, &mpx->cb->lock,
                     (LOCK_SIMPLE | INTERRUPTIBLE) ) != THREAD_AWAKENED)
                {
                        simple_unlock(&mpx->cb->lock);
                        m_freem (buf);       /* defects 154624 158348 */
                        return(EINTR);
		}
                simple_unlock(&mpx->cb->lock);
/* end defect 122577 */

		rc = pr_write(uiop, mpx, buf, &s_ext); /* Defect 115926 */
	}
	/* the lock is obtained in pr_write code */
        /* We should not release the lock at the */
        /* end of the pr_write since there is a  */
        /* small time gap that before teh control*/
        /* get back to the e_sleep, the user     */
	/* process may get dispatch out. And the */
        /* dlc kernel process may issue the wakup*/
        /* call before the user process get to   */
   	/* sleep				 */
/* <<< THREADS >>> */
	if ( thread_self() != mpx->cb->kproc_tid )
/* <<< end THREADS >>> */


/* defect 122577 */
  simple_unlock(&mpx->cb->lock);
/* end defect 122577 */

 
	if (rc)
	{
		m_freem(buf);
		return(rc);
	}
} 
/* <<< feature CDLI >>> */
else /* the write came from kernel space */
{
/* <<< removed  ext->dlh_len >>> */
/* <<< removed  buf >>> */
/* <<< end feature CDLI >>> */

	rc = pr_write(uiop, mpx, uiop->uio_iov->iov_base, ext);/*Defect 115926*/
/* <<< THREADS >>> */
	while ((rc == EAGAIN) && !(uiop->uio_fmode & (FNDELAY|FNONBLOCK))
		&& (thread_self() != mpx->cb->kproc_tid))
/* <<< end THREADS >>> */

	{
/* defect 122577 */
                if (e_sleep_thread(&mpx->writesleep, &mpx->cb->lock,
                     (LOCK_SIMPLE | INTERRUPTIBLE) ) != THREAD_AWAKENED)

/* end defect 122577 */

		{

/* defect 122577 */
  simple_unlock(&mpx->cb->lock);

                      return(EINTR);
/* end defect 122577 */
                 }

/* defect 122577 */
  simple_unlock(&mpx->cb->lock);
/* end defect 122577 */

			
		rc = pr_write(uiop, mpx, uiop->uio_iov->iov_base, ext); /* Defect 115926 */
	}
/* <<< THREADS >>> */
	if ( thread_self() != mpx->cb->kproc_tid )
/* <<< end THREADS >>> */


/* defect 122577 */
  simple_unlock(&mpx->cb->lock);
/* end defect 122577 */


      	       
	if (rc)
	{
		return(rc);
	}
}
return(DLC_OK);
}

