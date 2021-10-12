static char sccsid[] = "@(#)38  1.32 src/bos/kernext/c327/tcaintr.c, sysxc327, bos411, 9430C411a 7/27/94 09:33:44";
/*
 * COMPONENT_NAME: (SYSXC327) c327 tca pinned subroutines
 *
 * FUNCTIONS:    aixSendStat(), broadcastClear(), checkForHostAck(),
 *    clearIoInProg(), doCheckStatus(), intrSolStart(), intrSolWrite(), 
 *    mbroadcast_status(), mupdate_link_fields(), mwake_em_up(), 
 *    netId2La(), procAutoAck(), tcaintr()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <sys/types.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/device.h>
#include <sys/io3270.h>
#include "c327dd.h"
#include "dftnsDcl.h"
#include "tcadefs.h"
#include "tcadecls.h"
#include "tca3270.h"
#include "tcaexterns.h"
/*
** static function prototypes
*/
static void mbroadcast_status(uint, uint, int, int, boolean);
static void procAutoAck(linkAddr *, int, int);
static void intrSolStart(int, NETWORK_ID, int);
static void doCheckStatus (int, linkAddr *, int, int);
static void intrUnsol(linkAddr *, int, int, int, int);
static void intrSolWrite (linkAddr *, int, int, int);
static void broadcastClear (int, linkAddr *, int, boolean);
static void mwake_em_up(int, linkAddr *, int);
int checkForHostAck (linkAddr *, int, int);
int netId2La (int, NETWORK_ID, linkAddr **, int *);

/*PAGE*/
/*
 * NAME: mupdate_link_fields()
 *                                                                    
 * FUNCTION: update the link status and flags if there is an error
 *           condition or data available.  If the process is
 *           selecting on read or excepition then issue a select.
 *           only used for unsolicited interrupts
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *     This should describe the execution environment for this
 *     procedure. For example, does it execute under a process,
 *     interrupt handler, or both. Can it page fault. How is
 *     it serialized.
 *                                                                   
 * (NOTES:) More detailed description of the function, down to
 *     what bits / data structures, etc it manipulates. 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *     software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */  

