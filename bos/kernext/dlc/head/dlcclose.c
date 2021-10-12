static char sccsid[] = "@(#)60  1.5  src/bos/kernext/dlc/head/dlcclose.c, sysxdlcg, bos412, 9446B 11/15/94 16:49:56";
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: dlcclose
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
 * NAME: dlclose
 *                                                                    
 * FUNCTION: Dlcclose calls the protocol specific code to close the device
 *  handler if this is the last channel and to clean up any control blocks
 *  associated with the channel.  After this it will clean up the channel
 *  control block itself and return.
 *                                                                    
 * EXECUTION ENVIRONMENT: Dlcclose is called by the user to terminate an opened
 *  protocol or by the system when the opening process is terminated.  The 
 *  close should work at any state of a connection.  It serializes with opens
 *  by locking the channel id list lock.
 *                                                                     
 * RETURNS: DLC_OK
 */  

#include        "dlcadd.h"
/*ARGSUSED*/
dlcclose(dev,mpx)
dev_t   dev;
struct dlc_chan *mpx;
{
static_trace (mpx->cb,"Clos",DLC_OK);   /* defect 167068 */
return(DLC_OK);
}
