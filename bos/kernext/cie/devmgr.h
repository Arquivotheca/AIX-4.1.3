/* @(#)44   1.7  src/bos/kernext/cie/devmgr.h, sysxcie, bos411, 9428A410j 4/18/94 16:21:27 */

/*
 *
 * COMPONENT_NAME: (SYSXCIE) COMIO Emulator
 *
 * FUNCTIONS:
 *
 *   init_DEVMGR
 *   uninit_DEVMGR
 *   createDevice
 *   destroyDevice
 *   acquireDevice
 *   releaseDevice
 *   acquireChannel
 *   releaseChannel
 *
 * DESCRIPTION:
 *
 *    Device Manager External Interface
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

#if !defined(DEVMGR_H)
#define  DEVMGR_H

#include <sys/device.h>

/*---------------------------------------------------------------------------*/
/*                   Initialize the DEVMGR Data Structure                    */
/*---------------------------------------------------------------------------*/

int
   init_DEVMGR(
      void
   );

/*---------------------------------------------------------------------------*/
/*                  Un-initialize the DEVMGR data structure                  */
/*---------------------------------------------------------------------------*/

void
   uninit_DEVMGR(
      void
   );

/*---------------------------------------------------------------------------*/
/*                            Create a new Device                            */
/*---------------------------------------------------------------------------*/

int
   createDevice(
      dev_t                  devno       ,// I -Device identifier
      uio_t                * uiop         // I -Device Dependent Structure
   );

/*---------------------------------------------------------------------------*/
/*                             Destroy a Device                              */
/*---------------------------------------------------------------------------*/

int
   destroyDevice(
      dev_t                  devno        // I -Device identifier
   );

/*---------------------------------------------------------------------------*/
/*           Acquire temporary 'ownership' of the specified device           */
/*---------------------------------------------------------------------------*/

int
   acquireDevice(
      register dev_t         devno       ,// I -Device Number
      register DEV        ** pdev         //  O-Return area for dev
   );

/*---------------------------------------------------------------------------*/
/*                 Release ownership of the specified device                 */
/*---------------------------------------------------------------------------*/

void
   releaseDevice(
      register DEV         * dev          // IO-The device
   );

/*---------------------------------------------------------------------------*/
/*          Acquire temporary 'ownership' of the specified channel           */
/*---------------------------------------------------------------------------*/

int
   acquireChannel(
      register dev_t         devno       ,// I -Device Number
      register chan_t        chan        ,// I -Channel Number
      register DEV        ** pdev        ,//  O-Ptr to dev return
      register CHN        ** pchn         //  O-Ptr to chn return
   );

/*---------------------------------------------------------------------------*/
/*                Release ownership of the specified channel                 */
/*---------------------------------------------------------------------------*/

void
   releaseChannel(
      register DEV         * dev         ,// IO-The Device
      register CHN         * chn          // IO-The channel
   );

#endif