/*******************************************************************
**
** Function Name:       mupdate_link_fields()
**
** Description: update the link status and flags if there is an error
**              condition or data available.  If the process is
**              selecting on read or excepition then issue a select.
**              only used for unsolicited interrupts
**
** Inputs:      opResults       the return code returned 
**              link_address    pointer to the link address structure
**                              (see ../h/tcadecls.h)
**              laNum           link address number
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
void mupdate_link_fields (int opResults, linkAddr *laP, int laNum, int dev)
{
       C327TRACE5("Lin0",dev,laNum,opResults,getLinkState(laP));

       switch(opResults){
          case OR_NO_ERROR:
             break;
          case OR_CU_DOWN:       /* fatal card error */
             laP->io_flags &= WDI_ALL_CHECK;
             setCardDown(dev);       /* show card is down */
             setLinkState(laP, LS_LINKDOWN);
             if (laP->writeBuffer) {
                laP->writeBuffer_used = TRUE;
                e_wakeup((void *)&laP->sleep_waiting_for_write_buf);
             }
             break;
          case OR_WRITE_DISCARD:      /* write command was thrown away */
             laP->write_discarded = TRUE;
             if (laP->writeBuffer) {
                laP->writeBuffer_used = TRUE;
                e_wakeup((void *)&laP->sleep_waiting_for_write_buf);
             }
             break;
          case RC_NON_SUPP_CU:
             laP->io_flags &= ~WDI_ALL_CHECK;
             laP->io_flags |= WDI_COMM;
             laP->io_status = WEC_501;
             break;
          case RC_HOST_CONTEN:
             /*
             ** if a collision occurred - then change the state to where
             ** we are waiting for a read from the application
             */
             switch(getLinkState(laP)){
                case LS_EA_WRITE:
                   setLinkState(laP, LS_EA_WAIT_SYSREAD);
                   break;
                case LS_DPE_WRITE:
                   setLinkState(laP, LS_DPE_WAIT_SYSREAD);
                   break;
             }
             break;
          case RC_NO_SESS_AVAIL:       /* netID table full */
             laP->io_flags &= ~WDI_ALL_CHECK;
             laP->io_flags |= WDI_MACH;
             laP->io_status = WEP_FULL_NET;
             break;
          case OR_REC_DATA_AVAIL:
             /* C327PERF( 0x0700 ); */

             switch(getLinkState(laP)){
                case LS_DPE:
                case LS_RE:
                case LS_DPE_WRITE:
                   if(checkForHostAck(laP, laNum, dev))
                      return;
                   break;
                case LS_NORMAL:
                setLinkState(laP, LS_DATA_AVAIL);
                break;
             }

             laP->io_flags |= WDI_DAVAIL;    /* set data available flag*/
             /* C327PERF( 0x0701 ); */

             /* !!! really unlock - or should check keyb. reset bit */
             laP->mlocked = 0;               /* clear flags */
             if (laP->waiting_for_unmlocked){
                /* C327PERF( 0x0750 ); */
                /* wakeup wait for unlock */
                C327TRACE2("Lin3",laP);
                laP->waiting_for_unmlocked = 0;
                e_wakeup((void *)&laP->sleep_waiting_for_unmlocked);
             }

             /* -------------------------------------- */
             /* notify select READ or waiting for data */
             /* -------------------------------------- */
             if(laP->dev_selr){
                C327TRACE4("Lin4",laP->dev_selr,laP->dev_flags, dev);
                selnotify((int)
                          (makedev(((uint)c327_dev_major), ((uint)dev))),
                          laNum, (ushort) POLLIN);
                laP->dev_selr = 0;
                laP->dev_flags &= ~RCOL;
             }

             if(laP->waiting_for_data){
                C327TRACE2("Lin5",laP);
                laP->waiting_for_data = 0;
                e_wakeup((void *)&laP->sleep_waiting_for_data);
             }

             /* C327PERF( 0x0751 ); */
             procAutoAck(laP, laNum, dev);

             return;
          case OR_LOCK:        /* host locked */
             laP->mlocked = SET;
             if(isCardDown(dev))
                broadcastClear(dev, laP, laNum,(boolean) FALSE);
             laP->io_flags |= WDI_LOCKED;
             return;
          case OR_UNLOCK:     /* host unlocked */
             laP->mlocked = 0;
             laP->io_flags &= ~WDI_LOCKED;
             if((getLinkState(laP)==LS_RM_WRITE) ||
                (getLinkState(laP) == LS_DPE)    ||
                (laP->read_mod == TRUE)) {
                if(laP->writeBuffer) {
                   laP->writeBuffer_used = TRUE;
                   e_wakeup((void *)&laP->sleep_waiting_for_write_buf);
                   laP->read_mod = FALSE;
                }
             }

             if(laP->waiting_for_unmlocked){
                C327TRACE2("Lin6",laP);
                laP->waiting_for_unmlocked = 0;
                e_wakeup((void *)&laP->sleep_waiting_for_unmlocked);
             }
             if(isCardDown(dev)){
                C327TRACE4("Bin8",dev,laNum,getLinkState(laP));
                broadcastClear(dev, laP, laNum, (boolean) FALSE);
             }
             return;
          case RC_OPEN_TIMEOUT:        /* hardware problem */
             laP->io_flags &= ~WDI_ALL_CHECK;
             laP->io_flags |= WDI_MACH;
             laP->io_status = WEM_TIMEOUT;
             break;
          case RC_ADAPTER_FAIL:
             laP->io_flags &= ~WDI_ALL_CHECK;
             laP->io_flags |= WDI_MACH;
             laP->io_status = WEM_BAD_INIT;
             break;
          case RC_INVAL_NETID:    /* invalid network ID */
             laP->io_flags &= ~WDI_ALL_CHECK;
             laP->io_flags |= WDI_MACH;
             laP->io_status = WEP_BAD_NET;
             break;
          default:                /* spurrious interrupt occured */
             /* log error */
             C327TRACE4("Bin9",dev,laNum,opResults);
             break;
       }

       /* test for select wakeup on exception request  */
       if(laP->io_flags & WDI_ALL_CHECK){
          /* wake those sleeping on select or waiting for data */
          C327TRACE3("Lin7",laP->io_flags,laP);
          mwake_em_up(dev,laP,laNum);
       }

       return;
}
/*PAGE*/
/*
 * NAME: mbroadcast_status()
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
 *     what bits / data structures, etc it manipulates. 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *     software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */  

