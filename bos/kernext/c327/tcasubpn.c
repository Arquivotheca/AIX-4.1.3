static char sccsid[] = "@(#)10	1.4.1.2  src/bos/kernext/c327/tcasubpn.c, sysxc327, bos411, 9430C411a 7/27/94 09:35:20";
/*
 * COMPONENT_NAME: (SYSXC327) c327 tca pinned utility subroutines
 *
 * FUNCTIONS: bufferEmpty(), checkLinkState(), mdepSendStatus(), 
 *     mdepWriteBuffer(), mlogerr(), setLinkState()
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
#include <sys/intr.h>
#include <sys/malloc.h>
#include <sys/sleep.h>
#include <sys/errno.h>
#include <sys/io3270.h>
#include <string.h>
#include "c327dd.h"
#include "dftnsDcl.h"
#include "dftnsSubr.h"
#include "tcadefs.h"
#include "tcadecls.h"
#include "tcaexterns.h"
/*PAGE*/
/*******************************************************************
**
** Function Name:       mdepSendStatus
**
** Description: Used to send status to remote system.
**
**              Calls DFT 386 device handler subroutine "dftnsSend()"
**              to send status
**
**              Based on the return code, mdepSendStatus() may go to sleep
**              until the acknowledging interrupt occurs - or until the user
**              kills the process; or the return code may indicate no need
**              to wait for interrupt; or it may indicate an error.
**
** Inputs:      dev             minor device number
**              laNum           link address number
**              status          status to be sent to remote system
**
** Outputs:     {MDEP_SUCCESS}  Send status succeeded
**              {MDEP_FAILURE}  Send status failed
**
** Externals    tca_data[].mlnk_ptrs[]
** Referenced
**
*******************************************************************/
int mdepSendStatus (int dev, int laNum, int status)
{
   register linkAddr       *laP;
   int     returnCode;
   int     mdepStatus;

   mdepStatus = MDEP_SUCCESS;      /* start off assuming all will work */

   laP = tca_data[dev].mlnk_ptrs[laNum];
   aixSendStat(laP);  /* Set ls_state before sending status to Control Unit.*/ 

   returnCode = dftnsSend(laP->la_netID, status);
   C327TRACE4("Smd1",dev,laNum,returnCode);

   if (returnCode != 256) {
         mdepStatus = MDEP_FAILURE;                       
   }

   C327TRACE3("Smd2",laP->rc,mdepStatus);

   return(mdepStatus);
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       mdepWriteBuffer()
**
** Description: Writes out a buffer to DFT.
**
**              Calls DFT 386 device handler subroutine "dftnsWrite()"
**              to write data buffer
**
**      Common: Note that since this routine may be called from interrupt
**              level, it does not goto sleep(), rather it just returns the
**              return code from the write call
**
** Inputs:      dev             minor device number
**              laNum           link address number
**              dbP             pointer to a Data_Buffer
**
** Output:      int             return code from the subroutine
**
** Externals    mlnk_ptrs
** Referenced
**
** Externals
** Modified
**
********************************************************************/
int mdepWriteBuffer (int dev, int laNum, Data_Buffer *dbP)
{
   register linkAddr       *laP;
   int     returnCode;

   laP = tca_data[dev].mlnk_ptrs[laNum]; 

   /*
   ** non-zero in third parameter requests BSC transparency mode
   ** on a remote line from the api or the file transfer appls
   */
   returnCode = dftnsWrite(laP->la_netID, dbP, laP->BSC_mode );
   C327TRACE4("Wmd0",dbP,laP->BSC_mode,returnCode);

   return(returnCode);
}
/*PAGE*/
/*
********************************************************************
** bufferEmpty:
**      Returns 1 (true) if data buffer is empty or completly read 
**              0 otherwise
********************************************************************
*/
int bufferEmpty(Data_Buffer *dbP)
{
   return(dbP->dbhead.data_offset >= dbP->dbhead.data_len);
}
/*PAGE*/
/*******************************************************************
**
**      Function Name:  mlogerr
**
**      Description:    log an error to the error logger
**
**      Inputs:         err_id  type of error ( see errids.h )
**                      dev     minor dev_no
**                      results return_code of opResults of error
**                      address laNum causing error
**
**      Outputs:        void
**
**      Externals       none
**      Referenced
**
**      Externals       none
**      Modified
**
*******************************************************************/

void mlogerr (int err_id, int dev, int results, int address)
{
   errmsg   msglog;
   DDS_DATA *dds_ptr;

   C327TRACE5("LclS",err_id, dev, results, address);

   msglog.err.error_id = err_id;

   dds_ptr = (DDS_DATA *)tca_data[dev].cd_ddsPtr;
   strncpy(msglog.err.resource_name, (const unsigned char*)HDR.dev_name,
           (size_t)sizeof(msglog.err.resource_name));

   msglog.results = results;
   msglog.address = address;

   errsave(&msglog, (uint)sizeof(errmsg));
}
/*PAGE*/
/*
 * NAME: setLinkState()
 *                                                                    
 * FUNCTION: High level description of what the procedure does
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *     This should describe the execution environment for this
 *     procedure. For example, does it execute under a process,
 *     interrupt handler, or both. Can it page fault. How is
 *     it serialized.
 *                                                                   
 * (NOTES:) More detailed description of the function, down to
 *          what bits / data structures, etc it manipulates.
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *                       software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */  
void setLinkState (linkAddr *laP, state newState)
{
   int   plX;

   laP->link_state = newState;

   C327TRACE3("Sit0",laP,laP->link_state);
   /*
   ** let everyone examine new state
   */
   e_wakeup((void *)&laP->sleep_link_state);
}
