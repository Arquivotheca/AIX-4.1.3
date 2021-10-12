static char sccsid[] = "@(#)43  1.19.1.4 src/bos/kernext/c327/tcawrite.c, sysxc327, bos41J, 9510A_all 10/25/94 14:58:02";
/*
 * COMPONENT_NAME: (SYSXC327) c327 tca device driver entry points
 *
 * FUNCTIONS:    mwait_for_unmlocked(), mwrite_transfer(), tcawrite(), 
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
#include <sys/signal.h>
#include <sys/sleep.h>
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

/*
** static function prototypes
*/
static void mwrite_transfer (int, linkAddr *, int, struct uio *);
static void mwait_for_unmlocked (linkAddr *, int, int);
static void mwait_for_writebuf (linkAddr *, int, int);

/*PAGE*/
/*******************************************************************
**
** Function Name:       tcawrite()
**
** Description: write data from the RT PC to the host.
**              Wait until there isn't any I/O outstanding.
**              Provided the link status does not indicate
**              an error then send the data.
**              Update the user's ext iotca structure if an error
**              did not occur during the transfer.
**
** Inputs:      dev             device major/minor number
**              uiop            ptr to uio structure, specifying location
**                              and length of caller area
**              laNum           unique session number
**              ext             ptr to a iotca structure or 0
**
** Output:      errno return code, or 0 if successfull
**
** Externals    mlnk_ptrs[]
** Referenced
**
** Externals    mlnk_ptrs[]
** Modified
**
********************************************************************/
int tcawrite(dev_t devt, struct uio *uiop, int laNum , int ext)
{
   linkAddr *laP;
   iotca    *iotca_ptr;
   int      dev;

   dev = minor(devt);

   laP = tca_data[dev].mlnk_ptrs[laNum];
   C327TRACE5("WriS",dev,laNum,laP,uiop);
   iotca_ptr = (iotca *)ext;

   /*
   ** initialize errno to zero
   */
   if(laP) laP->rc = 0;

   /*
   ** set bisync bit on, if fxfer or api write command
   **
   ** set transparency mode only if fxfer or api
   */
   laP->BSC_mode = 0;
   if(iotca_ptr) {
      if (iotca_ptr->io_flags & WDI_TRANSP_MODE)
         laP->BSC_mode |= TRANSP_MODE;
   }

   /*
   ** sleep if link address IO in progress flag is set
   */
   mwait_for_io (laP, laNum, Write, dev);
   
   /*
   ** sleep if link address LOCKED flag is set only if data is not
   ** available
   */
   C327TRACE5("Wwr0",dev,laNum,laP->io_flags,laP->BSC_mode);
   /* don't care about WDI_DAVAIL on SNA write */
   if(!(laP->io_flags & WDI_DAVAIL) || (laP->non_sna == FALSE)){
      mwait_for_unmlocked(laP, laNum, dev);
   }

   /*
   ** If the write buffer is in use,  wait till it's free before continuing.
   */
   if (!laP->writeBuffer_used)
        mwait_for_writebuf(laP, laNum, dev);

   /*
   ** test link address io_flags if link address is operating continue
   ** otherwise set errno and return
   */
   C327TRACE3("Wwr1",laP->io_flags,getLinkState(laP));

   if (!laP->rc) {
      C327TRACE2("Wwr2",laP->writeBuffer_used);
      if ((laP->writeBuffer_used) && (!(laP->io_flags & WDI_DAVAIL) ||
           (laP->non_sna == FALSE))) {
         mwrite_transfer(laNum, laP, dev, uiop);
      }
      else {
         C327TRACE5("Bwr2",dev,laNum,laP->io_flags,laP->writeBuffer);
         laP->write_discarded = TRUE;    /* data collision on next read */
         laP->rc = EIO;
      }
   }

   /*
   ** if an error did not occur, and there is a command
   ** extension then copy the status flags into the ext
   ** if an error occured clear link address io_in_prog flag and
   ** perform wakeup if needed because interrupt handler will not
   ** be entered in this case
   */

   if(iotca_ptr && !laP->rc) {
      mtca_inquire_link(laP, iotca_ptr, laNum, dev);
   }

   if (laP->rc) {
      C327TRACE5("Bwr3",dev,laNum,laP,laP->rc);
      wakeup_mwait_for_io(laP);
   }

   C327TRACE5("WriE",getLinkState(laP),laP->io_flags,
   laP->io_status,laP->rc);

   return(laP->rc);
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       mwrite_transfer()
**
** Description: copy the data out of user memory into allocated buffers
**
** Inputs:      laNum   link address number
**              laP     pointer to the link address structure
**
** Output:      void
**
** Externals    mlnk_ptrs[]
** Referenced   
**              mI_want_my_buffer
**
** Externals    mlnk_ptrs[]
**              mI_want_my_buffer
**
********************************************************************/

void mwrite_transfer(int laNum, linkAddr *laP ,int dev, struct uio *uiop)
{
   Data_Buffer *dbP;
   int cnt;
   int return_code = 0;

   /*
   ** loop until all the data has been copied into
   ** buffers or an error occurs
   */

   C327TRACE3("Twr0",dev,laNum);

   /*
   ** make sure you can get a buffer
   */
   dbP = bufferAlloc(dev, laP->writeBuffer, uiop);
   if(!dbP) {
      laP->rc = EFAULT;
      if(isCardDown(dev)) {    /* test if card is down */
         laP->rc = EFAULT;
         C327TRACE4("Bwr5",dev,laNum,laP->rc);
      }
   }
   else {
      /*
      ** we have a buffer
      **
      ** update the buffer header.
      ** insert the number of bytes in the buffer
      ** residual is automatically set after buffer is moved
      */

      if(uiop->uio_resid > dbP->dbhead.buff_size)
         cnt = dbP->dbhead.buff_size;
      else
         cnt = uiop->uio_resid;

      dbP->dbhead.data_len = cnt;
      dbP->dbhead.data_offset = 0;

      /* copy (cnt bytes from user memory to buffer) */

      laP->rc = uiomove((caddr_t)&dbP->buf_start, cnt, UIO_WRITE, uiop);

      if(laP->rc){
         if (dbP) {
            bufferFree(dbP);
            dbP = laP->writeBuffer = NULL;
            laP->writeBuffer_used = TRUE;
         }
         laP->rc = EFAULT;
         C327TRACE4("Bwr6",dev,laNum,laP->rc);
      }
      else {
         /*
         ** make sure the copy into the buffer was successful
         */
         laP->writeBuffer = dbP;
         laP->writeBuffer_used = FALSE;
         C327TRACE3("Twr1", dbP, cnt);
      }
   }

   /*
   ** after you have enqued the entire buffer, send it to offlevel
   */

   C327TRACE3("Twr2",laP->rc,getLinkState(laP));
   if(!laP->rc){
      /*
      ** we are now doing Enter Action Write stuff
      */
      return_code = dftnsWrite(laP->la_netID,
                               laP->writeBuffer, laP->BSC_mode);

      if(return_code == 0x100) {
         /*
         ** if 256 returned then write is complete
         */
         if (laP->non_sna == TRUE) {
            /* state is data pending read */
            setLinkState(laP, LS_DPE);
         }
         else {
            /* SNA and we have responded to the bind. State back to normal */
            setLinkState(laP, LS_NORMAL);
         }
         clearIoInProg(laP);
      } else if (return_code) {
         /*
         ** if there is an error then terminate the data
         ** transfer. Don't overwrite the link state if there
         ** is host contention so that the contending read
         ** proceeds normally.
         */
         if (return_code != RC_HOST_CONTEN)
            setLinkState(laP, LS_NORMAL);
         if (laP->writeBuffer)
            laP->writeBuffer_used = TRUE;

         laP->rc = EIO;
         C327TRACE4("Bwr7",dev,laNum,laP->rc);
      }
   } else
      setLinkState(laP, LS_NORMAL);

   C327TRACE4("Twr3",laP->rc,getLinkState(laP),return_code);
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       mwait_for_unmlocked ()
**
** Description:
**              sleep until the link device is not mlocked (unmlocked).
**
** Inputs:      laP     pointer to the link address structure
**              laNum   link address number
**              module  module ID
**
** Output:      SET     indicates that you were wokeup while
**                      waiting to use the link
**              NULL    indicates that you now own the link
**
** Externals    mlnk_ptrs
** Referenced
**
** Externals    mlnk_ptrs
**
********************************************************************/

void mwait_for_unmlocked (linkAddr *laP, int laNum, int dev)
{
   uint    linkState;
   int     plX;

   C327TRACE5("Uwr0",dev,laNum,getLinkState(laP),laP->mlocked);

   DISABLE_INTERRUPTS(plX);
   if (laP->io_flags == WDI_LOCKED && !laP->mlocked)
      laP->mlocked = SET;
   RESTORE_INTERRUPTS(plX);

   if(laP->mlocked) {
      linkState = getLinkState(laP);

      /*
      ** if last outbound command was a read command
      */
      if(linkState == LS_WAIT_SYSWRITE){
         /*
         ** send status OKDATA (0x001) - which
         ** indicates that a read command received from
         ** the host was valid and that a write command
         ** will follow
         */
         if(mdepSendStatus(dev, laNum, 0x0001) ==
                 MDEP_FAILURE)
         {
            laP->rc = EIO;
            C327TRACE5("Bwr8",dev,
                       laNum,linkState,laP->rc);
         }
         laP->read_mod = TRUE;
         return;
      }

      DISABLE_INTERRUPTS(plX);
      while(laP->mlocked && !laP->rc){
         laP->waiting_for_unmlocked = SET;

#ifdef _POWER_MP
        if (e_sleep_thread(&laP->sleep_waiting_for_unmlocked, &c327_intr_lock,
            LOCK_HANDLER | INTERRUPTIBLE) == THREAD_INTERRUPTED)
#else
        if(e_sleep((void *)&laP->sleep_waiting_for_unmlocked,
                       EVENT_SIGRET) == EVENT_SIG)
#endif
        {
            laP->rc = EINTR;
            laP->waiting_for_unmlocked = 0;
         }
         if(isCardDown(dev)){
            laP->rc = EFAULT;
         }
      }
      C327UNPTRACE4("Uwr1",laP->rc,laP->mlocked,
      laP->waiting_for_unmlocked);
      laP->waiting_for_unmlocked = 0;
      laP->mlocked = 0;
      RESTORE_INTERRUPTS(plX);
   }

}

/*******************************************************************
**
** Function Name:       mwait_for_writebuf ()
**
** Description:
**              sleep until the write buffer is no longer used.
**
** Inputs:      laP     pointer to the link address structure
**              laNum   link address number
**              module  module ID
**
**
** Externals
** Referenced
**
** Externals
**
********************************************************************/
void mwait_for_writebuf (linkAddr *laP, int laNum, int dev)
{
        int plX;

        C327TRACE5("Uwr2",dev,laNum,getLinkState(laP),laP->writeBuffer_used);

        DISABLE_INTERRUPTS(plX);
        while (!laP->writeBuffer_used && !laP->rc) {
#ifdef _POWER_MP
            if (e_sleep_thread(&laP->sleep_waiting_for_write_buf,
               &c327_intr_lock, LOCK_HANDLER | INTERRUPTIBLE)
               == THREAD_INTERRUPTED)
#else
             if(e_sleep((void *)&laP->sleep_waiting_for_write_buf,
                       EVENT_SIGRET) == EVENT_SIG)
#endif
             {
                 laP->rc = EINTR;
             }
        }
        RESTORE_INTERRUPTS(plX);
}