/*******************************************************************
**
** Function Name:       mbroadcast_status()
**
** Description: notifys all processes of a severe error condition
**              returned from the offlevel handler all processes are woken up,
**              with status and flags set
**
** Inputs:      status    status value to put into the process's io_status
**              flags     flag settings to put into the process's io_flags
**
** Output:      none
**
**
** Externals    mlnk_ptrs[*]   all active processes
** Referenced
**
** Externals    mlnk_ptrs[*]   all active processes
** Modified
**
********************************************************************/
static void mbroadcast_status(uint flags, uint newStatus, int dev, 
       int laNum, boolean setLS)
{
       register int    i, j;
       register linkAddr *laP;

       C327TRACE5("Rin0",dev,newStatus,flags,setLS);

       i = tca_data[dev].lower_link_address;  /* first active link address */
       j = tca_data[dev].upper_link_address;  /* last active link address */

       for (; i <= j; i++){    /* spin through the wakeup list */
          /* see if link is open */
          if((laP = tca_data[dev].mlnk_ptrs[i]) != NULL){
             laP->io_status = newStatus;
             laP->io_flags = flags;
             if(setLS)
               setLinkState(laP, LS_NORMAL);
             /*
             ** wake those sleeping on select or waiting for data
             */
             mwake_em_up(dev,laP,laNum);
             C327TRACE4("Rin1",laP->waiting_for_unmlocked,
                        laP->waiting_on_io_in_prog,
             tca_data[dev].mlnk_ptrs[i]);
             if(laP->waiting_for_unmlocked){
                /* wakeup wait for unlock */
                laP->mlocked = 0;       /* clear flags */
                laP->waiting_for_unmlocked = 0;
                e_wakeup((void *)
                          &laP->sleep_waiting_for_unmlocked);
             }


             if(laP->waiting_on_io_in_prog){
                laP->waiting_on_io_in_prog = CLEAR;
                e_wakeup((void *)
                          &laP->sleep_waiting_on_io_in_prog);
             }
          }
       }
}
/*PAGE*/
/*
 * NAME: mwake_em_up()
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
 *     what bits / data structures, etc it manipulates. 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *     software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */  

/*******************************************************************
**
** Function Name:       mwake_em_up()
**
** Description: wakes up a process that is sleeping on the 'select' or
**              the 'waiting for data' addresses.
**
** Inputs:      laP     ptr to the link address structure to be woken up
**
** Output:      none
**
** Externals    link_addresses
** Referenced
**
** Externals    link_addresses
** Modified
**
********************************************************************/
static void mwake_em_up (int dev, linkAddr *laP, int laNum)
{
       C327TRACE5("Kin0",laP,laP->dev_sele,
                  laP->waiting_for_data,laP->la_waitClear);
       if(laP->dev_sele){
       selnotify((int)(makedev(c327_dev_major, dev)),
                    laNum, (ushort)POLLPRI); 

          laP->dev_sele = 0;
          laP->dev_flags &= ~ECOL;
       }

       if(laP->waiting_for_data){
          laP->waiting_for_data = CLEAR;
          e_wakeup((void *)&laP->sleep_waiting_for_data);
       }

       if(laP->la_waitClear){
          laP->la_waitClear = CLEAR;
          e_wakeup((void *)&laP->sleep_la_waitClear);
       }
}
/*PAGE*/
/*
 * NAME: checkForHostAck()
 *                                                                    
 * FUNCTION: Look the first element on the ring queue 
 *           for the link pointed to by "laP".  If this 
 *           element contains a host acknowledge, set the 
 *           link state to LS_DATA_AVAIL, flush the retry
 *           queue and return. Otherwise, set the link 
 *           state to LS_DPE_WRITE, and start writing out
 *           the buffers again
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *     This should describe the execution environment for this
 *     procedure. For example, does it execute under a process,
 *     interrupt handler, or both. Can it page fault. How is
 *     it serialized.
 *                                                                   
 * (NOTES:) More detailed description of the function, down to
 *     what bits / data structures, etc it manipulates. 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *     software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */  
