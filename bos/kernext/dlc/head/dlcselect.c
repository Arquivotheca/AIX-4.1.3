static char sccsid[] = "@(#)70	1.3  src/bos/kernext/dlc/head/dlcselect.c, sysxdlcg, bos411, 9428A410j 2/9/94 14:21:53";
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: dlcselect
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
 * NAME: dlcselect
 *                                                                    
 * FUNCTION: Dlcselect returns a flag with the status of the data and exception
 *  lists. If all of the lists queried are empty the system puts the user to 
 *  sleep.
 *                                                                    
 * EXECUTION ENVIRONMENT: Dlcselect is called by the application user only.    
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
dlcselect(dev,events,revent,mpx)
dev_t   dev;
ushort events;
ushort *revent;
struct dlc_chan *mpx;
{
ulong 		rc,mask;
extern ulong 	selwait;


/* defect 122577 */
  simple_lock(&mpx->lock);
/* end defect 122577 */



rc = 0;
*revent = 0;

if (events & DPOLLOUT)
	*revent |= DPOLLOUT;
/****************************************************************************/
/*                                                                          */
/*      If there is something on one of the lists set the return code to    */
/*      1.  Otherwise, a check is made to see if anyone else is doing a     */
/*      READ SELECT on this file descriptor.  If so, the collision flag     */
/*      is set and we return a value of 0.  If no one else was waiting,     */
/*      the process id is saved and we return a value of 0.                 */
/*                                                                          */
/****************************************************************************/

if (events & DPOLLIN)
{
	if (mpx->readlist != (struct mbuf *)EMPTY)
		*revent |= DPOLLIN;
		else
		if (!(events & DPOLLSYNC))
			mpx -> revents |= DPOLLIN;
}


/****************************************************************************/
/*                                                                          */
/*      If the channel indicates an EXCEPTION, set the return code to 1.    */
/*      Otherwise, check if anyone else is doing an EXCEPTION SELECT on     */
/*      this file descriptor.  If so, the collision flag is set and we      */
/*      return a value of 0.  If no one else was waiting, the process id    */
/*      is saved and we return a value of 0.                                */
/*                                                                          */
/****************************************************************************/

if (events & DPOLLPRI)
{
	if (mpx -> exceptlist != (struct mbuf *)EMPTY)
		*revent |= DPOLLPRI;  
		else
		if (!(events & DPOLLSYNC))
			mpx -> revents |= DPOLLPRI;
}


/* defect 122577 */
  simple_unlock(&mpx->lock);
/* end defect 122577 */



return(DLC_OK);
}
