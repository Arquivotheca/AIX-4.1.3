static char sccsid[] = "@(#)41	1.22  src/bos/kernext/c327/tcaread.c, sysxc327, bos411, 9430C411a 7/27/94 09:34:29";
/*
 * COMPONENT_NAME: (SYSXC327) c327 tca device driver entry points
 *
 * FUNCTIONS:    mcopy_buffer(), mcopy_tca_cmd(), mfree_read_buffer()
 *               mtca_read_routine(), mwait_for_data(), tcaread()
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
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <sys/sleep.h>
#include <sys/intr.h>
#include <sys/io3270.h>
#include "c327dd.h"
#include "dftnsDcl.h"
#include "tcadefs.h"
#include "tcadecls.h"
#include "tca3270.h"
#include "tcaexterns.h"

#ifdef _POWER_MP
#include <sys/lock_def.h> 
extern Simple_lock c327_intr_lock;
#endif

/*
** static function prototypes
*/
static int mwait_for_data (linkAddr *, int, int);
int mcopy_tca_cmd (Data_Buffer *, linkAddr *, struct uio *);
static void mtca_read_routine (linkAddr *, int, int, struct uio *);
int mcopy_buffer (int, Data_Buffer *, linkAddr *, struct uio *);
void mfree_read_buffer (Data_Buffer *, linkAddr *, int, int);