int checkForHostAck (linkAddr *laP, int laNum, int dev)
{
       Data_Buffer     *dbP;
       int     returnCode;
       char    *bufferData, cmdByte, wccByte;

       dbP = laP->la_recvDbP;

       bufferData = &dbP->buf_start;   /* pointer to stream */

       cmdByte = bufferData[0];        /* first byte in stream */
       wccByte = bufferData[1];        /* second byte in stream */

       C327TRACE5("Hin0",cmdByte,wccByte,laP->la_ackIncKeyun, wccKEYUN);

       switch(cmdByte){
          case cmdEWRT:   /* Erase/Write */
          case cmdEWRA:   /* Erase/Write Alternate */
          case cmdWRT:    /* Write */
          case cmdWSF:    /* Write Structured Field */
          case cmdEAU:    /* Erase All Unprotected */
             /*
             ** if ack. does not require keyb. reset or if it does require
             ** keyb. reset and keyb. reset bit is set - an ack. has been
             ** received
             */
             if(!laP->la_ackIncKeyun || (laP->la_ackIncKeyun &&
                (wccByte & wccKEYUN))) {
                if (laP->writeBuffer) {
                   laP->writeBuffer_used = TRUE;
                   e_wakeup((void *)&laP->sleep_waiting_for_write_buf);
                 }


                /* set link state to LS_DATA_AVAIL */
                setLinkState(laP, LS_DATA_AVAIL);

                break;
             }
             /*
             ** !!! are we doing the right thing if an acknowledge is
             ** !!! not received?
             */
          case cmdRDMD:   /* Read Modified */
          case cmdRDMA:   /* Read Modified All */
             /*
             ** host asked for retry - flush received data, set link state
             ** to LS_DPE_WRITE and start writing out retry buffers
             */
             if (laP->writeBuffer == NULL){
                C327TRACE2("Hin2",laNum);
                setLinkState(laP, LS_DATA_AVAIL);
                return(0);
             }
             setLinkState(laP, LS_DPE_WRITE);

             /*
             ** send OKDATA to acknowledge received data before we
             ** retry-resend buffer
             */
             mdepSendStatus(dev, laNum, 1);

             /*
             ** send the first buffer out again, if fail, then set link
             ** state to LS_NORMAL, free the buffers and wake up anyone
             ** needing a buffer, if it succeeds then set state to
             ** LS_RE.
             */
             returnCode = mdepWriteBuffer(dev, laNum, laP->writeBuffer);

             C327TRACE2("Hin2",returnCode);
             if(returnCode==RC_OK_NONE_PEND)
                returnCode=RC_OK_INTR_PEND;
             intrSolWrite(laP, laNum, dev, returnCode);
             return(1);      /* don't do any more - just return */

          case cmdRDBF:   /* Read Buffer */
             if (laP->writeBuffer) {
                laP->writeBuffer_used = TRUE;
                e_wakeup((void *)&laP->sleep_waiting_for_write_buf);
             }
             setLinkState(laP, LS_DATA_AVAIL);

             break;
          }
          C327TRACE2("Hin5",getLinkState(laP));
          return(0);
}
/*PAGE*/
/*
 * NAME: procAutoAck()
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
 *     what bits / data structures, etc it manipulates. 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *     software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */  
