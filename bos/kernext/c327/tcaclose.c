static char sccsid[] = "@(#)33	1.21  src/bos/kernext/c327/tcaclose.c, sysxc327, bos411, 9430C411a 7/27/94 09:33:18";

/*
 * COMPONENT_NAME: (SYSXC327) c327 tca device driver entry points
 *
 * FUNCTIONS:    mfree_la_struct(), tcaclose(), mscan_link_ptrs()
 *     
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

/*
** INCLUDE FILES
*/
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/errno.h>
#include <sys/io3270.h>
#include <sys/intr.h>
#include <sys/malloc.h>
#include <sys/trchkid.h>
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
**      Function Name:  tcaclose
**
**      Description:    issue a halt device, flush the device ring
**                      queue, and free the link address
**                      strucuture if this is the last
**                      close of a link address.  Issue a
**                      close to offlevel if this is the last close
**                      of the device.
**
**      Inputs:         dev             device minor number
**                      laNum           the link address number
**
**      Outputs:        void
**
**      Externals       mlnk_ptrs[]
**      Referenced
**
**      Externals       mlnk_ptrs[]
**      Modified
**
*******************************************************************/
int tcaclose (dev_t devt, int laNum)
{
       linkAddr *laP;
       int saved_intr_level, dev;

       C327TRACE3("CloS",devt, laNum);

       dev = minor(devt);

       laP = tca_data[dev].mlnk_ptrs[laNum];
       
       if (laP == NULL)
          return(0);
       
       laP->rc = 0;       /* initialize errno area to zero */

       /* --------------------------------------------------------- */
       /* check and see if this is the last close of a link address */
       /* clean up the link address structure */
       /* flush link address write buffer que and free all buffers */
       /* --------------------------------------------------------- */
       if (!--laP->num_processes){
          C327TRACE2("Ccl0",getLinkState(laP));
          /*
          ** make sure that the link address has been successfully
          ** started before issuing the halt.
          ** flush device ring que and free all buffers
          ** halt the link address
          */

	/* XXX - this disable is really stupid - see how ls can get changed */
          DISABLE_INTERRUPTS(saved_intr_level);
          if (getLinkState(laP) != LS_LINKDOWN) {
             RESTORE_INTERRUPTS(saved_intr_level);
             setLinkState(laP, LS_CLOSING);
             mdepHaltLA(laNum, laP, dev);
             setLinkState(laP, LS_LINKDOWN);
          }
          else
              RESTORE_INTERRUPTS(saved_intr_level);

          /*
          ** close if this is the last close of the device
          */

	/* XXX - this disable is even dumber - I can see why this would ever
	 * do anything but slow down the machine
         */
          DISABLE_INTERRUPTS(saved_intr_level);
          tca_data[dev].mlnk_ptrs[laNum] = (linkAddr *)NULL;
          RESTORE_INTERRUPTS(saved_intr_level);

          if (mscan_link_ptrs(dev))
             mdepClosePort(laNum,dev);

          /*
          ** free the link address structure
          */
          mfree_la_struct(laNum, laP, dev);

       }
       C327TRACE2("CloE",getLinkState(laP));

       return( laP->rc );
}
/*PAGE*/
/*******************************************************************
**
**      Function Name:  mscan_link_ptrs
**
**      Description:    scans through the mlnk_ptrs to see if
**                      there are any more link addresses started
**
**      Inputs:         device minor number
**
**      Outputs:        1       indicating that there are still link
**                              addresses started.
**                      0       indicating that there isn't a link
**                              address started.
**
**      Externals       mlnk_ptrs[]
**      Referenced      mlower_link_address
**                      mupper_link_address
**
**      Externals       mlnk_ptrs[]
**      Modified
**
*******************************************************************/

short mscan_link_ptrs(int dev)
{
       register short session, rc = 1;

       /* look for started link addresses */
       for(session = tca_data[dev].lower_link_address;
             session <= tca_data[dev].upper_link_address; session++){
          if(tca_data[dev].mlnk_ptrs[session]){
             rc = 0;
             break;
          }
       }
       C327TRACE5("Mcl0",dev,tca_data[dev].mlnk_ptrs[session],session,rc);
       return(rc);
}
/*PAGE*/
/*******************************************************************
**
**      Function Name:  mfree_la_struct
**      Description:    free the link address stucture and zero out
**                      the entry in mlnk_ptrs
**
**      Inputs:         laNum   link address number
**                      laP     pointer to a link address structure
**
**
**      Outputs:        void
**
**      Externals       mlnk_ptrs[]
**      Referenced
**
**      Externals       mlnk_ptrs[]
**      Modified
**
*******************************************************************/

void mfree_la_struct (int laNum, linkAddr *laP, int dev)
{
       int saved_intr_level;

       if (laP->writeBuffer)
           bufferFree(laP->writeBuffer);

       if (laP->la_recvDbP)
           bufferFree(laP->la_recvDbP);

       if (laP->la_WSFdbP)       /* free auto ack buffer if it exists */
           bufferFree(laP->la_WSFdbP);

       xmfree((void *)laP, pinned_heap);

	/* XXX - this disable is even dumber - I can see why this would ever
	 * do anything but slow down the machine
         */
       DISABLE_INTERRUPTS(saved_intr_level);
       tca_data[dev].mlnk_ptrs[laNum] = (linkAddr *)NULL;
       RESTORE_INTERRUPTS(saved_intr_level);
}
