/* @(#)37   1.9  src/bos/kernext/cie/chn.h, sysxcie, bos411, 9438B411a 9/20/94 10:15:47 */

/* 
 * 
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 * 
 * FUNCTIONS:
 * 
 *   lockChannel
 *   unlockChannel
 *   new_CHN
 *   free_CHN
 *   openChannel
 *   closeChannel
 *   getChannelStatus
 *   readData
 *   readStatus
 *   queueStatus
 *   writeData
 *   entMulticastAddr
 *   tokFuncAddr
 *   tokGroupAddr
 *   tokSetGroupAddr
 *   tokClearGroupAddr
 *   fddiGroupAddr
 *   fddiClearAllGroups
 *   fddiQueryGroupAddr
 *   entPromiscuousOn
 *   entPromiscuousOff
 *   startSession
 *   haltSession
 *   haltAllSessions
 * 
 * DESCRIPTION:
 * 
 *    Channel Object External Interface
 * 
 * ORIGINS: 27
 * 
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 */

#if !defined(CHN_H)
#define  CHN_H

#include <sys/ndd.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/err_rec.h>
#include <sys/entuser.h>
#include <sys/fddiuser.h>
#include <sys/tokuser.h>
#include <net/spl.h>

#include "sestype.h"
#include "devtype.h"
#include "status.h"
#include "queue.h"
#include "mbqueue.h"

/*---------------------------------------------------------------------------*/
/*                          Channel Instance Block                           */
/*---------------------------------------------------------------------------*/

struct CHN
{
   char                      iCatcher[4]; // Eye Catcher
   DEV                     * dev;         // Pointer to owning device
   Simple_lock               lock;        // Channel Lock
   dev_t                     devno;       // Device Number (Major/minor)
   chan_t                    chan;        // Channel Number
   int                       devflag;     // Device Flag word from Open
   MBQUEUE                   rdq;         // Read Queue Pointer
   MBQUEUE                   stq;         // Status Queue Pointer
   QUEUE                   * sesQueue;    // Queue of active Sessions
   EVENT_WORD                rdqNotEmpty; // Event for posting to Read Queue
   pid_t		     proc_id;	  // Process Id
   int			     rcv_sig;     // Ethernet Signal Support field
   uint                      sid;         // Status Filter Correlator
                                          // Channel Status Flags
   int            open             :1;    //    Channel is open
   int            lostStatus       :1;    //    Status Block was discarded
                                          //----------------------------------
   union                                  // Device-Specific Section
   {                                      //
      struct                              //    Ethernet
      {                                   //
         int  multiCastRef[ENT_MAX_MULTI];//      Multicast Addr Ref Counts
         int             proMode;         //      Promiscuous Mode ref count
         int             badFrame;        //      Bad Frame Mode ref count
      } ent;                              //
                                          //
      struct                              //    FDDI
      {                                   //
         int             smtActive    :1; //      SMT session active
         int  groupRef[FDDI_MAX_ADDRS];   //      Group Addr Reference Counts
      } fddi;                             //
                                          //
      struct                              //    Token Ring
      {                                   //
         int             macActive    :1; //      MAC session active
         int             groupActive  :1; //      Group Address active
      } tok;                              //
   } ds;
};

#define  lockChannel(p,chn)    ((void)((p) = disable_lock(PL_IMP,&(chn)->lock)))
#define  unlockChannel(p,chn)  ((void)(unlock_enable((p),&(chn)->lock)))

/*---------------------------------------------------------------------------*/
/*             Allocate and initialize a Channel Data Structure              */
/*---------------------------------------------------------------------------*/

CHN *
   new_CHN(
      register CIE_DEV_TYPE  devType     ,// I -Device Type Code
      register int           recQueueSize,// I -Receive Queue Size
      register int           staQueueSize // I -Status Queue Size
   );

/*---------------------------------------------------------------------------*/
/*                       Free a Channel Data Structure                       */
/*---------------------------------------------------------------------------*/

void
   free_CHN(
      register CHN         * chn          // IO-Channel to be freed
   );

/*---------------------------------------------------------------------------*/
/*                             Open the Channel                              */
/*---------------------------------------------------------------------------*/

int
   openChannel(
      register DEV         * dev         ,// IO-The Device
      register CHN         * chn         ,// IO-The Channel
      register ulong         devflag      // I -Device Flags
   );

/*---------------------------------------------------------------------------*/
/*                             Close the channel                             */
/*---------------------------------------------------------------------------*/

int
   closeChannel(
      register DEV         * dev         ,// IO-The Device
      register CHN         * chn          // IO-The Channel
   );

/*---------------------------------------------------------------------------*/
/*                           Return Channel Status                           */
/*---------------------------------------------------------------------------*/

int
   getChannelStatus(
      register DEV            * dev      ,// IO-Device
      register CHN            * chn      ,// IO-Channel
      register cio_stat_blk_t * arg      ,// IO-Status Block Return Area
      register ulong            devflag   // I -Device Flags from Open
   );

/*---------------------------------------------------------------------------*/
/*                Wait for and dequeue the next input buffer                 */
/*---------------------------------------------------------------------------*/

int
   readData(
      register DEV         * dev         ,// IO-The Device
      register CHN         * chn         ,// IO-The Channel
      register mbuf_t     ** pm           //  O-Output mbuf pointer
   );