static void procAutoAck(linkAddr *laP, int laNum, int dev)
{

       /* check to see if the auto ack flag is on */
       C327TRACE4("Ain0",dev,laNum,laP->la_autoAck);
       if(laP->la_autoAck){
          /* if in auto ack mode always do the copy to the tca buffer */

          /* wait until laP->la_WSFdbP has been read by
          ** the application before overwritting it
          */
          if (bufferEmpty(laP->la_WSFdbP)) {

             /* copy recieved data to a tca buffer so we can
             ** send status to the controller immediatly w/o
             ** possible data loss
             */
             /* C327PERF( 0x0920 ); */
             bcopy((void *)laP->la_recvDbP, (void *)laP->la_WSFdbP,
                   (uint)laP->la_WSFdbP->dbhead.buff_size);
             /* C327PERF( 0x0921 ); */

             if (isApiCmdWSF(&laP->la_WSFdbP->buf_start)) {
                /*
                ** set up as if buffer had already been read
                */
                setLinkState(laP, LS_AA_INPROG);
                /* C327PERF( 0x0921 ); */
                mdepSendStatus(dev,laNum,0);
                /* C327PERF( 0x0901 ); */
             }

             /* Indicate that not waiting for la_WSFdbP */
             laP->waiting_for_WSFdb = 0;
          } else
             /* Indicate that waiting for la_WSFdbP */
             laP->waiting_for_WSFdb = 1;
       }
}
/*PAGE*/
/*
 * NAME: clearIoInProg()
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
 *     what bits / data structures, etc it manipulates. 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *     software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */  
void clearIoInProg(linkAddr *laP)
{
       C327TRACE3("Iin0",laP,laP->waiting_on_io_in_prog);

       laP->io_in_prog = CLEAR;

       if(laP->waiting_on_io_in_prog){
          laP->waiting_on_io_in_prog = CLEAR;
          C327TRACE2("Iin1",laP);
          e_wakeup((void *)&laP->sleep_waiting_on_io_in_prog);
       }
}
/*PAGE*/
/*
 * NAME: tcaintr()
 *                                                                    
 * FUNCTION: This routine will notify device head of interrupts 
 *      processed by device handler so data structures showing 
 *      the state of the adapter can be updated and threads 
 *      waiting on a change in state can be woke up 
 *                                                                    
 * EXECUTION ENVIRONMENT: This routine is part of the bottom half 
 *      of the device driver. It can be called on an interrupt level, 
 *      is pinned in memory, and can not page fault.
 *                                                          
 * NOTES: This routine will call the proper subroutines to update
 *      the proper laP structure fields showing what state the 
 *      adapter is in and also wake up threads waiting on an event
 *      as follows:
 *
 *           INTR_UNSOL - Unsolicited interrupt notifying device
 *                head of a change in state or error condition.
 *
 *           INTR_SOL_START - Solicited start interrupt notifying
 *                device head of completion of start operation.
 *                Status of the operation contained in opResults.
 *
 *           INTR_SOL_WRITE - Solicited write interrupt notifying
 *                device head of completion of write operation.
 *                Status of the operation contained in opResults.
 *
 * RECOVERY OPERATION: notify application
 *
 * DATA STRUCTURES: laP structure
 *
 * RETURNS: NONE
 */  
void tcaintr (NETWORK_ID netID, INTR_TYPE intrType, 
       int opResults, int check_status)
{
       int             dev;
       linkAddr        *laP;
       int     laNum;

       C327TRACE4("IntS", intrType, check_status, opResults);

       dev = (int)(netID.adapter_code - 1);   /* make minor device # */


       /*
       ** if interrupt is not start, then make laNum and laP, if this fails
       ** return immediately
       */

       if(intrType != INTR_SOL_START && !netId2La(dev, netID, &laP, &laNum)){
          C327TRACE3("Bin1",laP, laNum);
          return;
       }

       switch(intrType){
          case INTR_UNSOL:
             C327TRACE3("Uin0",dev,laNum);
             intrUnsol(laP, laNum, dev, opResults,
                       check_status);
             break;
          case INTR_SOL_START:
             C327TRACE4("Sin0",dev,laNum,opResults);
             intrSolStart(dev, netID, opResults);
             C327TRACE4("wak1",dev,laNum,&tca_data[dev].sleep_open_link_address);
             e_wakeup((void *)&tca_data[dev].sleep_open_link_address);
             break;
          case INTR_SOL_WRITE:
             C327TRACE4("Win0",dev,laNum,opResults);
             intrSolWrite(laP, laNum, dev, opResults);
             break;
          default:
             C327TRACE5("Bin2",dev,intrType,opResults,laP);
             break;
          }
          C327TRACE2("IntE",dev);
}
/*PAGE*/
#define netIdEqual(n1, n2)      ((n1).session_code == (n2).session_code && \
                                 (n1).adapter_code == (n2).adapter_code)