/*PAGE*/
/*******************************************************************
**
** Function Name:       tcaread()
**
** Description: wait until there isn't another I/O command
**              outstanding for this link address.  Wait until
**              data becomes available.  Read the data from the
**              buffers to user memory until: 1) the
**              end of the host message occurs, 2) run out
**              of room in the user buffer, or 3) an error
**              occurs.  Wakeup anyone waiting to perform I/O
**
** Inputs:      dev             device major/minor number
**              uiop            ptr to uio struct, specifying location
**                              and length of user area
**              chan            unique number indicating each process
**                              attempting to access a link address
**              ext             ptr to a iotca structure or 0
**
** Output:      void
**
** Externals    mlnk_ptrs[]
** Referenced
**
** Externals    mlnk_ptrs[]
** Modified
**
********************************************************************/
int tcaread (dev_t devt, struct uio *uiop, int laNum, int ext)
{

   register linkAddr  *laP;
   register int       error=0;
   int                plX, dev;
   DDS_DATA           *dds_ptr;
   int                session_index;

   dev = minor(devt);

   C327TRACE5("ReaZ", dev, uiop, laNum, ext);

   laP = tca_data[dev].mlnk_ptrs[laNum];
   C327TRACE2("ReaS", laP);

   if(laP) laP->rc = 0;

   /* wait for linkState to become one of {READ_STATES} */
   if(waitForLinkState(laP, READ_STATES) == 0)
      return(EINTR);

   /* C327PERF( 0x0800 ); */
   C327TRACE5("Rrd1",dev,laNum,laP->io_flags,laP->io_status);

   /*
   ** make sure the status is nonzero and that there isn't
   ** data available
   */
   if (laP->io_flags & WDI_ALL_CHECK){
      /* indicate that the status of the link was non zero */
      C327TRACE4("Brd1",dev,laNum,EIO);
      return(EIO);
   }

   /* If collision (lock code didn't catch it), discard write and clear */
   /* io_in_prog flag                                                   */

   /* 1. get session index
    * 2. If COLLISION
    *    Interrupt device head to discard write
    *    Call AckCmd to complete command and clean up flags
    * 3. process read as normal
    */

   /* Get network_id, get dds_ptr, & session_index */
   ValidNetID(laP->la_netID,&dds_ptr,&session_index);

   if (laP->non_sna) {

       /* If Write/Read Collision then discard write and clear flags */
       if (! laP->writeBuffer_used )
       {
         InterruptDH (dds_ptr, session_index, INTR_UNSOL,
                      OR_WRITE_DISCARD, 0);
         AckCmd (dds_ptr, session_index, INTR_SOL_WRITE);
       }

       /*
       ** while link address io_in_prog set sleep on address of
       ** io_in_prog
       */
       /* If SNA, do not do flow control. Only wait if running non-SNA */
       error = mwait_for_io(laP, laNum, Read, dev);
   }


   /* make sure we weren't interrupted out of sleep */
   if(!error){

      /* wait until data becomes available */
      if(!(laP->rc = mwait_for_data(laP, laNum, dev))){
         mtca_read_routine(laP, laNum, dev, uiop);
         if (laP->non_sna == FALSE) {
            dftnsSend (laP->la_netID,TCA_FC);  /* send a function complete */
         }
      }
   }
   /* wakeup anyone waiting on io */
   wakeup_mwait_for_io ( laP );

   /*
   ** if a write command which the emulator thought was sent OK was
   ** discarded, tell the emulator now
   */
   if (laP->write_discarded){
      laP->write_discarded = FALSE;
      laP->io_flags |= WDI_COMM;
      laP->io_status = WEC_585;
   }

   /* check to see if you can copy the status into the user area */
   if(ext && !laP->rc){
      mtca_inquire_link( laP, (iotca *)ext, laNum, dev );
   }
   /* C327PERF( 0x0801 ); */
                           
   C327TRACE2("ReaE",laP->rc);
   return(laP->rc);
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       mtca_read_routine
**
** Description: read data out of buffers into
**              user memory.  In a large loop:  read the
**              tca command and then read the data from
**              buffers until the host message completes,
**              you run out of room in the user buffer, or
**              an error occurs.  Update the link status after
**              time you free every buffer.
**
** Inputs:      laP     pointer to the link address struct
**              laNum   link address number
**
** Output:
**
** Externals    mlnk_ptrs
** Referenced
**
** Externals    mlnk_ptrs
** Modified
**
********************************************************************/

void mtca_read_routine (linkAddr *laP, int laNum, int dev, struct uio *uiop)
{
   register int error = 0;
   register int cnt_to_transfer = 0;
   register Data_Buffer    *dbP;
   int plX;

   /* get the address of the data buffer */
   DISABLE_INTERRUPTS(plX);           /* disable interrupts */
   if(laP->la_autoAck)
      dbP = laP->la_WSFdbP;
   else
     dbP = laP->la_recvDbP;
   RESTORE_INTERRUPTS(plX);       /* restore interrupts to prev. level */

   /* copy the tca command into user area */
   error = mcopy_tca_cmd(dbP, laP, uiop);
   if (error)
     return;

   /*
   ** Copy data out of the buffer area to the user area.
   ** Copy the minimum of what the user asked for and what is
   ** available.
   */
   if (uiop->uio_resid >=
              (dbP->dbhead.data_len - dbP->dbhead.data_offset))
      cnt_to_transfer = dbP->dbhead.data_len -
              dbP->dbhead.data_offset;
   else
      cnt_to_transfer = uiop->uio_resid;

   C327TRACE4("Mrd0", dbP->dbhead.data_len,
              dbP->dbhead.data_offset, cnt_to_transfer);

   /* transfer the data from the buffer to user memory */
   error = mcopy_buffer(cnt_to_transfer, dbP, laP, uiop);
   if (error)
      return;

   /* test to see if entire buffer was read */
   if (bufferEmpty(dbP)) {
      /* All requested data has been copied to user area */
      mfree_read_buffer(dbP, laP, dev, laNum);
      laP->io_flags &= ~WDI_DAVAIL;
   } else {
      laP->io_flags |= WDI_DAVAIL;
   }

   C327TRACE4("Mrd2",laP->io_flags,laP->io_status,error);
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       mwait_for_data()
**
** Description: wait for data on the device ring queue.
**
** Inputs:      laP     pointer to the link address struct
**              laNum   link address number
**
** Output:      NULL    there is data available on the device ring queue
**              EINTR   process killed while sleeping for data
**
** Externals    mlnk_ptrs
** Referenced
**
** Externals    mlnk_ptrs
** Modified
**
********************************************************************/

static int mwait_for_data (linkAddr *laP, int laNum, int dev)
{
   int     returnCode = 0;
   int     plX;

   C327TRACE5("Wrd0",dev,laNum,laP->io_flags,laP->io_status);

   DISABLE_INTERRUPTS(plX);

   /* test to see if the device ring queue is empty */
   if(!(laP->io_flags & WDI_DAVAIL )) {
      /* set link address waiting_for_data flag */
      laP->waiting_for_data++;

      /* sleep on link address waiting_for_data flag */
#ifdef _POWER_MP
      if (e_sleep_thread(&laP->sleep_waiting_for_data, &c327_intr_lock,
         LOCK_HANDLER | INTERRUPTIBLE) == THREAD_INTERRUPTED)
#else
      if(e_sleep((void *)&laP->sleep_waiting_for_data, EVENT_SIGRET)
                 == EVENT_SIG)
#endif
     {
         /* woken up by a SIGKILL */
         RESTORE_INTERRUPTS(plX);
         C327TRACE5("Brd2",dev,laNum,&laP->waiting_for_data,
                    EINTR);
         return(EINTR);
      }

      if(isCardDown(dev)){
         returnCode = EFAULT;
         C327UNPTRACE4("Brd4",dev,laNum,EFAULT);
      }
   }

   RESTORE_INTERRUPTS(plX);

   return(returnCode);
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       mcopy_tca_cmd()
**
** Description: copy tca command out of buffer
**              into user memory
**
** Inputs:      dbP     pointer to the buffer
**
** Output:      NULL    there is data available on the device ring queue
**              EFAULT  error while copying cmd out of the buffer
**
** Externals    
** Referenced  
**
** Externals  
** Modified  
**
********************************************************************/

int mcopy_tca_cmd (Data_Buffer *dbP, linkAddr *laP, struct uio *uiop)
{
   char    *dataStream;
   int     plX;

   dataStream = &dbP->buf_start;
   C327TRACE5("Crd0",laP, dbP->buf_start,*(&dbP->buf_start+1),
   getLinkState(laP));

   /*
   ** check to see if we have already read the tca command
   ** ( we zero out the wcb after reading it )
   */
   if(*dataStream != 0){
      switch(*dataStream){
         case cmdWRT:            /* write command */
            *dataStream = 0x01;
            if(laP->la_autoAck) 
               setLinkState(laP, LS_AA_RD_OF_WRT_INPROG);
            else
               setLinkState(laP, LS_RD_OF_WRT_INPROG);
            break;
         case cmdEWRT:           /* erase/write command */
            *dataStream = 0x03;
            if(laP->la_autoAck)
               setLinkState(laP, LS_AA_RD_OF_WRT_INPROG);
            else
               setLinkState(laP, LS_RD_OF_WRT_INPROG);
            break;
         case cmdEWRA:           /* erase/write alternate */
            *dataStream = 0x0D;
            if(laP->la_autoAck) 
               setLinkState(laP, LS_AA_RD_OF_WRT_INPROG );
            else
               setLinkState(laP, LS_RD_OF_WRT_INPROG);
            break;
         case cmdEAU:            /* erase all unprotected */
            *dataStream = 0x0F;
            if(laP->la_autoAck) 
               setLinkState(laP, LS_AA_RD_OF_WRT_INPROG);
            else
               setLinkState(laP, LS_RD_OF_WRT_INPROG);
            break;
         case cmdWSF:            /* write structured field */
            if(laP->la_autoAck)  {
               setLinkState(laP, LS_AA_RD_OF_WRT_INPROG);
               /*
               ** check to see if this API data
               */
               if (isApiCmdWSF(dataStream))
                  laP->api_cmdWSF = 1;
               else
                  laP->api_cmdWSF = 0;
            } else
               setLinkState(laP, LS_RD_OF_WRT_INPROG);
            *dataStream = 0x11;
            break;
         case cmdRDBF:           /* read buffer */
            *dataStream = 0x02;
            setLinkState(laP, LS_RD_OF_RD_INPROG);
            break;
         case cmdRDMD:           /* read modified */
            *dataStream = 0x06;
            setLinkState(laP, LS_RD_OF_RD_INPROG);
            break;
      }

      /* If we are on an SNA cu, we need to force the state back to normal */
      if (laP->non_sna == FALSE) {
         setLinkState(laP, LS_AA_RD_OF_WRT_INPROG);
      }

      C327TRACE3("Crd1",laP->la_autoAck,getLinkState(laP));

      /* test read modifieds in the v3 platform...*/

      /* Copy the command byte to user area */
      laP->rc = uiomove(dataStream, 1, UIO_READ, uiop);

      dbP->dbhead.data_offset++; /* read in the first byte */

      /* zero out the wcb so that we don't read the wcb twice */
      *dataStream = 0;
   }

   return(laP->rc);
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       mcopy_buffer()
**
** Description: copy a host message out of a buffer
**              into user memory
**
** Inputs:      cnt     number of bytes to transfer from the buffer
**                      into user memory
**              dbP     pointer to the buffer
**              laP     link address
**
** Output:      NULL    there is data available on the device ring queue
**              EFAULT  error while copying cmd out of the buffer
**
** Externals    
** Referenced  
**
** Externals  
** Modified  
**
********************************************************************/

int mcopy_buffer ( int cnt_to_transfer, Data_Buffer *dbP, 
                   linkAddr *laP, struct uio *uiop)
{
   /*
   ** transfer the data from the kernel buffer to user memory,
   ** the residual cnt of bytes to be moved is automatically updated
   */

   laP->rc = uiomove((caddr_t)
                     (&dbP->buf_start + dbP->dbhead.data_offset),
                     cnt_to_transfer, UIO_READ, uiop);

   if(laP->rc){
      /* set error flag if copy failed to EFAULT */
      laP->rc = EFAULT;
      C327TRACE5("Brd5",cnt_to_transfer,laP->rc,uiop->uio_resid,
                 &dbP->buf_start);
      return(laP->rc);
   }

   /* increment data_offset by cnt */
   dbP->dbhead.data_offset += cnt_to_transfer;

   C327TRACE3("Grd0", cnt_to_transfer, uiop->uio_resid);

   return(0);
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       mfree_read_buffer()
**
** Description: free the buffer.  Wait for further
**              host data if there is more to the host
**              message.
**
** Inputs:      laP             pointer to link address structure
**
** Output:
**
** Externals    
** Referenced   
**
** Externals   
** Modified   
**
********************************************************************/

void mfree_read_buffer (Data_Buffer *dbP, linkAddr *laP, int dev, int laNum)
{
   int     plX;

   C327TRACE3("Frd0",dev,getLinkState(laP));

   DISABLE_INTERRUPTS(plX);           /* disable interrupts */

   /* Indicate that the buffer is empty */
   dbP->dbhead.data_offset = 0;
   dbP->dbhead.data_len = 0;


   switch(getLinkState(laP)){
      case LS_RD_OF_WRT_INPROG:
         /* Completed read of write data */
         setLinkState(laP, LS_W_WAIT_READACK);
         break;
      case LS_AA_RD_OF_WRT_INPROG:
         /*
         ** Completed read of write data that was autoack'ed.
         ** Since ack has been done,  set link state back to normal
         */
         setLinkState(laP, LS_NORMAL);

         /*
         ** If in auto ack mode waiting for la_WSFdbP to become free
         ** and it now has been completly read, copy la_recvDbP
         ** to la_WSFdbP and send status.
         */
         if (laP->waiting_for_WSFdb) {

            /*
            ** copy received data to a tca buffer so we can
            ** send status to the controller immediately w/o
            ** possible data loss
            */

            /* C327PERF( 0x0910 ); */

	   /* XXX - bcopy with interrupts disabled? */
            bcopy((void *)laP->la_recvDbP, (void *)laP->la_WSFdbP,
                  (uint)(laP->la_WSFdbP->dbhead.buff_size +
                  sizeof(struct dbhead)));

            /* C327PERF( 0x0911 );*/

            /* No longer waiting for WSFdb to be free */
            laP->waiting_for_WSFdb = 0;

            /* Data now available for the application */
            laP->io_flags |= WDI_DAVAIL;

            /*
            ** check to see if this API data; if yes send status
            */
            if (laP->api_cmdWSF)
               /*
               ** set up as if buffer had already been read
               */
               setLinkState(laP, LS_AA_INPROG);

            RESTORE_INTERRUPTS(plX);
            mdepSendStatus(dev,laNum,0);
            DISABLE_INTERRUPTS(plX);           /* disable interrupts */
         }
         break;
      case LS_RD_OF_RD_INPROG:
         /* Completed read of read data */
         setLinkState(laP, LS_WAIT_SYSWRITE);
         break;
   }

   RESTORE_INTERRUPTS(plX);

   C327TRACE2("Frd1",getLinkState(laP));
}
