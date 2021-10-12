static char sccsid[] = "@(#)39  1.23.1.2 src/bos/kernext/c327/tcaioctl.c, sysxc327, bos411, 9430C411a 7/27/94 09:33:58";
/*
 * COMPONENT_NAME: (SYSXC327) c327 tca device driver ioctl handler
 *
 * FUNCTIONS:    GetInfo(), mreset_link(), mtca_inquire_link(),
 *               mwait_for_io(), tcaioctl(), wakeup_mwait_for_io(),
 *               wdcAuto(), wdcSstat(), wdcWaitclear() 
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
#include <sys/intr.h>
#include <sys/ioctl.h>
#include <sys/sleep.h>
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
static void mreset_link (linkAddr *, int, int);
static void wdcAuto     (int, linkAddr *, int, int);
static void wdcSstat    (linkAddr *, int, int, int);
static void wdcWaitclear(linkAddr *, caddr_t, int);
static void GetInfo (int, iocinfo *, int, linkAddr *);
/*PAGE*/
/*******************************************************************
**
** Function Name:       tcaioctl()
**
** Description:         calls the appropriate ioctl routine base on
**                      the value of cmd.  The valid ioctl commands
**                      are: inquire link, send status, issue a
**                      power on reset, and inquire information about
**                      the device.
**
** Inputs:      dev     device major/minor number
**              cmd     ioctl command
**              arg     command specific argument
**              flag    currently unused
**              laNum   unique number indicating each process attempting
**                      to access a link address
**
** Output:      void
**
** Externals    mlnk_ptrs
** Referenced
**
** Externals    mlnk_ptrs
** Modified
**
********************************************************************/