/*
 * NAME: netId2La()
 *                                                                    
 * FUNCTION: Using the netID, get the laP and laNum.  
 *                                                                    
 * EXECUTION ENVIRONMENT: This routine is part of the bottom half 
 *      of the device driver. It can be called on an interrupt level, 
 *      is pinned in memory, and can not page fault.
 *                                                          
 * RETURNS: Returns 1 if successful, 0 otherwise
 */  
int netId2La (int dev, NETWORK_ID netID, linkAddr **laPP, int *laNumP)
{
       register int    laNum;
       cardData        *cdP;
       linkAddr        *laP;

       cdP = &tca_data[dev];

       /* look for started link addresses */
       for(laNum = cdP->lower_link_address; laNum <= cdP->upper_link_address;
            laNum++){
          laP = tca_data[dev].mlnk_ptrs[laNum];

          if( (laP!=NULL) && ( netIdEqual (laP->la_netID, netID) ) )
             break;
       }

       if(laNum > cdP->upper_link_address){
          C327TRACE4("Bina",dev, cdP->upper_link_address,laNum);
          C327TRACE5("Binb",laP->la_netID.session_code,laP->la_netID.adapter_code,netID.session_code,netID.adapter_code);
          return(0);
       }

       *laNumP = laNum;
       *laPP = laP;
       return(1);
}
/*PAGE*/
/*
 * NAME: IntrSolStart()
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
 *     what bits / data structures, etc it manipulates. 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *     software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */  
static void intrSolStart(int dev, NETWORK_ID netID, int opResults)
{
       register int    laNum;
       cardData        *cdP;
       linkAddr        *laP;

       cdP = &tca_data[dev];

       /* look for started link addresses */
       for(laNum = cdP->lower_link_address; laNum <= cdP->upper_link_address;
             laNum++){
          laP = tca_data[dev].mlnk_ptrs[laNum];

          if(laP && getLinkState(laP) == LS_START_INPROG)
             break;
       }

       if(laNum > cdP->upper_link_address){
          C327TRACE5("Bin4",dev,laNum,opResults,getLinkState(laP));
          return;
       }

       laP->la_netID = netID;

       if(opResults == OR_NO_ERROR){
          setLinkState(laP, LS_NORMAL);
          laP->io_flags = 0;
          laP->io_status = 0;
       } else {                /* start failed for this link address */
                               /* bottom half only returns OR_NO_ERROR */
          C327TRACE5("Bin5",dev,laNum,opResults,getLinkState(laP));
          setLinkState(laP, LS_LINKDOWN);
          mupdate_link_fields(opResults, laP, laNum, dev);
       }

       return;
}
/*PAGE*/
/*
 * NAME: intrUnSol()
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
 *     what bits / data structures, etc it manipulates. 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *     software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */  

#define CS_ERR1  0x0f00  /* error code in Packed Decimal format (high) */
#define CS_ERR2  0x00f0  /* error code in Packed Decimal format (mid)*/
#define CS_ERR3  0x000f  /* error code in Packed Decimal format (low)*/

