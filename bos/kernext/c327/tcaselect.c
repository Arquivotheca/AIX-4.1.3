static char sccsid[] = "@(#)42	1.13  src/bos/kernext/c327/tcaselect.c, sysxc327, bos411, 9430C411a 7/27/94 09:34:45";
/*
 * COMPONENT_NAME: (SYSXC327) c327 tca device driver entry points
 *
 * FUNCTIONS:    tcaselect()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/types.h>
#include <sys/poll.h>
#include <sys/sysmacros.h>
#include <sys/errno.h>
#include <sys/intr.h>
#include <sys/io3270.h>
#include "c327dd.h"
#include "dftnsDcl.h"
#include "tcadefs.h"
#include "tcadecls.h"
#include "tcaexterns.h"

#ifdef _POWER_MP
#include <sys/lock_def.h> 
extern Simple_lock c327_intr_lock;
#endif

/*******************************************************************
**
** Function Name:       tcaselect()
**
** Description:         perform a select on a read or exception to
**                      the device driver.
**
** Inputs:      dev     minor device number
**              events  events that are to be checked, READ, WRITE
**                      or EXCEPTION. (Synchronous request)
**              revents returned event ptr, set by this select call
**                      will set to 0 if no events are satisfied.
**              chan    a uniqe code identifying the link address
**                      number and the process using it.
**
** Output:      
**
** Externals  
** Referenced
**
** Externals    mlnk_ptrs[]
** Modified
** Note: at this time i have kept the switch structure of the
** old tca code. In the future, it could be possible to
** handle multiple select flags at once from application.
********************************************************************/
int tcaselect (dev_t devt, ushort events, ushort *return_events, int chan)
{
   register int laNum;
   register linkAddr *laP;
   int      plX, dev;

   dev = minor(devt);

   laNum = chan & 0x00ff;

   laP = tca_data[dev].mlnk_ptrs[laNum];

   *return_events = 0;/* Nothing selected yet */

   DISABLE_INTERRUPTS(plX);

   /* ---------------------------------------------------------- */
   /* If a read event is true, update returned event parameters */
   /* ---------------------------------------------------------- */
   if(events & POLLIN){
      if((laP->io_flags & WDI_DAVAIL))  /* if data available */
         *return_events |= POLLIN;    /* set event to READ */
      else{
         if(!(events & POLLSYNC))/* if not synchronous */
            laP->dev_selr = chan;/* wait to be notified */
      }
   }

   /* ---------------------------------------------------------- */
   /* If a write event is true, update returned event parameters */
   /* ---------------------------------------------------------- */
   if(events & POLLOUT)
      *return_events |= POLLOUT;/* set for WRITE */

   /* ---------------------------------------------------------- */
   /* If an exception event is true, update returned event       */
   /* ---------------------------------------------------------- */
   if(events & POLLPRI) {
      if((laP->io_flags & WDI_ALL_CHECK))  /* no exceptions pending */
         *return_events |= POLLPRI;     /* set event to EXCEPTION */
      else{
         if(!(events & POLLSYNC))/* if not synchronous */
         laP->dev_sele = chan;/* wait to be notified */
      }
   }

   RESTORE_INTERRUPTS(plX);

   /* ---------------------------------------------------------- */
   /* no event was provided, */
   /* ---------------------------------------------------------- */
   C327TRACE2("SelE",*return_events);
   return( SUCCESS );
}
