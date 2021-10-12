/* @(#)43   1.7  src/bos/kernext/cie/dev.h, sysxcie, bos411, 9428A410j 4/18/94 16:21:17 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   new_DEV
 *   free_DEV
 *   attachChannelToDevice
 *   detachChannelFromDevice
 *   createChannel
 *   destroyChannel
 *   describeDevice
 *   queryStatistics
 *   tokQueryVPD
 *   tokQueryTokenRingInfo
 *   entQueryVPD
 *
 * DESCRIPTION:
 *
 *    Device Object External Interface
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

#if !defined(DEV_H)
#define  DEV_H

#include <sys/ndd.h>
#include <sys/comio.h>
#include <sys/ciouser.h>
#include <sys/err_rec.h>
#include <sys/entuser.h>
#include <sys/fddiuser.h>
#include <sys/tokuser.h>
#include "devtype.h"
#include "addrtbl.h"
#include "ciedds.h"
#include "devstats.h"

/*---------------------------------------------------------------------------*/
/*                    Ethernet Device-Type-Specific Data                     */
/*---------------------------------------------------------------------------*/

typedef struct DEV_DS_ENT    DEV_DS_ENT;

struct DEV_DS_ENT
{
   char         hdwAddr[ent_NADR_LENGTH]; // Hardwired Network Address
   char         curAddr[ent_NADR_LENGTH]; // Current Network Address
   int                   proMode;         // Promiscuous Mode ref count
   int                   badFrame;        // Bad Frame Mode ref count
   ADDRTABLE           * multiCastTable;  // Multicast address table
};

/*---------------------------------------------------------------------------*/
/*                        FDDI SMT Filter Definition                         */
/*---------------------------------------------------------------------------*/

typedef struct FDDI_SMT_DEF  FDDI_SMT_DEF;

struct FDDI_SMT_DEF
{
   int                       numFilters;  // Count of enabled SMT filter codes
   int       filter[FDDI_SMT_MAX_FILTERS];// List of enabled SMT filter codes
};

/*---------------------------------------------------------------------------*/
/*                      FDDI Device-Type-Specific Data                       */
/*---------------------------------------------------------------------------*/

typedef struct DEV_DS_FDDI   DEV_DS_FDDI;

struct DEV_DS_FDDI
{
   char         hdwAddr[FDDI_NADR_LENGTH];// Hardwired Network Address
   char         curAddr[FDDI_NADR_LENGTH];// Current Network Address
   int                       smtActive:1; // FDDI SMT Netid is active
   FDDI_SMT_DEF              smtDef;      // FDDI SMT Parameters
   ADDRTABLE               * groupTable;  // FDDI Grp Address Table
};

/*---------------------------------------------------------------------------*/
/*                   Token Ring Device-Type-Specific Data                    */
/*---------------------------------------------------------------------------*/

typedef struct DEV_DS_TOK    DEV_DS_TOK;

struct DEV_DS_TOK
{                                         //
   char         hdwAddr[TOK_NADR_LENGTH]; // Hardwired Network Address
   char         curAddr[TOK_NADR_LENGTH]; // Current Network Address
   int                   macActive    :1; // MAC session active
   int                   groupActive  :1; // Group Address active
   unsigned int          groupAddr;       // Active Group Address
   chan_t                groupOwner;      // Group Address Owner (channel)
};

/*---------------------------------------------------------------------------*/
/*                               Device Class                                */
/*---------------------------------------------------------------------------*/

struct DEV
{
   char                      iCatcher[4]; // Eye Catcher
   int                       devno;       // Device Number (Major/minor)
   Simple_lock               lock;        // Device Lock
   CHN_LIST                * chnList;     // Channel List
   ndd_t                   * nddp;        // ndd structure
   IODT_ENTRY              * iodTable;    // IOCTL Dispatch Table
   int                       iodtSize;    // Size of IOCTL Dispatch Table
   int                       nchOpen;     // Number of current open channels
   int                       numSes;      // Active Sessions (all channels)
   DEVSTATS                  stats;       // Device Statistics
   DDS                       dds;         // DDS structure
                                          //----------------------------------
   union                                  // Device-Type-Specific Section
   {                                      //
      DEV_DS_ENT             ent;         //   Ethernet
      DEV_DS_TOK             tok;         //   Token Ring
      DEV_DS_FDDI            fddi;        //   FDDI
   } ds;                                  //
};                                        //

/*---------------------------------------------------------------------------*/
/*                 Create a new Device Instance Block object                 */
/*---------------------------------------------------------------------------*/

DEV *
   new_DEV(
      register dev_t         devno       ,// I -Device Number
      register const DDS   * dds          // I -DDS Temporary in kernel mem
   );

/*---------------------------------------------------------------------------*/
/*                    Free the storage occupied by a DEV                     */
/*---------------------------------------------------------------------------*/

void
   free_DEV(
      register DEV         * dev          // IO-DEV to be freed
   );

/*---------------------------------------------------------------------------*/
/*               Attach a Channel Structure to a Channel List                */
/*---------------------------------------------------------------------------*/

int
   attachChannelToDevice(
      register DEV         * dev         ,// IO-Device
      register CHN         * chn          // IO-Channel to be attached
   );

/*---------------------------------------------------------------------------*/
/*              Detach a Channel Structure from a Channel List               */
/*---------------------------------------------------------------------------*/

void
   detachChannelFromDevice(
      register DEV         * dev         ,// IO-Device
      register CHN         * chn          // I0-Channel to be detached
   );

/*---------------------------------------------------------------------------*/
/*                           Create a new channel                            */
/*---------------------------------------------------------------------------*/

int
   createChannel(
      register DEV         * dev         ,// IO-The Device
      chan_t               * chanp        //  O-Output channel pointer
   );

/*---------------------------------------------------------------------------*/
/*                             Destroy a channel                             */
/*---------------------------------------------------------------------------*/

int
   destroyChannel(
      register DEV         * dev         ,// IO-The device
      register chan_t      * chanp        // I -The channel number
   );

/*---------------------------------------------------------------------------*/
/*                     ioctl(IOCINFO) - Describe Device                      */
/*---------------------------------------------------------------------------*/

int
   describeDevice(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * info        ,//  O-User's devinfo buffer
      register ulong         devflag      // I -Device Flags from open
   );

/*---------------------------------------------------------------------------*/
/*                    Query Statistics - ioctl(CIO_QUERY)                    */
/*---------------------------------------------------------------------------*/

int
   queryStatistics(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,//  O-Paramter Block
      register ulong         devflag      // I -Device Flags from open
   );

/*---------------------------------------------------------------------------*/
/*                          Retrieve Token Ring VPD                          */
/*---------------------------------------------------------------------------*/

int
   tokQueryVPD(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * vpd         ,//  O-User's VPD Buffer
      register ulong         devflag      // I -Device Flags from open
   );

/*---------------------------------------------------------------------------*/
/*              Retrieve Token-Ring-Specific Device Information              */
/*---------------------------------------------------------------------------*/

int
   tokQueryTokenRingInfo(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * argp        ,// IO-User's tok_q_ring_info_t buf
      register ulong         devflag      // I -Device Flags from open
   );

/*---------------------------------------------------------------------------*/
/*                           Retrieve Ethernet VPD                           */
/*---------------------------------------------------------------------------*/

int
   entQueryVPD(
      register DEV         * dev         ,// I -Device
      register CHN         * chn         ,// I -Channel
      register void        * vpd         ,//  O-User's VPD Buffer
      register ulong         devflag      // I -Device Flags from open
   );

#endif