static void 
intrUnsol(linkAddr *laP, int laNum, int dev, int opResults, int cs)
{
       if (opResults & OR_CHK_STAT_MSK) {  /* if check status special case */
          doCheckStatus(dev,laP,laNum,cs); /* Always bad.                  */
       } else {
          if (opResults & OR_SNA_INFO) {   /* if SNA info. (WCUS 40/41) */
             laP->io_flags |= WDI_CU;
             laP->io_extra = cs;
             mwake_em_up(dev,laP,laNum);
          } 
          else
             if (opResults & OR_SOFT_DSC) {   /* Process WCUS 30/31        */
                if (cs == 0) {
                   C327TRACE1 ("wc31");  
                   laP->io_flags |= WDI_WCUS_31;
                   laP->io_status = 0;
                   mbroadcast_status(laP->io_flags,
                                     laP->io_status,dev,laNum, (boolean)FALSE);
                }
                else
                {
                   laP->io_flags |= WDI_WCUS_30;
                   laP->io_status = ((cs & CS_ERR1)>>8)*100 
                                     + ((cs & CS_ERR2)>>4)*10 + (cs & CS_ERR3);
                   C327TRACE3 ("wc30",laP->io_flags,laP->io_status);  
                   mbroadcast_status(laP->io_flags,
                                     laP->io_status,dev,laNum, (boolean)FALSE);
                }
             }
             else if (opResults & OR_SNA_RUSIZE) { /* xmit RUSIZE */
                C327TRACE1 ("RSZE");
                laP->io_flags |= WDI_RUSIZE;
                laP->io_status = cs;
                mwake_em_up(dev,laP,laNum);
             }
             else {
                if (opResults & OR_REC_DATA_AVAIL) {
                   if (cs == TRUE)  /* cmd_chaining is on */
                      laP->io_flags |= WDI_CCHAIN;
                   else 
                      laP->io_flags &= ~WDI_CCHAIN;  /* not command chaining */
                }
                mupdate_link_fields(opResults, laP, laNum, dev);
             }
       }

       /* log an error, if appropriate */
       if (!(opResults & OR_SNA_INFO)) {   /* if not SNA info. (WCUS 40/41) */
          if((laP->io_flags & WDI_ALL_CHECK) && 
             (opResults != OR_REC_DATA_AVAIL)){
             C327TRACE5("Bin3",dev,laNum,opResults,cs);
             mlogerr(ERRID_C327_INTR, dev, (int)laP->io_status, laNum);
          }
       }
}
/*PAGE*/
/*
 * NAME: doCheckStatus()
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
 *     what bits / data structures, etc it manipulates. 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *     software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */  


static void doCheckStatus (int dev,linkAddr *laP,int laNum,int cs)
{
       int     io_status;
       boolean setLS;

       C327TRACE2("Cin0", cs);
       /* compute the status value */
       io_status = ((cs & CS_ERR1)>>8)*100 + ((cs & CS_ERR2)>>4)*10 + 
                    (cs & CS_ERR3);

       /*
       ** if no COMM, PROG or MACH error and no error code or if the status
       ** code is "restart finished", then this is a broadcast clear.
       ** If the card is down, then this is the first (and legit) broadcast
       ** clear, otherwise ignore this broadcast clear - we have already
       ** received one
       */
       if ( (!(cs & (CC_ERROR_MSK | PC_ERROR_MSK | MC_ERROR_MSK)) &&
             (io_status == 0)) || (cs == OR_RESTART_DONE)) {
          if(isCardDown(dev)){
             if(cs == OR_RESTART_DONE)
                setLS = TRUE;
             else
                setLS = FALSE;
             broadcastClear(dev, laP, laNum, setLS);
          }
       } else {
          /*
          ** ... otherwise its a broadcast error
          */
          laP->io_status = io_status;
          laP->io_flags &= ~WDI_ALL_CHECK;
    
          if(cs & CC_ERROR_MSK)                  /* if a comm error */
             laP->io_flags |= WDI_COMM;
          if(cs & PC_ERROR_MSK)                  /* if a prog error */
             laP->io_flags |= WDI_PROG;
          if(cs & MC_ERROR_MSK)                  /* if a mach error */
             laP->io_flags |= WDI_MACH;
          if(cs & BROADCAST_MSK) {               /* if a fatal error */
             laP->io_flags &= WDI_ALL_CHECK;
             laP->io_flags |= WDI_FATAL;
          }

          /* status to be broadcast */
          if((cs & BROADCAST_MSK) || isCardDown(dev)){
             setCardDown(dev);      /* show card is down */
    
             mbroadcast_status(laP->io_flags,
                               laP->io_status,dev,laNum, (boolean)FALSE);
          } else
             mwake_em_up(dev,laP,laNum);
       }

       C327TRACE5("Bin7",dev,laP->io_flags,laP->io_status,isCardDown(dev));
}
/*PAGE*/
/*
 * NAME: intrSolWrite()
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
 *     what bits / data structures, etc it manipulates. 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *     software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */  