/*---------------------------------------------------------------------------*/
/*                 Dequeue and return the next status block                  */
/*---------------------------------------------------------------------------*/

void
   readStatus(
      register CIE_DEV_TYPE     devType     ,// I -Device Type Code
      register CHN            * chn         ,// IO-The Channel
      register cio_stat_blk_t * stat         //  O-Output status buffer
   );

/*---------------------------------------------------------------------------*/
/*                     Queue a status block to a channel                     */
/*---------------------------------------------------------------------------*/

void
   queueStatus(
      register CHN           * chn         ,// IO-The channel
      register const STATBLK * s            // I -The status block
   );

/*---------------------------------------------------------------------------*/
/*                       Write a data block to the NDD                       */
/*---------------------------------------------------------------------------*/

int
   writeData(
      register DEV         * dev         ,// IO-The device
      register CHN         * chn         ,// IO-The channel
      register mbuf_t      * m            // I -The mbuf
   );

/*---------------------------------------------------------------------------*/
/*                Handle Ethernet Multicast Address Requests                 */
/*---------------------------------------------------------------------------*/

int
   entMulticastAddr(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter block
      register ulong         devflag      // I -Device Flags from open
   );

/*---------------------------------------------------------------------------*/
/*                 Set/Clear a Token Ring Functional Address                 */
/*---------------------------------------------------------------------------*/

int
   tokFuncAddr(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter Block
      register ulong         devflag      // I -Device Flags from open
   );

/*---------------------------------------------------------------------------*/
/*                      Set a Token Ring Group Address                       */
/*---------------------------------------------------------------------------*/

int
   tokGroupAddr(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter Block
      register ulong         devflag      // I -Device Flags from open
   );

/*---------------------------------------------------------------------------*/
/*                      Set a Token Ring Group Address                       */
/*---------------------------------------------------------------------------*/

int
   tokSetGroupAddr(
      register DEV         * dev         ,// IO-Device
      register CHN         * chn         ,// IO-Channel
      register ndd_t       * nddp        ,// IO-NDD
      register unsigned int  groupAddr    // I -Group Address (lsw)
   );

/*---------------------------------------------------------------------------*/
/*                     Clear a Token Ring Group Address                      */
/*---------------------------------------------------------------------------*/

int
   tokClearGroupAddr(
      register DEV         * dev         ,// IO-Device
      register CHN         * chn         ,// IO-Channel
      register ndd_t       * nddp        ,// IO-NDD
      register unsigned int  groupAddr    // I -Group Address (lsw)
   );

/*---------------------------------------------------------------------------*/
/*                       Set a Group Address for FDDI                        */
/*---------------------------------------------------------------------------*/

int
   fddiGroupAddr(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter Block
      register ulong         devflag      // I -Device Flags from open
   );

/*---------------------------------------------------------------------------*/
/*              Disable all fddi group addresses for a channel               */
/*---------------------------------------------------------------------------*/

int
   fddiClearAllGroups(
      register DEV         * dev         ,// IO-Device
      register CHN         * chn          // IO-Channel
   );

/*---------------------------------------------------------------------------*/
/*                      Query Group Addresses for FDDI                       */
/*---------------------------------------------------------------------------*/

int
   fddiQueryGroupAddr(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter Block
      register ulong         devflag      // I -Device Flags from open
   );

/*---------------------------------------------------------------------------*/
/*                     Turn on Ethernet Promiscuous Mode                     */
/*---------------------------------------------------------------------------*/

int
   entPromiscuousOn(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter Block
      register ulong         devflag      // I -Device Flags from open
   );

/*---------------------------------------------------------------------------*/
/*                    Turn off Ethernet Promiscuous Mode                     */
/*---------------------------------------------------------------------------*/

int
   entPromiscuousOff(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter Block
      register ulong         devflag      // I -Device Flags from open
   );

/*---------------------------------------------------------------------------*/
/*                      Start a session on the channel                       */
/*---------------------------------------------------------------------------*/

int
   startSession(
      register DEV            * dev         ,// IO-Device Block
      register CHN            * chn         ,// IO-Channel Block
      register cio_sess_blk_t * uarg        ,// IO-IOCTL Command Parameters
      register ulong            devflag      // I -Device Flags from Open
   );

/*---------------------------------------------------------------------------*/
/*                       Halt a session on the channel                       */
/*---------------------------------------------------------------------------*/

int
   haltSession(
      register DEV            * dev         ,// IO-Device Block
      register CHN            * chn         ,// IO-Channel Block
      register cio_sess_blk_t * uarg        ,// IO-IOCTL Command Parameters
      register ulong            devflag      // I -Device Flags from Open
   );

/*---------------------------------------------------------------------------*/
/*                       Terminate all active sessions                       */
/*---------------------------------------------------------------------------*/

void
   haltAllSessions(
      DEV                  * dev         ,// IO-Device
      CHN                  * chn          // IO-Channel
   );


int
   entSignalSupport(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-Parameter Block
      register ulong         devflag      // I -Device Flags from open
   );

#endif
/*---------------------------------------------------------------------------*/
/*                     Turn on Ethernet Signal Support                       */
/*---------------------------------------------------------------------------*/
