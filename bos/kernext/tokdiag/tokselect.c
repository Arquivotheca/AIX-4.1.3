static char sccsid[] = "@(#)73	1.11  src/bos/kernext/tokdiag/tokselect.c, diagddtok, bos411, 9428A410j 10/26/93 16:24:38";
/*
 * COMPONENT_NAME: (SYSXTOK) - Token-Ring device handler
 *
 * FUNCTIONS:  tokselect()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include "tok_comio_errids.h"
#include <sys/intr.h>
#include <sys/ioctl.h>
#include <sys/limits.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/pri.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/watchdog.h>
#include <sys/comio.h>  /* this and following files must be in this sequence */
#include <sys/tokuser.h>
#include "tokddi.h"
#include "tokdshi.h"
#include "tokdds.h"
#include "tokdslo.h"
#include "tokproto.h"
#include "tokprim.h"

/*
*  get access to the trace table
*/
extern tracetable_t    tracetable;

/*
*  get access to the component dump table
*/
extern cdt_t   ciocdt;

/*
*  get access to the device driver control block
*/
extern dd_ctrl_t   dd_ctrl;

/*****************************************************************************/
/*
 * NAME:     tokselect
 *
 * FUNCTION: select entry point from kernel (user processes only)
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:  0 or errno
 *           if successful, also returns bits for requested events that
 *           are found to be true
 *
 */
/*****************************************************************************/
int tokselect (
   dev_t   devno,   		/* major and minor number */
   unsigned short  events,  	/* requested events */
   unsigned short *reventp, 	/* address for returning detected events */
   int     chan)    		/* channel number */
{
   register dds_t *p_dds;
   register int adap;
   open_elem_t *open_ptr;

   DEBUGTRACE5 ("SELb", (ulong)devno, (ulong)events, (ulong)*reventp,
      (ulong)chan); /* tokselect begin */

   /* this shouldn't fail if kernel and device driver are working correctly */
   adap=minor(devno);
   if ((chan <= 0)                                 ||
       (chan > MAX_OPENS)                          ||
       ((p_dds = dd_ctrl.p_dds[adap]) == NULL) ||
       (CIO.chan_state[chan-1] != CHAN_OPENED)     ||
       ((open_ptr = CIO.open_ptr[chan-1]) == NULL)    )
   {
      TRACE2 ("SEL1",(ulong)ENXIO); /* tokselect end (bad device or channel) */
      return (ENXIO);
   }

   if (open_ptr->devflag & DKERNEL) /* illegal call from kernel process */
   {
      TRACE2 ("SEL2",(ulong)EACCES); /* tokselect end (call from kernel proc)*/
      return (EACCES);
   }

   *reventp = 0;                   /* initialize return value */
   if ((events & ~POLLSYNC) == 0) /* no events requested */
      return (0);

   if ( (p_dds->wrk.limbo == NO_OP_STATE) &&
	(p_dds->wrk.adap_state == NULL_STATE) )
   {
	/* We have had a hard failure.
	 * set all events that can not be
	 * satisfied to 1.
	 */

         *reventp |= POLLOUT;
         *reventp |= POLLIN;
         *reventp |= POLLPRI;
   } else {
   	/* set return status for all requested events that are true */
   	if (events & POLLOUT)
      		if (!XMITQ_FULL)
       	  		*reventp |= POLLOUT;
   	if (events & POLLIN)
      		if (open_ptr->rec_que.num_elem > 0)
       		  *reventp |= POLLIN;
   	if (events & POLLPRI)
      		if (open_ptr->sta_que.num_elem > 0)
       		  *reventp |= POLLPRI;
   }

   /* if no requested event was found, then see if async notification wanted */
   if (*reventp == 0)
   {
      if (!(events & POLLSYNC)) /* this is asynchronous request */
      {
         /* set flags so later notification with selnotify will be done */
         open_ptr->selectreq |= events;
      }
   }

   DEBUGTRACE2 ("SELe", (ulong)0); /* tokselect end */
   return (0);
} /* end tokselect */
