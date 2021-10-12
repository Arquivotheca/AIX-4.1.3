static char sccsid[] = "@(#)64	1.7  src/bos/kernext/dlc/head/dlcioctl.c, sysxdlcg, bos411, 9428A410j 2/9/94 14:19:17";
/*
 * COMPONENT_NAME: (sysxdlcg) DLC
 *
 * FUNCTIONS: dlcioctl
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
 * NAME: dlcioctl
 *                                                                    
 * FUNCTION: Dlcioctl allows the user to send a variety of commands.  The 
 *  IOCINFO and GET_EXCEPTION commands are handled in dlcioctl and any others
 *  are passed down to the protocol specific code.  IOCINFO passes back a 
 *  character describing device.  GET_EXCEPTION returns the exception buffer
 *  placed on a list headed by the channel id in dlcinte.  The buffer is then
 *  freed from the list. 
 *                                                                    
 * EXECUTION ENVIRONMENT: Dlcioctl is called by the user.
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
dlcioctl(dev, op, arg, flag, mpx)
int dev,op,flag;
caddr_t arg;
struct dlc_chan *mpx;
{
struct devinfo devinfo;
int     rc = DLC_OK;
int     mask;
struct mbuf      *buf;
struct mbuf      *ebuf;

switch(op)
{


/**************************************************************************/
/*                                                                        */
/*    DLC_GET_EXCEP: return the next exception buffer on the list or an   */
/* an error if the list is empty.  After copying the buffer into user     */
/* space, free the buffer and adjust the list accordingly.                */
/*                                                                        */
/**************************************************************************/

case DLC_GET_EXCEP :

/* defect 122577 */
  simple_lock(&mpx->lock);
/* end defect 122577 */


	if (mpx->exceptlist == (struct mbuf *)DLC_NULL)
	{ 

/* defect 122577 */
  simple_unlock(&mpx->lock);
/* end defect 122577 */


			return(EAGAIN); 
	}

	ebuf = mpx->exceptlist;

	if (copyout(ebuf->m_dat, arg, sizeof(struct dlc_getx_arg)) == 
               DLC_ERR)
	{

/* defect 122577 */
  simple_unlock(&mpx->lock);
/* end defect 122577 */

		
		return(EIO);
	}

	mpx->exceptlist = (struct mbuf *)mpx->exceptlist->m_nextpkt;


/* defect 122577 */
  simple_unlock(&mpx->lock);
/* end defect 122577 */


	m_freem(ebuf);
	break;

/**************************************************************************/
/*                                                                        */
/*      default:  Protocol Specific ioctl, pass thru.                     */
/*                                                                        */
/**************************************************************************/

default:
	rc=pr_ioctl(dev, op, arg, flag, mpx);
	return(rc);
} /* end switch(op) */
return(DLC_OK);

}