int tcaioctl (dev_t devt, int cmd, int arg, int flag, int laNum)
{

   linkAddr  *laP;
   int dev;

   dev = minor(devt);

   laP = tca_data[dev].mlnk_ptrs[laNum];

   laP->rc = 0;/* set errno rc to zero */

   C327TRACE5("IocS",dev,laNum,cmd,laP);

   switch(cmd){
      case WDC_INQ:
         mtca_inquire_link(laP, (iotca *)arg, laNum, (int)dev);
         break;
      case WDC_SSTAT:      /* cmd = 0x7702 */
         /* Block this command for SNA */
         if (laP->non_sna) {
            wdcSstat(laP, arg, laNum, (int)dev);
         }
         else {
            return (EINVAL);
         }
         break;
      case WDC_POR:
         mreset_link(laP, laNum, (int)dev);
         break;
      case IOCINFO:                /* cmd = 0xff01 */
         GetInfo(laNum, (iocinfo *)arg, (int)dev, laP);
         break;
      case WDC_AUTO:
         /* block this command for SNA */
         if (laP->non_sna) {
            wdcAuto((int)dev, laP, (int)arg, laNum);
         }
         else {
            return (EINVAL);
         }
         break;
      case WDC_WAITCLEAR:
         wdcWaitclear(laP, (caddr_t)arg, laNum);
         break;
      default:
         break;
   }

   C327TRACE3("IocE",getLinkState(laP), laP->rc);

   return(laP->rc);
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       mtca_inquire_link()
**
** Description:
**              copy the iostatus and ioflags values out of the link address
**              structure and into the user area.  Zero the link address
**              status and flags after the copy, provided that the
**              status does not indicate that the link was not successfully
**              enabled.
**
** Inputs:      laP             pointer to the link address struct for
**                              this link address
**              arg             pointer to the user's iotca structure
**
** Output:      void
**
** Externals    mlnk_ptrs
**
** Externals    mlnk_ptrs
**
********************************************************************/
void mtca_inquire_link (linkAddr *laP, iotca *arg, int laNum, int dev)
{
   int     plX;

   C327UNPTRACE4("Mio1",laP->io_flags,laP->io_status,laP->io_extra);
 
   if(copyout((void *)&laP->io_flags, (void *)arg, sizeof(iotca))){
      C327UNPTRACE5("Bio7",dev,laNum,laP->io_flags,arg);
      laP->rc = EFAULT;
   }
   
   C327UNPTRACE4("Mio2",arg->io_flags,arg->io_status,arg->io_extra);
   

   /*
   ** if link address enabled and card is up - adjust io_flags if data
   ** avail or chaining is set and clear remaining states of io_flags
   ** along with all states pertaining to io_status else close routine
   ** must not issue a halt svc against this link therefore the status
   ** must be preserverd
   */
   C327UNPTRACE4("Mio0",dev,laNum,getLinkState(laP));
   if(getLinkState(laP) != LS_LINKDOWN && isCardUp(dev)){

      /* preserve certain other status indicators */
      laP->io_flags &= (WDI_DAVAIL | WDI_LOCKED);
      laP->io_status = 0;
      laP->io_extra = 0;
   }
   C327UNPTRACE4("Mio1",laP->io_flags,laP->io_status,laP->io_extra);
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       wdcSstat()
**
** Description:
**              send status, concerning the last data stream
**              received by the workstation, to the host.
**
** Inputs:      laP     pointer to the link address struct for
**                      this link address
**              status  status to send to the device card
**
** Output:      void
**
** Externals    mlnk_ptrs
**
** Externals    mlnk_ptrs
**
********************************************************************/

static void wdcSstat (linkAddr *laP, int status, int laNum, int dev)
{
   register int    return_code = 0;
   int     plX;

   /* C327PERF( 0x0850 ); */

   C327TRACE5("Sio0",dev,laNum,laP,status);

   /*
   ** wait until there isn't any outstanding I/O to the AEA for
   ** this link address.  Although, if it's a reset then just
   ** send it down!!
   */
   if(status != STAT_RESET){
      /* C327PERF( 0x0950 ); */
      return_code = mwait_for_io(laP, laNum, Send_Stat_Ioctl, dev);
      /* C327PERF( 0x0951 ); */
   } else {
      return_code = 0;
   }
   C327TRACE2("Sio1",return_code);

   if(!return_code){       /* check to see if we were interrupted */
      /* make sure that there isn't an error or data available */
      if(laP->io_flags & (WDI_ALL_CHECK | WDI_DAVAIL)){
         laP->rc = EIO;
         C327UNPTRACE5("Bio8",dev,laNum,laP->io_flags,laP->rc);
         return_code = -1;
      }

      /*
      ** send the status down to the offlevel handler as long as
      ** an error hasn't occurred up to this point
      */
      if(!return_code){
         if(mdepSendStatus(dev, laNum, status) ==
                 MDEP_FAILURE){
            laP->rc = EIO;
            C327TRACE4("Bio9",dev,
                       laNum,getLinkState(laP));
         }
      }
   }

   wakeup_mwait_for_io(laP);      /* wakeup anyone waiting to use link */

   /*
   ** if this is an acknowledgement for read, and we are waiting for
   ** the appl. to send the read ack., change state to LS_NORMAL.
   */
   C327TRACE2("Sio2",getLinkState(laP));

   DISABLE_INTERRUPTS(plX);
   if((status == STAT_READ || status == STAT_ACK) &&
          getLinkState(laP) == LS_W_WAIT_READACK){
      setLinkState(laP, LS_NORMAL);
   }
   RESTORE_INTERRUPTS(plX);
   /* C327PERF( 0x0851 ); */
   return;
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       mreset_link()
**
** Description: performs power on reset of link address
**              ( halt and start the link address )
**
** Inputs:      laP     pointer to the link address struct for
**                              this link address
**
** Output:      void
**
** Externals    link_addr
** Referenced
**
** Externals    link_addr
** Modified
**
********************************************************************/

void mreset_link (linkAddr *laP, int laNum, int dev)
{
   int     plX;
   int     returnCode;

   /*
   ** wait until there isn't any more outstanding I/O on this
   ** link address
   */
   returnCode = mwait_for_io(laP, laNum, Power_On_Reset_Ioctl, dev);
   C327TRACE5("Rio0",dev,laNum,laP,returnCode);
   /* make sure we were not interrupted out of our sleep */
   if(!returnCode){

      setLinkState(laP, LS_RESETTING);

      /*
      ** wakeup anyone that might be waiting to perform I/O on this
      ** link address
      */
      /* DISABLE_INTERRUPTS(plX) */

      wakeup_mwait_for_io(laP);

      /* set io_in_prog to block out everyone else */
      laP->io_in_prog = SET;

      /* wakeup anyone that might be waiting for data */
      if(laP->waiting_for_data){
         C327UNPTRACE2("Rio1",laP);
         laP->waiting_for_data = CLEAR;
         e_wakeup( (void *)&laP->sleep_waiting_for_data );
      }

      /* if link is up */
      C327UNPTRACE2("Rio2",getLinkState(laP));

      if(getLinkState(laP) != LS_LINKDOWN){
         /* issue a Halt SVC for laNum */
         /* RESTORE_INTERRUPTS(plX) */
         mdepHaltLA(laNum, laP, dev);
         /* DISABLE_INTERRUPTS(plX) */

         if (laP->writeBuffer) {
            C327TRACE2("Rio3",laP->writeBuffer);
            laP->writeBuffer_used = TRUE;
            e_wakeup((void *)&laP->sleep_waiting_for_write_buf);
         }
      }

      /*
      ** provided an error did not occur during the halt then
      ** perform the start
      */
      /*
      ** start link address - if the mdepStartLA() failed
      ** or if the link state is LS_LINKDOWN, then an
      ** error occurred.
      */
      if(!laP->rc){
         /* RESTORE_INTERRUPTS(plX) */
         if(mdepStartLA(laP, laNum, Power_On_Reset_Ioctl, dev, (void *)NULL)
                    == MDEP_FAILURE || getLinkState(laP) == LS_LINKDOWN){
            if(!laP->rc)
               laP->rc = EIO;        /* set it */
            C327TRACE5("Bio3",dev,laNum,
                       getLinkState(laP),
                       laP->rc);
         }
         /* DISABLE_INTERRUPTS(plX) */
      }
      else{
         C327TRACE5("Bio4",dev,laNum,
                    getLinkState(laP),laP->rc);
      }
   }

   /* wakeup anyone waiting for the I/O to finish */
   wakeup_mwait_for_io(laP);
   laP->mlocked = 0;               /* clear flags */
   if (laP->waiting_for_unmlocked){
      laP->waiting_for_unmlocked = 0;
      e_wakeup((void *)&laP->sleep_waiting_for_unmlocked);
   }
   C327TRACE2("Rio2",getLinkState(laP));

   /* RESTORE_INTERRUPTS(plX) */
   return;
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       wakeup_mwait_for_io ()
**
** Description:
**              clears the io_in_prog flag and wakeup anyone waiting
**              to use the link address
**
** Inputs:      laP     pointer to the link address structure
**
** Output:      void
**
** Externals    mlnk_ptrs
** Referenced
**
** Externals    mlnk_ptrs
** Modified
**
********************************************************************/

void wakeup_mwait_for_io (linkAddr *laP)
{
   laP->io_in_prog = CLEAR;

   if(laP->waiting_on_io_in_prog){
      C327TRACE2("Wio0",laP);
      e_wakeup((void *)&laP->sleep_waiting_on_io_in_prog);
   }
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       mwait_for_io ()
**
** Description:
**              sleep until the link is free.  Set the flag
**              indicating that you now own it.
**
** Inputs:      laP     pointer to the link address structure
**              laNum   link address number
**              module  module ID
**
** Output:      SET     indicates that you were wokeup while
**                      waiting to use the link
**              NULL    indicates that you now own the link
**
** Environment: can only be called from the process environment.
**              CANNOT hold the c327_lock when making this call.
**
**
** Externals    mlnk_ptrs
** Referenced
**
** Externals    mlnk_ptrs
**
********************************************************************/

int mwait_for_io (linkAddr *laP, int laNum, int module, int dev)
{
   int     returnCode = 0;
   int     plX;

   C327TRACE5("Wio1",dev,laNum,laP,module);
   DISABLE_INTERRUPTS(plX);

   /* sleep if link address IO in progress flag is set */
   while(laP->io_in_prog && !returnCode){
      laP->waiting_on_io_in_prog = SET;

#ifdef _POWER_MP
      if (e_sleep_thread(&laP->sleep_waiting_on_io_in_prog,
         &c327_intr_lock, LOCK_HANDLER | INTERRUPTIBLE)
         == THREAD_INTERRUPTED)
#else
      if (e_sleep((void *)&laP->sleep_waiting_on_io_in_prog,
                EVENT_SIGRET) == EVENT_SIG)
#endif
     {
         C327UNPTRACE5("Bioa",dev,laNum,
                       laP->waiting_on_io_in_prog,laP->rc);
         laP->rc = EINTR;
         laP->waiting_on_io_in_prog = CLEAR;
         returnCode = SET;
      }
      if(isCardDown(dev)){
         laP->rc = EFAULT;
         C327UNPTRACE5("Bio5",dev,laNum,isCardDown(dev),laP->rc);
         returnCode = SET;
      }
   }

   /* turn on the flag indicating that we own the link address */
   laP->io_in_prog = SET;


   RESTORE_INTERRUPTS(plX);
   C327TRACE3("Wio2",returnCode,laP->rc);
   return(returnCode);
}
/*PAGE*/
/*******************************************************************
**
** Function Name:       wdcAuto()
**
** Description:         enable or disable auto acknowledgement
**                      of outbound data streams
**
** Inputs:              laP             pointer to the link address struct for
**                                      this link address
**                      ackState        New state for auto. acknowledge
**                      laNum           the number of the link address
**
** Output:              void
**
** Externals    -       lnk_ptrs
**
** Externals    -       lnk_ptrs
**
********************************************************************/

static void wdcAuto (int dev, linkAddr *laP, int ackState, int laNum)
{
   int     plX;
   Data_Buffer    *tmp_buf;

   /* XXX - these micro disables are killin me! */
   DISABLE_INTERRUPTS(plX);

   if(ackState){   /* turn on the auto acknowledgement */
      /* if haven't already, allocate extra space for auto ack */
      if(!laP->la_WSFdbP){
         RESTORE_INTERRUPTS(plX);
         /* C327PERF( 0x0710 ); */
         tmp_buf = bufferAlloc(dev, laP->la_WSFdbP,
                               (struct uio *)NULL);
         /* C327PERF( 0x0711 ); */
         DISABLE_INTERRUPTS(plX);
         laP->la_WSFdbP = tmp_buf;

         /* la_WSFdbP is free to use */
         laP->waiting_for_WSFdb = 0;
      }
      laP->la_autoAck = TRUE;
   } else {        /* turn off the auto acknowledgement */
      if(laP->la_WSFdbP){
         RESTORE_INTERRUPTS(plX);
         bufferFree(laP->la_WSFdbP);
         laP->la_WSFdbP = NULL;
         DISABLE_INTERRUPTS(plX);
      }
      laP->la_autoAck = FALSE;
   }

   RESTORE_INTERRUPTS(plX);

   C327TRACE5("Aio0",dev,laNum,laP->la_autoAck,ackState);
   return;
}
/*PAGE*/
/*
* --------------------------------------------------------------------- *
* Function:
* --------------------------------------------------------------------- *
*/
void wdcWaitclear(linkAddr *laP, caddr_t arg, int laNum)
{
   iotca   uIotca;
   int     plX;

   if(copyin((void *)arg, (void *)&uIotca, sizeof(iotca)))
   {
      C327TRACE5("Bio0",laNum,arg,&uIotca,sizeof(iotca));
      return;
   }

   DISABLE_INTERRUPTS(plX);

   /*
   ** if this is the same COMM, PROG or MACH error and the same
   ** status as specificed in user argument, then goto sleep.
   ** Once woken up, check if due to signal else check if and
   ** any COMM, PROG or MACH bits set, if so set EIO otherwise
   ** wakeup due to a broadcast clear.
   */

   C327UNPTRACE5("Cio0",laNum,laP->io_flags,uIotca.io_flags,arg);
   C327UNPTRACE5("Cio1",laP->io_status,uIotca.io_status,
                 laP->la_waitClear,laP->rc);

   if(((laP->io_flags & uIotca.io_flags) & WDI_ALL_CHECK) &&
       laP->io_status == uIotca.io_status)
   {

      C327UNPTRACE3("Cio2",laNum,getLinkState(laP));

      laP->la_waitClear = 1;

#ifdef _POWER_MP
      if (e_sleep_thread(&laP->sleep_la_waitClear, &c327_intr_lock,
         LOCK_HANDLER | INTERRUPTIBLE) == THREAD_INTERRUPTED)
#else
      if(e_sleep((void *)&laP->sleep_la_waitClear, EVENT_SIGRET)
         == EVENT_SIG)
#endif
      {
         C327UNPTRACE4("Bio1",laNum,&laP->la_waitClear,
                       laP->rc);
         laP->rc = EINTR;
      }
      else if(laP->io_flags & WDI_ALL_CHECK)
      {
         C327UNPTRACE5("Bio2",laNum,laP->io_flags,
                       WDI_ALL_CHECK,laP->rc);
         laP->rc = EIO;
      }
   }

   C327UNPTRACE2("Cio3",getLinkState(laP));
   RESTORE_INTERRUPTS(plX);
}
/*PAGE*/
/*******************************************************************
**
** Function Name:  GetInfo()
**
** Description:
**             returns the device type, the link address and the define device
**              structure (DDS) associated with a file descriptor
**
** Inputs:      laNum   number of the link address
**              info_ptr                pointer to the tcadinfo struct
**
** Output:      {MDEP_SUCCESS}  Get info suceeded
**              {MDEP_FAILURE}  Get info failed
**
** Externals  
** Referenced
**
** Externals  
** Modified
**
********************************************************************/
static void GetInfo (int laNum, iocinfo *info_ptr, int dev, linkAddr *laP)
{
   unsigned int cont;/* controller type */
   unsigned char cuat;/* attachment protocol type */

   C327TRACE1("GmdS");


   if (tca_data[dev].CONTROLLER_TYPE1)
   {
      cont = tca_data[dev].CONTROLLER_TYPE1;
   }
   else
   {
      if(tca_data[dev].CONTROLLER_TYPE2)
         cont = tca_data[dev].CONTROLLER_TYPE2;
      else
         cont = 0x00f4;/* fake controller response */
   }

   cuat = tca_data[dev].ATTACHMENT_TYPE;


   /* copy the link address number into the user memory */

   info_ptr->chan = laNum;
   info_ptr->controller = cont;
   info_ptr->attach_protocol = cuat;

   C327TRACE3("GmdE",info_ptr->controller, info_ptr->attach_protocol);

}
