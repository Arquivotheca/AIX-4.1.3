static char sccsid[] = "@(#)04	1.2  src/bos/kernext/c327/c327cutnpn.c, sysxc327, bos411, 9430C411a 7/27/94 09:31:11";
/*
 * COMPONENT_NAME: (SYSXC327) c327 cut mode non-pinned functions
 *
 * FUNCTIONS: c327cutread(), c327cutwrite()
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

/****************************************************************************/
/* cut (em78) mode device driver subroutines called from c327dd             */
/****************************************************************************/
#   include <sys/types.h>
#   include <sys/devinfo.h>
#   include <sys/errno.h>
#   include <sys/intr.h>
#   include <sys/param.h>
#   include <sys/sleep.h>
#   include <sys/io3270.h>
#   include "c327dd.h"

/*
** external variables
*/
#ifdef _POWER_MP
#include <sys/lock_def.h> 
extern Simple_lock c327_intr_lock;
#endif

extern int *cut_read_asleep;

/*PAGE*/
/*
 * NAME: c327cutread()
 *
 * FUNCTION: read
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

int c327cutread(DDS_DATA *dds_ptr, struct uio * uiop)
{

   int rc, saved_intr_level, read_length;

   C327TRACE2 ("CRED", dds_ptr);

   rc = 0;

   if ( WRK.CUT_reset_command )        /* if reset occurred, return error   */
   {
      WRK.CUT_reset_command = FALSE;
      return(EL3RST);
   }

   if ( !WRK.CUT_read_without_wait )   /* do we wait for bufr or cursor chg */
   {
      DISABLE_INTERRUPTS(saved_intr_level);
      if ( (!WRK.CUT_buffer_changed) && (!WRK.CUT_cursor_changed) )
      {
          WRK.CUT_read_sleeping = TRUE;  /* mark waiting                    */
          C327UNPTRACE2 ("CRSL", dds_ptr);

#ifdef _POWER_MP
          if (e_sleep_thread(&WRK.CUT_read_sleep_event,
             &c327_intr_lock, LOCK_HANDLER | INTERRUPTIBLE)
             == THREAD_INTERRUPTED)
#else
          if (e_sleep(&WRK.CUT_read_sleep_event,
             EVENT_SIGRET) == EVENT_SIG)
#endif
          {
             WRK.CUT_read_sleeping = FALSE;
             C327UNPTRACE2 ("CRIN", dds_ptr);
             RESTORE_INTERRUPTS(saved_intr_level);
             return(EINTR);
          }
          WRK.CUT_read_sleeping = FALSE;
          if ( WRK.CUT_had_interrupt == FALSE )
          {
             C327UNPTRACE2 ("CRNO", dds_ptr);
             RESTORE_INTERRUPTS(saved_intr_level);
             return(ENOCONNECT);
          }
          C327UNPTRACE2 ("CRWK", dds_ptr);
       }
       RESTORE_INTERRUPTS(saved_intr_level);
    }

    WRK.CUT_cursor_changed = FALSE;    /* on cursor chg or vis/sound update */

    if ( (WRK.CUT_buffer_changed) || (WRK.CUT_read_without_wait) )
    {
       WRK.CUT_buffer_changed = FALSE;                   /*  read the TCA */

       if ( (uiop->uio_resid + WRK.seek_displacement) > HDW.bus_mem_size)
          return(EFAULT);                              /* request too large */

       read_length = uiop->uio_resid;

       rc = gets_busmem (dds_ptr, (int)WRK.seek_displacement, uiop); 

       if (rc == 0)
          WRK.seek_displacement = WRK.seek_displacement + read_length;
    }
    return(rc);
}

/*PAGE*/
/*
 * NAME: c327cutwrite()
 *
 * FUNCTION:
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * (NOTES:)
 *
 *
 * (RECOVERY OPERATION:)
 *
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS:
 *
 */

int c327cutwrite (DDS_DATA *dds_ptr, struct uio * uiop)
{

   int rc, write_length;

   rc = 0;
   C327TRACE1 ("CWRT");

   if (WRK.CUT_intr_stat & CUT_RESETOP)
   {
       WRK.CUT_intr_stat &= ~CUT_RESETOP ;  /* reset occurred, return error */
       return(EL3RST);
   }

    if ( (uiop->uio_resid + WRK.seek_displacement) > HDW.bus_mem_size)
       return(EFAULT);                                /* request too large */

    write_length = uiop->uio_resid;

    rc = puts_busmem (dds_ptr, WRK.seek_displacement, uiop); 

    if (rc == 0)
       WRK.seek_displacement = WRK.seek_displacement + write_length;

    return(rc);
}
