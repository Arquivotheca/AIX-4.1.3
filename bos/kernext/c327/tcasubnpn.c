static char sccsid[] = "@(#)09	1.4  src/bos/kernext/c327/tcasubnpn.c, sysxc327, bos411, 9430C411a 7/27/94 09:35:07";
/*
 * COMPONENT_NAME: (SYSXC327) c327 tca non-pinned utility subroutines
 *
 * FUNCTIONS: bufferAlloc(), bufferFree(), mdepClosePort(), mdepHaltLA(), 
 *     mdepOpenPort(), mdepStartLA(), waitForLinkState()
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
#include <sys/signal.h>
#include <sys/sleep.h>
#include <sys/errno.h>
#include <sys/io3270.h>
#include "c327dd.h"
#include "dftnsDcl.h"
#include "dftnsSubr.h"
#include "tcadefs.h"
#include "tcadecls.h"
#include "tcaexterns.h"

#ifdef _POWER_MP
#include <sys/lock_def.h> 
extern Simple_lock c327_intr_lock;
#endif

/*PAGE*/
/*******************************************************************
**
** Function Name:       mdepHaltLA
**
** Description: Calls DFT 386 device handler subroutine "dftnsHalt()"
**              to halt a link address.
**
**              Based on the return code, mdepHaltLA() may go to sleep
**              until the acknowledging interrupt occurs - or until the user
**              kills the process; or the return code may indicate no need
**              to wait for interrupt; or it may indicate an error.
**
** Inputs:      laP             pointer to a link address structure
**              laNum           link address number
**              dev             minor device number
**
** Outputs:     none
**
** Externals    mlnk_ptrs[]
** Referenced   mtca_storer
**
** Externals    mlnk_ptrs[]
**
*******************************************************************/
void mdepHaltLA (int laNum, linkAddr *laP, int dev)
{
   int     returnCode;

   returnCode = dftnsHalt(laP->la_netID);
   C327TRACE4("Hmd0",dev,laNum,returnCode);

   if(returnCode != RC_OK_NONE_PEND){
      mupdate_link_fields(returnCode, laP, laNum, dev);
      C327UNPTRACE4("Bmd0", dev, laNum, laP);
   }
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       mdepClosePort
**
** Description: Used to terminate access to a specific port.
**
**              Calls DFT 386 device handler subroutine "dftnsClose()"
**              to terminate access to DFT card.
**
**              The user will not be able to break out of the sleep loop.
**              It is assumed, and hoped, that the acknowledging interrupt
**              will always occur.
**
** Inputs:      laNum           link address number
**              dev             minor device number
**
** Outputs:     {MDEP_SUCCESS}  Close of port succeeded
**              {MDEP_FAILURE}  Close of port failed
**
** Externals    mlnk_ptrs[]
** Referenced   mopen_first_time
**              mtca_storer
**              msnoozer
**
** Externals    mlnk_ptrs[]
** Modified     mopen_first_time
**
*******************************************************************/
int mdepClosePort (int laNum, int dev)
{
   int     returnCode;
   int     mdepStatus;
   int     plX;

   mdepStatus = MDEP_SUCCESS;      /* start off assuming all will work */
   returnCode = dftnsClose(tca_data[dev].cd_ddsPtr);
   C327TRACE4("Cmd0", dev, laNum, returnCode);


   if(returnCode != RC_OK_NONE_PEND){
      mdepStatus = MDEP_FAILURE;
      C327TRACE3("Bmd1", dev, laNum);
   }

   DISABLE_INTERRUPTS(plX);           /* disable interrupts */
   setCardDown(dev);               /* show card is down */

   /* clear the open first time flag so that on the next
   ** first open to the driver an attach and open will occur */

   tca_data[dev].open_first_time = 0;

   RESTORE_INTERRUPTS(plX);       /* restore interrupts to prev. level */
   C327TRACE3("Cmd1",mdepStatus,returnCode);

   return(mdepStatus);
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       mdepStartLA()
**
** Description: Used to establish a session with the DFT card.
**
**              Calls DFT 386 device handler subroutine "dftnsStart()"
**              to start a link address.
**
**              Based on the return code, mdepStartLA() may go to sleep
**              until the acknowledging interrupt occurs - or until the user
**              kills the process; or the return code may indicate no need
**              to wait for interrupt; or it may indicate an error.
**
** Inputs:      laP             pointer to the link address structure
**              laNum           link address number
**              module_ID       module ID for mlogerr()
**              dev             minor device number
**              lock_word_p     pointer to lock word, NULL if no lock
**                              is help by calling process.
**
** Output:      {MDEP_FAILURE}  indicates that an error occurred
**              {MDEP_SUCCESS}  start device successful
**
** Externals    link_ptrs[]
**
** Externals    link_ptrs[]
**
********************************************************************/

int mdepStartLA (linkAddr *laP, int laNum, int module_ID,
	int dev,
	void *lock_word_p)
{
   int     returnCode;
   boolean sleepIntrupted;
   int     mdepStatus;
   boolean non_sna;

   setLinkState(laP, LS_START_INPROG);    /* show start is in progress */

   C327TRACE4("Lmd0", dev, laNum, module_ID);

   returnCode = dftnsStart(tca_data[dev].cd_ddsPtr, laP->la_sess_type,
                           laP->la_printerAddr, laP->la_recvDbP, &non_sna);
   /* set the non_sna in laP structure */
   laP->non_sna = non_sna;

   mdepStatus = MDEP_SUCCESS;      /* start off assuming all will work */

   switch(returnCode){
      case 0:         /* wait for interrupt */
         /*
         ** go to sleep until we get an interrupt from the interrupt
         ** hndlr indicating that the link address has been started
         */
#ifdef _POWER_MP
        if (lock_word_p == (void *)NULL) {
           C327TRACE3("mdsn",dev, &tca_data[dev].sleep_open_link_address);
           sleepIntrupted = (e_sleep_thread(
		&tca_data[dev].sleep_open_link_address, (void *)NULL,
		INTERRUPTIBLE) != THREAD_AWAKENED);
	} else {
           C327TRACE3("mdsl",dev, &tca_data[dev].sleep_open_link_address);
           sleepIntrupted = (e_sleep_thread(
		&tca_data[dev].sleep_open_link_address,
		lock_word_p, LOCK_WRITE|INTERRUPTIBLE) != THREAD_AWAKENED);
        }
#else
        sleepIntrupted = (e_sleep(
		(void *)&tca_data[dev].sleep_open_link_address, EVENT_SIGRET)
		!= EVENT_SUCC);
#endif
        if (sleepIntrupted)
        {
            C327TRACE5("Bmd3",dev,laNum,sleepIntrupted,
                       (int)&tca_data[dev].open_link_address);
            mdepStatus = MDEP_FAILURE;
         }
         break;
      case 256:       /* don't wait for interrupt */
         break;
      default:
         /*
         ** send message to error logger indicating start link
         ** address failed
         */
         mlogerr(ERRID_C327_START, dev, returnCode, laNum);
         laP->rc = EIO;
         mdepStatus = MDEP_FAILURE;
         C327TRACE5("Bmd4",dev,laNum,returnCode,laP->rc);
         break;
   }


   return(mdepStatus);
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       mdepOpenPort()
**
** Description: Used to initiate access to port, which starts a communications
**              session with a controller.
**
**              Calls DFT 386 device handler subroutine "dftnsOpen()"
**
** Inputs:      none
**
** Output:      {MDEP_FAILURE}  indicates that an error occurred
**              {MDEP_SUCCESS}  indicates that open open port was successful
**
** Externals    mtca_storer
**              msnoozer
**
********************************************************************/

int mdepOpenPort (int dev)
{
   int     returnCode;
   int     mdepStatus;

   mdepStatus = MDEP_SUCCESS;      /* start off assuming all will work */

   returnCode = dftnsOpen(tca_data[dev].cd_ddsPtr);
   C327TRACE4("Omd0",dev,tca_data[dev].cd_ddsPtr,returnCode);
   /* assume that parm2 is actually the operation results */
   if(returnCode == RC_OK_NONE_PEND){      /* open successfull */
      setCardUp(dev);         /* show card is up */
   } else {                        /* open failed */
      setCardDown(dev);       /* show card is down */
      mdepStatus = MDEP_FAILURE;
      /* call error logger */
      C327TRACE2("Bmd5",dev);
      mlogerr(ERRID_C327_START, dev, returnCode, 0);
   }

   return(mdepStatus);
}
/*PAGE*/
/*
********************************************************************
** bufferAlloc:
**      Memory allocation routine for Data_Buffer's.
**      "dev" is not used for ps/2
********************************************************************
*/
Data_Buffer *bufferAlloc (int dev, Data_Buffer *odbP, struct uio *uiop)
{
   Data_Buffer     *dbP;
   int     tmp_buff_size;
   
   /*
   ** Set tmp_buff_size to use LOBIBP to begin with
   */
   tmp_buff_size =
             tca_data[dev].cd_ddsPtr->dds_dev_section.buffer_size;
   /*
   ** If uiop is not null that means we are allocating for a write
   ** so let's use a little intelligence about how big a buffer
   ** to allocate.
   **
   ** If write is smaller or equal to the LO_BUFSIZE
   ** then make sure LO_BUFSIZE  is not greater than LOBIBP.
   ** If it is then use LOBIBP otherwise use LO_BUFSIZE.
   */
   if ((uiop) && (uiop->uio_resid <= LO_BUFSIZE))
      tmp_buff_size =
               (LO_BUFSIZE > tmp_buff_size) ? tmp_buff_size : LO_BUFSIZE;
   /*
   ** If we don't have a previous buffer allocate one
   ** (i.e. read, autoack, and first write allocs)
   */
   if (!odbP) {
      dbP = (Data_Buffer *)
             xmalloc((uint)(tmp_buff_size + sizeof(struct dbhead)),
                     (uint)3, pinned_heap);
   } 
   /*
   ** If we do have a previous buffer only allocate a new one
   ** if the size of the old one is wrong for this write.
   */
   else if (odbP->dbhead.buff_size != tmp_buff_size) {
           bufferFree(odbP);
           odbP = NULL;
           dbP = (Data_Buffer *)
                  xmalloc((uint)(tmp_buff_size + sizeof(struct dbhead)),
                          (uint)3, pinned_heap);
        }
        else
           dbP = odbP;

    bzero((void *)&dbP->buf_start, (uint)tmp_buff_size);

    dbP->dbhead.buff_size = tmp_buff_size;
    dbP->dbhead.buf_ovrflw = FALSE;
    dbP->dbhead.data_len = 0;
    dbP->dbhead.data_offset = 0;

    C327TRACE4("Amd0", dev, dbP->dbhead.buff_size, dbP);

    return(dbP);
}
/*PAGE*/
/*
********************************************************************
** bufferFree:
**      Memory free routine for Data_Buffer's.
********************************************************************
*/
void bufferFree (Data_Buffer *dbP)
{

   C327TRACE2("Umd0",dbP);
   xmfree((void *)dbP, pinned_heap);
}
/*PAGE*/
/*
 * NAME: waitForLinkState()
 *
 * FUNCTION: Wait for link state to change to "newState",
 *           if not already there.
 *
 * EXECUTION ENVIRONMENT:
 *
 *   This should describe the execution environment for this
 *   procedure. For example, does it execute under a process,
 *   interrupt handler, or both. Can it page fault. How is
 *   it serialized.
 *
 * (NOTES:) More detailed description of the function, down to
 *          what bits / data structures, etc it manipulates.
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *                       software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: Returns new link state if successful, zero (0) if failed.
 */
int waitForLinkState (linkAddr *laP, state newState)
{
   int   plX;

   C327TRACE4("Wit1",laP,newState,laP->link_state);
   DISABLE_INTERRUPTS(plX);

   while((laP->link_state & newState) == 0)
   {
      C327UNPTRACE4("Wit2",laP,newState,laP->link_state);
#ifdef _POWER_MP
      if (e_sleep_thread(&laP->sleep_link_state, &c327_intr_lock,
         LOCK_HANDLER | INTERRUPTIBLE) == THREAD_INTERRUPTED)
#else
      if(e_sleep((void *)&laP->sleep_link_state,
                  EVENT_SIGRET) == EVENT_SIG)
#endif
      {
         laP->rc = EINTR;
         C327UNPTRACE5("Bit1",laP,laP->link_state,newState,
                       EINTR);
         break;
      }
   }


   RESTORE_INTERRUPTS(plX);
   C327TRACE4("Wit0",laP,newState,laP->link_state);

   return(laP->rc == EINTR ? 0 : laP->link_state);
}