static void intrSolWrite (linkAddr *laP, int laNum, int dev, int opResults)
{
       uint    linkState;

       linkState = getLinkState(laP);

       C327TRACE2("Win1",linkState);

       if(opResults == RC_OK_INTR_PEND) {/* if ack from a regular write */
            if(laP->writeBuffer) {
               C327TRACE2("Win2",laP->writeBuffer);
               laP->writeBuffer_used = TRUE;
               e_wakeup((void *)&laP->sleep_waiting_for_write_buf);
            }
       }
       else if(opResults == RC_OK_NONE_PEND) {/* if ack from read mod write */
               /* do not free the buffer yet */
               setLinkState(laP,LS_RM_WRITE);
            }
            else {
               setLinkState(laP, LS_NORMAL);
               mupdate_link_fields(opResults, laP, laNum, dev);
               if(laP->writeBuffer) {
                  C327TRACE2("Win3",laP->writeBuffer);
                  laP->writeBuffer_used = TRUE;
                  e_wakeup((void *)&laP->sleep_waiting_for_write_buf);
               }
               C327TRACE5("Bin6",dev,laNum,laP,opResults);
            }

       clearIoInProg(laP);
       C327TRACE2("Win4",getLinkState(laP));
}
/*PAGE*/
/*
 * NAME: aixSendStat()
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
 *     what bits / data structures, etc it manipulates. 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *     software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */  
void aixSendStat (linkAddr *laP)
{
       uint            linkState;

       /*
       ** If waiting for interrupt ack. that a read ack was sent.
       */
       linkState = getLinkState(laP);
       C327TRACE2("Nin0", linkState);
       if (linkState == LS_W_WAIT_READACK)
          setLinkState(laP, LS_NORMAL);

       else if(linkState == LS_AA_INPROG){
          /* if auto ack in progress... don't clear io_* */
          setLinkState(laP, LS_DATA_AVAIL);
       }
}
/*PAGE*/
/*
 * NAME: broadcastClear()
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
 *     what bits / data structures, etc it manipulates. 
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *     software error recovery.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: What this code returns (NONE, if it has no return value)
 */  
static void broadcastClear (int dev, linkAddr *laP, int laNum, boolean setLS)
{
       setCardUp(dev);
       C327TRACE5("Tin0",dev,laP,laP->io_status,laP->io_flags);
       laP->io_flags &= ~WDI_ALL_CHECK;
       laP->io_status = 0;
       mbroadcast_status(laP->io_flags, laP->io_status, dev, laNum, setLS);
}
/*PAGE*/
/*
 * NAME: isApiCmdWSF()
 *                                                                    
 * FUNCTION: Checks if current command is an API WSF command
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: TRUE if command is an API WSF, FALSE otherwise
 */  
int isApiCmdWSF(char *buff) {

    if ((buff)[0] == cmdWSF && (buff)[1] == 0    &&
        (buff)[2] == 0x06   && (buff)[3] == 0x40 &&
        (buff)[4] == 0x00   && (buff)[5] == 0xF1 &&
        (buff)[6] == 0xC2   && (buff)[9] == 0x10 &&
        (buff)[10] == 0x14)
           return(TRUE);
    else
           return(FALSE);
}
